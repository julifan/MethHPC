#ifndef USECASE
#define USECASE

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>


struct Pair {
	std::string key;
	int value;
	Pair(std::string key, int value): key(key), value(value) {}
};


Pair map(char* block, int* moved, const int totalLength);
int reduce(int one, int two);


#endif
