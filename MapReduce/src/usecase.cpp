#include "usecase.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>


#define MAX_LENGTH 20

Pair map(char* block, int* moved, const int totalLength) 
{
	std::string toReturn = std::string("");
	while (*moved < totalLength) {
		if ((block[0] < 'a' || block[0] > 'z') && (block[0] < 'A' || block[0] > 'Z')) {
			if (toReturn.length() > 0) {
				Pair tup(toReturn, 1);
				block = block + 1;
				*moved += 1;
				return tup;
			} else {
				*moved += 1;
				block = block + 1;
			}
		} else if (toReturn.length() == MAX_LENGTH) {
			Pair tup(toReturn, 1);
			block = block + 1; 
			// *moved += 1;
			// return tup;
		} else {
			toReturn.append(block, 1);
			*moved += 1;
			block = block + 1;
		}
	}

	Pair tup(toReturn, 1);
	return tup;
}

int reduce(int one, int two) {
	return one + two;
}
