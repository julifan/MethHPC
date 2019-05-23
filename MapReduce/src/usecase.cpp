#include <string>
#include <iostream>
#include <regex>
#include <unordered_map>

#include "usecase.h"

std::tuple<std::string, int> map(char* block, int* moved, const int totalLength) 
{

	std::string toReturn = std::string("");
	while (*moved <= totalLength) {
		if (((block[0] < 'a' || block[0] > 'z') && (block[0] < 'A' || block[0] > 'Z'))
			|| *moved + 1 > totalLength) {
			//std::cout << "not appended" << std::endl;
			if (toReturn.length() > 0) {
				std::tuple<std::string, int> tup = make_tuple(toReturn, 1);
	    			//std::cout << "Separated string: " << toReturn << " length: " << toReturn.length() << std::endl;
				block = block + 1;
				*moved += 1;
				//std::cout << "moved: " << *moved << std::endl;
				return tup;
			} else {
				*moved += 1;
				block = block + 1;
			}
		} else {
			toReturn.append(block, 1);
			*moved += 1;
			//std::cout << "Appended: " << toReturn << std::endl;
			block = block + 1;
		}
	}

	//TODO: include maximum length of words

	/*std::unordered_map<std::string, int> map;
	std::regex r(R"([^\W_]+(?:['_-][^\W_]+)*)");
	std::string s = std::string(block);
	std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r);
	//if (i != std::sregex_iterator()) {
	    std::smatch m = *i;
	    std::tuple<std::string, int> tup = make_tuple(m.str(), 1);
	    std::cout << "Separated string: " << m.str() << " length: " << m.str().length() << std::endl;
	    std::cout << "Whole block: " << s << std::endl; 
	    return tup;*/
	//}
}

void reduce(std::unordered_map<std::string, int> * map) {



}
