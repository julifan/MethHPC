#include <string>
#include <iostream>
#include <regex>
#include <unordered_map>

#include "usecase.h"

void map(char* block, int length) 
{
	
	std::unordered_map<std::string, int> map;


	std::regex r(R"([^\W_]+(?:['_-][^\W_]+)*)");
	std::string s = std::string(block, length); //"Hello, world! Aren't you clever? 'Later', she said. Maybe 5 o'clock?' In the year 2017 ... G2g, cya l8r hello_world.h Hermione's time-turner. Good mor~+%g. Hi' Testing_ Bye- The kids' toys toys hello_world ";
	for(std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r);
                         i != std::sregex_iterator();
                         ++i)
	{
	    std::smatch m = *i;
	    std::cout << m.str() << '\n';
		if (map.find(m.str()) == map.end()) {
			map.insert(make_pair(m.str(), 1));		
		} else {
			map.find(m.str())->second += 1;
		}
}



//    iterating over all values of map 
    std::unordered_map<std::string, int>:: iterator itr; 
    std::cout << "\nAll Elements : \n"; 
    for (itr = map.begin(); itr != map.end(); itr++) 
    {
        std::cout << itr->first << "  " << itr->second << std::endl; 
    } 
}

void reduce() {



}
