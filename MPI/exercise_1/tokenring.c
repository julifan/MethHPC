#include <mpi.h>

int main(int argc, char **argv) {
        int rank, num_ranks, token;
        token = 0;

        MPI_Init(&argc, &argv);

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

        for(int i = 0; i < num_ranks - 1; i++) {
                if (rank == i) {
                        MPI_Send(&token, 1, MPI_INT, i+1, 0, MPI_COMM_WORLD);
                } else if (rank == i+1) {
                        MPI_Recv(&token, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        printf("Rank %d received %d from rank %d \n", i+1, token, i);
                        token = token + 1;
                }
        }

        MPI_Finalize();
}

