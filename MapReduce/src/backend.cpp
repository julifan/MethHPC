#include "backend.h"

#include <iostream>

#define SIZE 100

struct Map {
	char* key;
	int value;
};

struct Map *map;
MPI_File outputFile;


void init(char* input, char* output) {
	//MPI init
	//set output file
	//read from given input file (if rank == 0), 
	//scatter data to map's of processes (if rank == 0)
	
	int world_size;
	int world_rank;
	
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	MPI_File file;
	
	MPI_Offset read_pointer = 0;
	MPI_Offset file_size = 0;
	
	
	int read_buffer_size = 1024;
	int map_buffer_size = 256;
	
	
	char* read_buffer = new char[read_buffer_size];
	char* map_buffer = new char[map_buffer_size];
	
	if(world_rank == 0) 
	{
		MPI_File_open(MPI_COMM_SELF, input, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
		MPI_File_get_size(file, &file_size);
	}
	
	MPI_Bcast(&file_size, 1, MPI_OFFSET, 0, MPI_COMM_WORLD);
	
	while(read_pointer < file_size)
	{
		if(world_rank == 0)
		{
			int read_size = read_buffer_size;
			if(read_pointer + read_buffer_size >= file_size){read_size = file_size - }
			MPI_File_read_at(file, read_pointer, read_buffer, read_size, MPI_CHAR, MPI_STATUS_IGNORE);
		}
		MPI_Iscatter(read_buffer, 8, MPI_CHAR, map_buffer, 8, MPI_CHAR, 0, MPI_COMM_WORLD, MPI_);
		
		
		read_pointer += read_buffer_size * sizeof(char);
	}
	
	delete[] read_buffer;
	delete[] map_buffer;

}


void mapReduce() {
	//call map() from usecase
	//redistribute
	//reduce
	//write to file (if rank == 0)
	
	
	
}


void cleanup() {
	//free everything
	//MPI_Finalize
}

