#include "backend.h"

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

