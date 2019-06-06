#ifndef BACKEND
#define BACKEND


#include <mpi.h>
#include <omp.h>

void init(char*, char*);
void mapReduce();
void cleanup();

#endif
