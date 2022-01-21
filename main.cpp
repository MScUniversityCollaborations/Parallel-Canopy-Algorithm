#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cfloat>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <ctime>
#include <mpi.h>
#include <fstream>
#include <sstream>
using namespace std;

int world_rank;
int world_size;

int number0fPoints = 150; //number of points for clustering
int dimensionsOfPoint = 2;

float distance1 = 200; // loose distance T1
float distance2 = 100; // tight distance T2

class Point {
private:
  int point_id;
  
protected:
  float* vals;

public:
  static int point_id_counter;
  Point() {
    vals = new float[dimensionsOfPoint];
    memset(vals, 0, sizeof(vals));
    point_id = point_id_counter++;
  }

  Point(float *vals) {
    this->vals = new float[dimensionsOfPoint];
    memcpy(this->vals, vals, sizeof(float)*dimensionsOfPoint);
    point_id = point_id_counter++;
  }

  Point(vector<float>& vals) {
    assert(vals.size() == dimensionsOfPoint);
    this->vals = new float[dimensionsOfPoint];
    memcpy(this->vals, vals.data(), sizeof(this->vals));
    point_id = point_id_counter++;
  }

  int get_point_id() const {
    return point_id;
  }

  float get_val(int i) const {
    
    return vals[i];
  }

  float get_squared_dist(Point& other_point) const {
    float result = 0.0;
    for (int i=0; i<dimensionsOfPoint; i++) {
      result += pow(vals[i] - other_point.vals[i], 2);
    }
    return result;
  }

  void print() const {
    for (int i=0; i<dimensionsOfPoint; i++) {
      cout << vals[i] << ' ';
    }
    cout << endl;
  }

  bool operator==(const Point& other_point) const {
    return other_point.get_point_id() == point_id;
  }
};

class Canopy {
private:
  vector<Point*> data_points;
  Point* centre;
public:
  Canopy(Point* centre) {
    assert(centre);
    this->centre = centre;
    data_points.push_back(centre);
  }

  void add_point(Point* point) {
    data_points.push_back(point);
  }

  vector<Point*> get_data_points() {
    return data_points;
  }

  void print() {
    centre->print();
  }

  void printElements() const {
    for (int i=0; i<data_points.size(); i++) {
      cout << "\t";
      (*data_points[i]).print();
    }
  }
};

void generate_points(vector<Point*>& points, int number0fPoints) {
  for (int i=0; i<number0fPoints; i++) {
    float vals[dimensionsOfPoint];
    for (int j=0; j<dimensionsOfPoint; j++) {
      vals[j] = ((rand() / float(RAND_MAX) ) * 1000);
    }
    Point* point = new Point(vals);
    points.push_back(point);
  }
}


vector<Canopy> canopy_mpi(vector<Point*>& all_points) {

  // the main process will generate points and scatter them among
  // other processes
  int rec_buff_cnt =
    (number0fPoints / world_size + (world_rank < number0fPoints % world_size ? 1 : 0)) * dimensionsOfPoint;
  float* rec_buff = new float[rec_buff_cnt];

  vector<Point*> points;

  int* scatter_counts = new int[world_size];
  int* scatter_displs = new int[world_size];
  int sum = 0;
  for (int i=0; i<world_size; i++) {
    scatter_counts[i] =  number0fPoints / world_size * dimensionsOfPoint;
    if (i < number0fPoints % world_size) {
      scatter_counts[i] += dimensionsOfPoint;
    }
    scatter_displs[i] = sum;
    sum += scatter_counts[i];
  }

  float* data = NULL;
  if (world_rank == 0) {
    data = new float[number0fPoints * dimensionsOfPoint];
    int i = 0;
    for (const Point* p : all_points) {
      for (int j=0; j<dimensionsOfPoint; j++) {
        data[i++] = p->get_val(j);
      }
    }
  }
  MPI_Scatterv(data, scatter_counts, scatter_displs, MPI_FLOAT,
               rec_buff, rec_buff_cnt, MPI_FLOAT,
               0, MPI_COMM_WORLD);

  // reconstruct points from rec_points_data
  Point::point_id_counter = 0;
  for (int i=0; i<rec_buff_cnt; i += dimensionsOfPoint) {
    Point* data_point = new Point(rec_buff + i);
    points.push_back(data_point);
  }

  // each process has a chunk of points

  unordered_set<Point*> point_set;
  for (Point* p : points) {
    point_set.insert(p);
  }

  vector<Canopy> canopies;
  
  while (!point_set.empty()) {
    Point* new_canopy_centre = *(point_set.begin());
    Canopy new_canopy(new_canopy_centre);
    point_set.erase(new_canopy_centre);
    vector<Point*> points_to_erase;
    for (Point* p : point_set) {
      float squared_dist = new_canopy_centre->get_squared_dist(*p);
      if (squared_dist < distance1 * distance1) {
        new_canopy.add_point(p);
      }
      if (squared_dist < distance2 * distance2) {
        points_to_erase.push_back(p);
      }
    }
    for (Point* p : points_to_erase) {
      point_set.erase(p);
    }
    canopies.push_back(new_canopy);
  }

  // process that picks the new canopy centre
  int root_process = 0;
  int canopy_id = 0;
  int* canopy_id_send_data = new int[scatter_counts[world_rank] / 2];
  memset(canopy_id_send_data, -1, sizeof(int) * scatter_counts[world_rank] / 2);

  while (true) {
    MPI_Barrier(MPI_COMM_WORLD);

    // try to update the root_process
    int prev_root_process = root_process;
    int temp_root_process = root_process;
    if (world_rank == root_process && point_set.empty()) {
      temp_root_process++;
    }
    MPI_Bcast(&temp_root_process, 1, MPI_INT, root_process, MPI_COMM_WORLD);
    root_process = temp_root_process;

    if (root_process >= world_size) {
      // we got through all the processes and can break
      break;
    }

    if (root_process != prev_root_process) {
    
      continue;
    }

    Point* new_canopy_centre = NULL;
    float* new_canopy_centre_data = new float[dimensionsOfPoint];
    if (world_rank == root_process) {
      new_canopy_centre = *(point_set.begin());
      point_set.erase(new_canopy_centre);
      for (int i=0; i<dimensionsOfPoint; i++) {
        new_canopy_centre_data[i] = new_canopy_centre->get_val(i);
      }
    }
    //broadcast new canopy centre
    MPI_Bcast(new_canopy_centre_data, dimensionsOfPoint, MPI_FLOAT, root_process, MPI_COMM_WORLD);
    if (world_rank != root_process) {
      new_canopy_centre = new Point(new_canopy_centre_data);
    }
    if (world_rank == 0) {
      canopies.push_back(Canopy(new_canopy_centre));
    }

    vector<Point*> points_to_erase;
    for (Point* p : point_set) {
      float squared_dist = new_canopy_centre->get_squared_dist(*p);
      if (squared_dist < distance1 * distance1) {
        canopy_id_send_data[p->get_point_id()] = canopy_id;
      }
      if (squared_dist < distance2 * distance2) {
        points_to_erase.push_back(p);
      }
    }
    for (Point* p : points_to_erase) {
      point_set.erase(p);
    }
    canopy_id++;
  }

  // gather the information about canopy points in process 0
  int* canopy_id_data = NULL;
  if (world_rank == 0) {
    canopy_id_data = new int[all_points.size()];
  }

  int* gather_counts = new int[world_size];
  int* gather_displs = new int[world_size];
  sum = 0;
  for (int i=0; i<world_size; i++) {
    gather_counts[i] =  number0fPoints / world_size;
    if (i < number0fPoints % world_size) {
      gather_counts[i]++;
    }
    gather_displs[i] = sum;
    sum += gather_counts[i];
  }

  MPI_Gatherv(canopy_id_send_data, gather_counts[world_rank], MPI_INT,
             canopy_id_data, gather_counts, gather_displs, MPI_INT,
             0, MPI_COMM_WORLD);

  // create new canopy in process 0
  if (world_rank == 0) {
    for (int i=0; i<all_points.size(); i++) {
      if (canopy_id_data[i] != -1) {
        canopies[canopy_id_data[i]].add_point(all_points[i]);
      }
    }
  }
  return canopies;
}

int Point::point_id_counter = 0;

int main(int argc, char** argv) {

  MPI_Init(NULL, NULL);

  srand(time(NULL));

  assert(distance1 > distance2);

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  vector<Point*> all_points;
  if (world_rank == 0) {
    generate_points(all_points, number0fPoints);
    number0fPoints = all_points.size();

  }

  // call canopy
  vector<Canopy> canopies = canopy_mpi(all_points);

  // print points
  if (world_rank == 0) {
   
    // print canopies
    cout<<"Resulted clusters: "<<endl;
    for (Canopy& c : canopies) {
      cout<<"Cluster center: ";
      c.print();
      c.printElements();
    }
  }

  MPI_Finalize();
  return 0;
}