#ifndef HASH
#define HASH

#include <cstdint>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef uint64_t Hash;

Hash getHash(const char *word, size_t length);

#endif
