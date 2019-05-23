#include "backend.h"
#include "usecase.h"
#include "hash.h"

char* input;
int length;

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


void mapReduce() {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::vector<std::tuple<std::string, int > > buckets[size];

	//call map() from usecase

	char* myInput = "Hello, world! Aren't you clever? 'Later', she said. Maybe 5 o'clock?' In the year 2017 ... G2g, cya l8r hello_world.h Hermione's time-turner. Good mor~+%g. Hi' Testing_ Bye- The kids' toys toys hello_world ''aaaaaaaaaaaaaaaaaaaabbbbb";
	int totalLength = 233;
	int mv = 0;
	int * moved = &mv;

	//assumption: input and length are initialized.
	while (totalLength > 0) {
		std::tuple<std::string, int> tup = map(myInput, moved, totalLength);
		if (std::get<0>(tup) != "") {
		//TODO probably pad strings? (to achieve constant length)
		Hash hash = getHash(std::get<0>(tup).c_str(), std::get<0>(tup).length());
		buckets[hash % size].push_back(tup);

std::cout << "Tupel: " << std::get<0>(tup) << " bucket: " << std::endl;//hash % size << " Hash: " << hash << std::endl; 

		}
		myInput = myInput + *moved; 
		totalLength = totalLength - *moved;
		mv = 0;
		
	}

	//redistribute
	//reduce
	//write to file (if rank == 0)

	
}


void cleanup() {
	//free everything
	//MPI_Finalize
}

