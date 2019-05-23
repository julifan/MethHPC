#include "backend.h"
#include "usecase.h"

#include <iostream>


char* input;
int length;

MPI_File outputFile;


void init(char* input, char* output) {
	//MPI init
	//set output file
	//read from given input file (if rank == 0), 
	//scatter data to map's of processes (if rank == 0)

	int world_size, world_rank;
	
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	MPI_File file;
	
	MPI_Offset read_pointer = 0;
	MPI_Offset file_size = 0;
	
	
	int read_buffer_size = 32;
	
	
	char* read_buffer = new char[read_buffer_size];
	
	if(world_rank == 0) 
	{
		MPI_File_open(MPI_COMM_SELF, input, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
		MPI_File_get_size(file, &file_size);
	}
	MPI_Bcast(&file_size, 1, MPI_OFFSET, 0, MPI_COMM_WORLD);
	
	
	int map_buffer_size = file_size / world_size;
	
	if(world_rank <= file_size - map_buffer_size * world_size) {
		map_buffer_size++;
	}
	
	
	
	char* map_buffer = new char[map_buffer_size];
	int map_buffer_offset = 0;
	
	
	while(read_pointer < file_size)
	{
		
		int read_size = read_buffer_size;
		if(read_pointer + read_size > file_size) {
			read_size = file_size - read_pointer;
		}
		
		if(world_rank == 0)
		{
			MPI_File_read_at(file, read_pointer, read_buffer, read_size, MPI_CHAR, MPI_STATUS_IGNORE);
		}
		
		int map_buffer_read_size = read_size / world_size;
		
		if(world_rank < read_size - map_buffer_read_size * world_size) {
			map_buffer_read_size++;
		}
		
		int* sizes = new int[world_size];
		int* offsets = new int[world_size];
		
		
		sizes[world_rank] = map_buffer_read_size;
		for(int i = 0; i < world_size; i++)
		{
			MPI_Bcast(sizes + i, 1, MPI_INT, i, MPI_COMM_WORLD);
		}
		int offset = 0;
		for(int i = 0; i < world_size; i++)
		{
			offsets[i] = offset;
			offset += sizes[i];
		}
		
		
		MPI_Scatterv(read_buffer, sizes, offsets, MPI_CHAR, map_buffer + map_buffer_offset, map_buffer_read_size, MPI_CHAR, 0, MPI_COMM_WORLD);
		map_buffer_offset += map_buffer_read_size;
		
		
		delete[] sizes;
		delete[] offsets;
		
		read_pointer += read_buffer_size * sizeof(char);
	}

	delete[] read_buffer;
	
	length = map_buffer_size;
	input = map_buffer;
	
	std::string str(input, length);
	
	std::cout << "rank " << world_rank << ": " << str << std::endl;
}


void mapReduce() {
	//call map() from usecase
	//redistribute
	//reduce
	//write to file (if rank == 0)
	
	
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::vector<std::tuple<std::string, int > > buckets[size];

	//call map() from usecase

	char* myInput = "Hello, world! Aren't you clever? 'Later', she said. Maybe 5 o'clock?' In the year 2017 ... G2g, cya l8r hello_world.h Hermione's time-turner. Good mor~+%g. Hi' Testing_ Bye- The kids' toys toys hello_world";
	int totalLength = 205;
	int mv = 0;
	int * moved = &mv;

	//assumption: input and length are initialized.
	while (totalLength > 0) {
		std::tuple<std::string, int> tup = map(myInput, moved, totalLength);
		//TODO probably pad strings? (to achieve constant length)
		//TODO hash string from tuple
		//TODO sort in right bucket
		buckets[0].push_back(tup);

		myInput = myInput + *moved; 
		totalLength = totalLength - *moved;
		mv = 0;
		
		std::cout << "Tupel: " << std::get<0>(tup) << std::endl; 
	
	}

	//redistribute
	//reduce
	//write to file (if rank == 0)

	
}


void cleanup() {
	//free everything
	//MPI_Finalize
}
