#include "pi.h"
#include <math.h>

void init_pi(int set_seed, char *outfile)
{
        if (filename != NULL) {
                free(filename);
                filename = NULL;
        }

        if (outfile != NULL) {
                filename = (char*)calloc(sizeof(char), strlen(outfile)+1);
                memcpy(filename, outfile, strlen(outfile));
                filename[strlen(outfile)] = 0;
        }
        seed = set_seed;
}

void cleanup_pi()
{
        if (filename != NULL)
                free(filename);
}

void compute_pi(int flip, int *local_count, double *answer)
{

    int num_ranks;
    int world_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int count = 0;
    double x, y, z;

    seed = seed * world_rank;
    srand(seed); // Important: Multiply SEED by "rank" when you introduce MPI!

    // Calculate PI following a Monte Carlo method
    int iter;
    for (iter = 0; iter * num_ranks + world_rank < flip; iter++)
    {
        // Generate random (X,Y) points
        x = (double)random() / (double)RAND_MAX;
        y = (double)random() / (double)RAND_MAX;
        z = sqrt((x*x) + (y*y));

        // Check if point is in unit circle
        if (z <= 1.0)
        {
            *local_count += 1;
        }
    }

    MPI_Request req[num_ranks-1];

    if (world_rank == 0) {
        //issue all recv operations. 
        int count_temp[num_ranks];
        count_temp[0] = *local_count;
        int j;

        for (j = 1; j < num_ranks; j++) {
            //receive on specified place in array
            MPI_Irecv(&count_temp[j], 1, MPI_INT, j, 0, MPI_COMM_WORLD, &req[j-1]);

        }

        MPI_Waitall(num_ranks-1, req, MPI_STATUSES_IGNORE);
        //sum up all local_counts
        int i;
        for (i = 0; i < num_ranks; i++) {
            count += count_temp[i];
        }

        // Estimate Pi and display the result
        *answer = ((double)count / (double)flip) * 4.0;
    } else {
        //send local_count to rank 0 
        MPI_Isend(local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &req[world_rank-1]);

 }



}
