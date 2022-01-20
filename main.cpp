#include <ostream>
#include <iostream>
#include <mpi.h>    
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <bits/stdc++.h>
using namespace std;

#define ZERO 0  

class Point{

    private:

        int point_id, cluster_id;
        vector<double> dimensions;
        int total_dimensions;


    public:
        Point(int point_id, vector<double> dimensions)
        {
            this->point_id = point_id;
            total_dimensions = dimensions.size();

            for(int i = 0; i<total_dimensions; i++)
                this->dimensions.push_back(dimensions[i]);

            cluster_id = -1;
        }

        int getID()
        {
            return point_id;
        }

        void setCluster(int cluster_id)
        {
            this->cluster_id = cluster_id;
        }

        double getDimensions(int index)
        {
            return dimensions[index];
        }

        int getTotaldimensions()
        {
            return total_dimensions;
        }

};

void generateRandomCoordinates(int numberOfRandCoordinates) {

            srand(time(NULL));

            float divide = 69/4.20;
            double coordinateX, coordinateY;
            bool xsign, ysign; // true if negative
            
            double arrX[numberOfRandCoordinates];
            double arrY[numberOfRandCoordinates];
            
            for(int i=0; i<numberOfRandCoordinates; i++)
            {
             
                coordinateX = rand()%1000;
                coordinateY = rand()%1000;
               
                xsign = rand()%4;
                ysign = rand()%4;
                
                if(xsign) coordinateX /=  divide;
                if(ysign) coordinateY /=  divide;
           
                arrX[i] = coordinateX;
                arrY[i] = coordinateY;		
            
            }
            
            for (int i=0; i  < numberOfRandCoordinates; i++) {
                cout << arrX[i] << ", " << arrY[i] << endl;
            }	
}

void mpi() {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
        
    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if (world_rank == ZERO) {

        printf("I am processes ZERO boss\n");

        int number0fPoints; 
        cout << "Set number of points for clustering: \n"; // User Input
        cin >> number0fPoints;     // Get user input from the keyboard
        cout << "Your number is:\n" << number0fPoints; // Display the input value  
        generateRandomCoordinates(number0fPoints);
        MPI_Scatter(MPI_COMM_WORLD);
    }






    //We will create the master processes 
    //Well will create scatter() function (για αποστολη μηνυματος στις υπολοιπες διεργασίες)
    //Χρειαζόμαστε την scatter() για να στειλουμε ενα κομματι πινακα σε αλλες διεργασίες 
    //Συγκεκριμένα τα περιεχόμενα της ι-οστης θέσης του πίνακα στέλνονται στην ι-οστη διεργασία
    //Gather() για να πιαμε πισω τα δεδομενα
    //Reduce() κανουμε πραξεις με αυτην την fun οταν κατα την διαρκεια επιστροφης των δεδομενων



    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    //printf("Hello world from processor %s, rank %d out of %d processors with cpp \n",
      //     processor_name, world_rank, world_size);


    // Finalize the MPI environment.
    MPI_Finalize();

}

int main(int argc, char** argv) {


    mpi();

    return 0;
      
}