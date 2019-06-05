#ifndef BACKEND
#define BACKEND

#include <mpi.h>

void init(char*, char*);
void mapReduce();
void cleanup();

#endif
