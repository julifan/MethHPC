#ifndef USECASE
#define USECASE

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>

#include <tuple>
#include <string>


std::tuple<std::string, int> map(char* block, int* moved, const int totalLength);
int reduce(int one, int two);

#endif
