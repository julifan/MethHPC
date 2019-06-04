#include <string>
#include <iostream>
#include <tuple>

#include "usecase.h"

#define MAX_LENGTH 20

std::tuple<std::string, int> map(char* block, int* moved, const int totalLength) 
{
	std::string toReturn = std::string("");
	while (*moved <= totalLength) {
		if ((block[0] < 'a' || block[0] > 'z') && (block[0] < 'A' || block[0] > 'Z')) {
			if (toReturn.length() > 0) {
				std::tuple<std::string, int> tup = make_tuple(toReturn, 1);
				block = block + 1;
				*moved += 1;
				return tup;
			} else {
				*moved += 1;
				block = block + 1;
			}
		} else if (toReturn.length() == MAX_LENGTH) {
			std::tuple<std::string, int> tup = make_tuple(toReturn, 1);
			block = block + 1; 
			*moved += 1;
			return tup;
		} else {
			toReturn.append(block, 1);
			*moved += 1;
			block = block + 1;
		}
	}

	std::tuple<std::string, int> tup = make_tuple(toReturn, 1);
	return tup;
}

int reduce(int one, int two) {
	return one + two;
}
