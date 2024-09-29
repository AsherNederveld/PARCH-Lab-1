// mpi.c
#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);
    int world_size;
    int world_rank;
    int name_len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); 
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); 
    MPI_Get_processor_name(processor_name, &name_len); 
    printf("Hello from processor %s, rank %d out of %d processors\n",
            processor_name, world_rank, world_size);
    MPI_Finalize();
    return 0;
}