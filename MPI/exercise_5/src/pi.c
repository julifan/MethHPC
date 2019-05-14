#include "pi.h"

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
    
    int flips = 0;
    int count = 0;
    double x, y, z;

    seed = seed * world_rank;
    srand(seed);

    int iter;
    for(iter = 0; iter * num_ranks + world_rank < flip; iter++)
    {
        x = (double) random() / (double) RAND_MAX;
        y = (double) random() / (double) RAND_MAX;
        z = sqrt((x*x) + (y*y));

        if(z <= 1.0) {
            *local_count += 1;
        }
        flips += 1;
    }
    
    MPI_Reduce(local_count, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double contribution = (double) *local_count / (double) flips;
    char output[15] = "               ";
    snprintf(output, 15, "%d \t%f", world_rank, contribution);
    output[15 - 1] = *"\n";
    MPI_File file;
    
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file);
    MPI_File_write_at(file, world_rank * 15, output, 15, MPI_CHAR, MPI_STATUS_IGNORE);
    
    if(world_rank == 0) {
        *answer = ((double)count / (double)flip) * 4.0;
        char pi_output[15] = "               ";
        snprintf(pi_output, 15, "pi = %f", *answer);
        MPI_File_write_at(file, num_ranks * 15, pi_output, 15, MPI_CHAR, MPI_STATUS_IGNORE);
        MPI_File_set_size(file, (num_ranks+1) * 15 - 1);
    }
    MPI_File_close(&file);
}


