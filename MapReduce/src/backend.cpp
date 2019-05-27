#include "backend.h"
#include "usecase.h"
#include "hash.h"

#include <iostream>

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
	
	
	int read_buffer_size = 64;
	
	
	char* read_buffer = new char[read_buffer_size];
	
	if(world_rank == 0) 
	{
		MPI_File_open(MPI_COMM_SELF, input, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
		MPI_File_get_size(file, &file_size);
	}
	MPI_Bcast(&file_size, 1, MPI_OFFSET, 0, MPI_COMM_WORLD);
	
	
	int map_buffer_size = file_size / world_size;
	
	if(world_rank <= file_size - map_buffer_size * world_size) {
		map_buffer_size++;
	}
	
	
	
	char* map_buffer = new char[map_buffer_size];
	int map_buffer_offset = 0;
	
	
	while(read_pointer < file_size)
	{
		
		int read_size = read_buffer_size;
		if(read_pointer + read_size > file_size) {
			read_size = file_size - read_pointer;
		}
		
		if(world_rank == 0)
		{
			MPI_File_read_at(file, read_pointer, read_buffer, read_size, MPI_CHAR, MPI_STATUS_IGNORE);
		}
		
		int map_buffer_read_size = read_size / world_size;
		
		if(world_rank < read_size - map_buffer_read_size * world_size) {
			map_buffer_read_size++;
		}
		
		int* sizes = new int[world_size];
		int* offsets = new int[world_size];
		
		
		sizes[world_rank] = map_buffer_read_size;
		for(int i = 0; i < world_size; i++)
		{
			MPI_Bcast(sizes + i, 1, MPI_INT, i, MPI_COMM_WORLD);
		}
		int offset = 0;
		for(int i = 0; i < world_size; i++)
		{
			offsets[i] = offset;
			offset += sizes[i];
		}
		
		
		MPI_Scatterv(read_buffer, sizes, offsets, MPI_CHAR, map_buffer + map_buffer_offset, map_buffer_read_size, MPI_CHAR, 0, MPI_COMM_WORLD);
		map_buffer_offset += map_buffer_read_size;
		
		
		delete[] sizes;
		delete[] offsets;
		
		read_pointer += read_buffer_size * sizeof(char);
	}

	delete[] read_buffer;
	
	config.length = map_buffer_size;
	config.input = map_buffer;
	
	
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
		
// std::cout << "Tupel: " << std::get<0>(tup) << " bucket: " << std::endl;//hash % size << " Hash: " << hash << std::endl; 

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
	int* key_format_displs = new int[size];
	int num_chars = 0;
	
	int* key_offsets;
	int* key_lengths;
	char* chars;
	

	for(int i = 0; i < size; i++)
	{
		bucket_sizes[i] =  buckets[i].size();
		key_format_displs[i] = num_keys;
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
	
	std::ostringstream ss;
	ss << rank << ", keys: ";
	
	int i_key = 0;
	char* current = chars;
	int offset = 0;
	for(int i = 0; i < size; i++) {
		for(int j = 0; j < bucket_sizes[i]; j++) {
			key_offsets[i_key] = offset;
			std::string s = std::get<0>(buckets[i][j]);
			
			s.copy(current, s.size());
			key_lengths[i_key] = s.size(); 
			
			ss << s << " ";
			
			current += s.size();
			offset += s.size();
			
			i_key++;
		}
	}
	
	std::cout << ss.str() << std::endl;
	
	
	int* recv_bucket_num_chars = new int[size];
	int* recv_char_displs = new int[size];
	char* recv_chars;
	
	int* recv_bucket_sizes = new int[size];
	
	int recv_num_keys;
	int* recv_key_lengths;
	int* recv_key_format_displs;
	
	
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
	recv_key_format_displs = new int[size];
	for(int i = 0; i < size; i++) {
		recv_key_format_displs[i] = recv_num_keys;
		recv_num_keys += recv_bucket_sizes[i];
	}
	recv_key_lengths = new int[recv_num_keys];
	
	MPI_Alltoallv(key_lengths, bucket_sizes, key_format_displs, MPI_INT, recv_key_lengths, recv_bucket_sizes, recv_key_format_displs, MPI_INT, MPI_COMM_WORLD);

	std::vector<std::string> keys;
	current = recv_chars;
	
	std::ostringstream ss_recv;
	ss_recv << rank << ", keys_recv: ";
	for(int i = 0; i < recv_num_keys; i++)
	{
		int key_length = recv_key_lengths[i];
		// std::cout << rank << ", recv length: " << key_length << std::endl;
		std::string s(current, key_length);
		ss_recv << s << " ";
		keys.push_back(s);
		current += key_length;
		
	}
	std::cout << ss_recv.str() << std::endl;
	
	
	
	

	
	//redistribute
	//reduce
	//write to file (if rank == 0)


}


void cleanup() {
	//free everything
	//MPI_Finalize
}

