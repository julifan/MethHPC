#ifndef BACKEND
#define BACKEND

#include <vector>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
//#include <string.h>
#include <string>
#include <assert.h>

#include <mpi.h>
#include <omp.h>

void init(char*, char*);
void mapReduce();
void cleanup();

#endif
