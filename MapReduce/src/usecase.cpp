#include <string>
#include <iostream>
#include <unordered_map>

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

std::tuple<std::string, int> reduce(std::tuple<std::string, int> one, std::tuple<std::string, int> two) {
	std::tuple<std::string, int> tup = make_tuple(std::get<0>(one), std::get<1>(one) + std::get<1>(two));
	return tup;
}
