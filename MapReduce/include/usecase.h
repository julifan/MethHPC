#ifndef USECASE
#define USECASE



#include <mpi.h>

#include <string>



struct Pair {
	std::string key;
	int value;
	Pair(std::string key, int value): key(key), value(value) {}
};


Pair map(char* block, int* moved, const int totalLength);
int reduce(int one, int two);


#endif
