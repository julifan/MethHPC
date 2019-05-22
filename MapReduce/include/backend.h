#ifndef BACKEND
#define BACKEND

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>

void init(char*, char*);
void mapReduce();
void cleanup();

#endif
