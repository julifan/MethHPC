#include "backend.h"

void init(char* input, char* output) {}

void mapReduce() 
{
    int world_size;
    int world_rank;
    
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    
    
    char* filename = "wikipedia_test_small.txt";
    
    MPI_File file;
    
    MPI_Offset read_pointer = 0;
    MPI_Offset file_size = 0;
    
    char read_buffer[32];
    char map_buffer[8];
    
    if(world_rank == 0) 
    {
        MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
        MPI_File_get_size(file, &file_size);
    }
    
    while(read_pointer + 32 * sizeof(char) < file_size)
    {
        if(world_rank == 0)
        {
            MPI_File_read_at(file, read_pointer, read_buffer, 32, MPI_CHAR, MPI_STATUS_IGNORE);
        }
        MPI_Scatter(read_buffer, 32, MPI_CHAR, map_buffer, 8, MPI_CHAR, 0, MPI_COMM_WORLD);
        
        std::cout << world_rank << ": " << map_buffer << std::endl;
        
        read_pointer += 32 * sizeof(char);
    }
    
    
    
}


void cleanup() {}

