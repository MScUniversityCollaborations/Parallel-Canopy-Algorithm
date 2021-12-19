#include <ostream>
#include <iostream>
#include <mpi.h>
#include <cstdio>
#include <ctime>
#include <cstdlib>

using namespace std;

#define ZERO 0

void randomNumberGenerator(char rank) {
    if(rank == ZERO){
        int number0fPoints; 
        cout << "Set number 0f points for clustering: "; // User Input
        cin >> number0fPoints;     // Get user input from the keyboard
        cout << "Your number is:\n" << number0fPoints; // Display the input value    

        srand(time(0));  // Initialize random number generator.
        cout << "Random numbers generated between 1 and 10:" << endl;
        for(int i=0; i<number0fPoints ;i++){
            cout << (rand() % 10) + 1<<" "; 
        }
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

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors with cpp \n",
           processor_name, world_rank, world_size);


    // Finalize the MPI environment.
    MPI_Finalize();

}

int main(int argc, char** argv) {
    
    

    
}

