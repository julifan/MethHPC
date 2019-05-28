#include "backend.h"
#include "usecase.h"
#include "hash.h"

#include <iostream>
#include <unordered_map>

#include <sstream>

char* input;
int length;

MPI_File outputFile;


struct Config {
	char* input;
	int length;
};

struct Config config;

void init(char* input, char* output) {
	//MPI init
	//set output file
	//read from given input file (if rank == 0), 
	//scatter data to map's of processes (if rank == 0)

	int world_size, world_rank;
	
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	MPI_File file;
	
	MPI_Offset read_pointer = 0;
	MPI_Offset file_size = 0;
	
	MPI_File_open(MPI_COMM_WORLD, input, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
	MPI_File_get_size(file, &file_size);
	
	
	int chunk_size = 64;
	char* chunk = new char[chunk_size / sizeof(char)];
	
	MPI_Datatype chunk_type;
	
	MPI_Type_contiguous(chunk_size, MPI_CHAR, &chunk_type);
	MPI_Type_commit(&chunk_type);
	
	
	while(read_pointer < file_size)
	{
		int read_size;
		int disp;
		if(read_pointer + world_size * chunk_size < file_size) {
			read_size = chunk_size;
			disp = read_pointer + world_rank * read_size;
		}
		else {
			int leftover = file_size - read_pointer;
			read_size = leftover / world_size;
			if(world_rank < leftover - read_size * world_size) {
				read_size++;
				disp = read_pointer + world_rank * read_size;
			}
			else {
				disp = read_pointer + (leftover - read_size * world_size) * (read_size + 1) + (world_rank - (leftover - read_size * world_size)) * read_size;
			}
			MPI_Type_contiguous(read_size, MPI_CHAR, &chunk_type);
			MPI_Type_commit(&chunk_type);
	
		}
		std::cout << "filesize: " << file_size << std::endl;
		std::cout << world_rank << ": " << disp << std::endl;
		std::cout << world_rank << ": " << read_size << std::endl;
		MPI_File_set_view(file, disp, MPI_CHAR, chunk_type, "native", MPI_INFO_NULL);
		MPI_File_read_all(file, chunk, read_size, MPI_CHAR, MPI_STATUS_IGNORE);
		
		read_pointer += chunk_size * world_size;
		
		
		std::string str(chunk, read_size);
	
		std::cout << "rank " << world_rank << ": " << str << std::endl;
		
		
		
	}
	
	
	
}



void mapReduce() {
	//call map() from usecase
	//redistribute
	//reduce
	//write to file (if rank == 0)
	
	char* input = config.input;
	int length = config.length;
	
	
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::string str(input, length);
	
	std::cout << "rank " << rank << ": " << length <<  ", " << str << std::endl;
	
	std::vector<std::tuple<std::string, int > > buckets[size];

	//call map() from usecase

	
	int mv = 0;
	int * moved = &mv;
	
	char* current_input = input;
	int remaining = length;

	//assumption: input and length are initialized.
	while (remaining > 0) {
		std::tuple<std::string, int> tup = map(current_input, moved, remaining);
		if (std::get<0>(tup) != "") {
		//TODO probably pad strings? (to achieve constant length)
		Hash hash = getHash(std::get<0>(tup).c_str(), std::get<0>(tup).length());
		buckets[hash % size].push_back(tup);

//std::cout << "Tupel: " << std::get<0>(tup) << " bucket: " << std::endl;//hash % size << " Hash: " << hash << std::endl; 
		}
		
		current_input += *moved; 
		remaining -= *moved;
		mv = 0;
	}

	std::cout << rank << ": finished mapping" << std::endl;
		
	int num_keys =  0;
	
	
	int* bucket_sizes = new int[size];
	int* bucket_num_chars = new int[size];
	int* char_displs = new int[size];
	int* bucket_displs = new int[size];
	int num_chars = 0;
	
	int* key_offsets;
	int* key_lengths;
	
	/*
	int* value_offsets;
	int* value_lengths;
	*/
	
	char* chars;
	int* values;
	

	for(int i = 0; i < size; i++)
	{
		bucket_sizes[i] =  buckets[i].size();
		bucket_displs[i] = num_keys;
		num_keys += bucket_sizes[i];
		
		bucket_num_chars[i] = 0;
		char_displs[i] = num_chars;
		for(int j = 0; j < bucket_sizes[i]; j++)
		{
			bucket_num_chars[i] += std::get<0>(buckets[i][j]).size();
		}
		num_chars += bucket_num_chars[i];
		
	}
	
	key_offsets = new int[num_keys];
	key_lengths = new int[num_keys];
	chars = new char[num_chars];
	
	values = new int[num_keys];
	
	std::ostringstream ss;
	ss << rank << ", keys: ";
	
	int i_key = 0;
	char* current_key = chars;
	int* current_value = values;
	
	int offset = 0;
	
	for(int i = 0; i < size; i++) {
		for(int j = 0; j < bucket_sizes[i]; j++) {
			key_offsets[i_key] = offset;
			std::string key = std::get<0>(buckets[i][j]);
			int value = std::get<1>(buckets[i][j]);
			
			key.copy(current_key, key.size());
			key_lengths[i_key] = key.size(); 
			
			*current_value = value;
			
			ss << key << " ";
			
			current_key += key.size();
			offset += key.size();
			
			i_key++;
			current_value++;
		}
	}
	
	std::cout << ss.str() << std::endl;
	
	
	int* recv_bucket_num_chars = new int[size];
	int* recv_char_displs = new int[size];
	char* recv_chars;
	
	int* recv_bucket_sizes = new int[size];
	
	int recv_num_keys;
	int* recv_key_lengths;
	int* recv_bucket_displs;
	
	int* recv_values;
	
	
	MPI_Alltoall(bucket_num_chars, 1, MPI_INT, recv_bucket_num_chars, 1, MPI_INT, MPI_COMM_WORLD);
	
	
	
	int recv_num_chars = 0;
	for(int i = 0; i < size; i++) {
		recv_char_displs[i] = recv_num_chars; 
		recv_num_chars += recv_bucket_num_chars[i];
	}
	
	recv_chars = new char[recv_num_chars];
	
	MPI_Alltoallv(chars, bucket_num_chars, char_displs, MPI_CHAR, recv_chars, recv_bucket_num_chars, recv_char_displs, MPI_CHAR, MPI_COMM_WORLD);

	
	MPI_Alltoall(bucket_sizes, 1, MPI_INT, recv_bucket_sizes, 1, MPI_INT, MPI_COMM_WORLD);
	

		
	recv_num_keys = 0;
	recv_bucket_displs = new int[size];
	for(int i = 0; i < size; i++) {
		recv_bucket_displs[i] = recv_num_keys;
		recv_num_keys += recv_bucket_sizes[i];
	}
	recv_key_lengths = new int[recv_num_keys];
	
	MPI_Alltoallv(key_lengths, bucket_sizes, bucket_displs, MPI_INT, recv_key_lengths, recv_bucket_sizes, recv_bucket_displs, MPI_INT, MPI_COMM_WORLD);
	
	
	recv_values = new int[recv_num_keys];
	
	
	MPI_Alltoallv(values, bucket_sizes, bucket_displs, MPI_INT, recv_values, recv_bucket_sizes, recv_bucket_displs, MPI_INT, MPI_COMM_WORLD);
	
	

	
	
	std::vector<std::tuple<std::string, int > > key_value_pairs_received;
	
	current_key = recv_chars;
	current_value = recv_values;
	
	std::ostringstream ss_recv;
	ss_recv << rank << ", keys_recv: ";
	for(int i = 0; i < recv_num_keys; i++)
	{
		int key_length = recv_key_lengths[i];
		// std::cout << rank << ", recv length: " << key_length << std::endl;
		std::string key(current_key, key_length);
		ss_recv << key;
		
		int value = *recv_values;
		ss_recv << ":" << value << " ";
		
		key_value_pairs_received.push_back(make_tuple(key, value));
		current_key += key_length;
		recv_values++;
	}
	std::cout << ss_recv.str() << std::endl;
	
	
	
	std::vector<std::tuple<std::string, int>>& received = key_value_pairs_received;
	
	//====================================
	//		Reduce
	//====================================
	
	/*
	std::vector<std::tuple<std::string, int> > received[3];
	//std::tuple<std::string, int> tup = std::make_tuple("abc", 1);
	received[0].push_back(std::make_tuple("abc", 1));
	received[0].push_back(std::make_tuple("deee", 1));
	received[0].push_back(std::make_tuple("ddd", 2));
	received[1].push_back(std::make_tuple("abc", 3));
	*/
	//assumption: tuples are in received[size]
	std::unordered_map<std::string, int> map;
	//TODO change 3 to size XD
	for (int i = 0; i < received.size(); i++) {
		std::tuple<std::string, int> tup = received[i];
		if (map.find(std::get<0>(tup)) == map.end()) {
			map.insert(make_pair(std::get<0>(tup), std::get<1>(tup)));
			
		} else {
			map.find(std::get<0>(tup))->second = reduce(map.find(std::get<0>(tup))->second, std::get<1>(tup));	
		}
	}

	std::unordered_map<std::string, int>:: iterator itr; 
	std::cout << "\nAll Elements : \n"; 
	for (itr = map.begin(); itr != map.end(); itr++) 
	{
		std::cout << itr->first << "  " << itr->second << std::endl; 
	} 


	//output will be stored in output.


	//write to file (if rank == 0)


}


void cleanup() {
	//free everything
	//MPI_Finalize
}

