#include "backend.h"
#include "usecase.h"
#include "hash.h"

#include <vector>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <string.h>
#include <assert.h>


#include <unordered_map>

struct Config
{
	char* input;
	char* output;
	MPI_File outputFile;
};

struct Config config;

void init(char* input, char* output) {

	config.input = input;
	config.output = output;
	
}


void mapChunks(char* input, int length, std::unordered_map<std::string, int>* buckets) {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int MAX_THREADS = omp_get_max_threads();
	std::vector<std::tuple<std::string, int> > localValues[size][MAX_THREADS];

	#pragma omp parallel for
	for(int i = 0; i < omp_get_num_threads(); i++) {
		int tid = omp_get_thread_num();
		int nt = omp_get_num_threads();

		char * current_input = input;
		current_input += (length / nt) * tid;
		int remaining = length / nt;

		if (tid == nt - 1) {
			int missing = length - (nt * (length / nt));
			remaining += missing;
		int mv = 0;
		int* moved = &mv; 
		
		while (remaining > 0) {
			std::tuple<std::string, int> tup = map(current_input, moved, remaining);
			if (std::get<0>(tup) != "") {
				Hash hash = getHash(std::get<0>(tup).c_str(), std::get<0>(tup).length());
				int procNo = hash % size;
				localValues[procNo][tid].push_back(tup);
				
				int size = localValues[procNo][tid].size();
			}
			current_input += *moved; 
			remaining -= *moved;
			mv = 0;
		}
	}

	#pragma omp parallel for
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < MAX_THREADS; j++) {
			for (int k = 0; k < localValues[i][j].size(); k++) {
				std::tuple<std::string, int> tup = localValues[i][j].at(k);
				if (buckets[i].find(std::get<0>(tup)) != buckets[i].end()) {
					//key already exists. reduce locally
					buckets[i].find(std::get<0>(tup))->second = reduce(buckets[i].find(std::get<0>(tup))->second, std::get<1>(tup));
				} else {
					buckets[i].insert(make_pair(std::get<0>(tup), std::get<1>(tup)));
				}
			}
		}
	}
}


void mapReduce() {
	
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	std::unordered_map<std::string, int>* buckets = new std::unordered_map<std::string, int>[size];
	
	
	//==============================================
	//		Read and Map input
	//==============================================
	{
		MPI_File file;
		
		MPI_Offset file_size = 0;
		
		MPI_File_open(MPI_COMM_WORLD, config.input, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
		MPI_File_get_size(file, &file_size);
		
		
		uint64_t chunk_size = 67108864;
		char* chunk = new char[chunk_size / sizeof(char)];
		
		MPI_Datatype chunk_type, file_type;
		MPI_Datatype reduced_chunk_type;
		
		MPI_Type_contiguous(chunk_size, MPI_CHAR, &chunk_type);
		MPI_Type_create_resized(chunk_type, 0, size * chunk_size, &file_type);
		MPI_Type_commit(&file_type);
		
		uint64_t disp = rank * chunk_size;
		MPI_File_set_view(file, disp, MPI_CHAR, file_type, "native", MPI_INFO_NULL);
		
		uint64_t full_iterations = file_size / (size * chunk_size);
		uint64_t leftover = file_size - full_iterations * (size * chunk_size);
		
		for(int i = 0; i < full_iterations; i++) {
			MPI_File_read_all(file, chunk, chunk_size, MPI_CHAR, MPI_STATUS_IGNORE);
			
			mapChunks(chunk, chunk_size, buckets);
		}
		if(leftover > 0 ) {
			uint64_t read_size = leftover / size;
			if(rank < leftover - read_size * size) {
				read_size++;
				disp = full_iterations * size * chunk_size + rank * read_size;
			}
			else {
				disp = full_iterations * size * chunk_size + (leftover - read_size * size) * (read_size + 1) + (rank - (leftover - read_size * size)) * read_size;
			}
			MPI_Type_contiguous(read_size, MPI_CHAR, &reduced_chunk_type);
			MPI_Type_commit(&reduced_chunk_type);
			MPI_File_set_view(file, disp, MPI_CHAR, reduced_chunk_type, "native", MPI_INFO_NULL);
			MPI_File_read_all(file, chunk, read_size, MPI_CHAR, MPI_STATUS_IGNORE);
			mapChunks(chunk, read_size, buckets);
		}

		MPI_File_close(&file);
		delete[] chunk;
	
	}
	
	//==============================================
	//		Redistribute Data
	//==============================================
	std::vector<Pair > key_value_pairs_received;
	
	{
		int num_keys =  0;
		int* bucket_sizes = new int[size];
		int* bucket_displs = new int[size];
		
		int num_chars = 0;
		int* bucket_num_chars = new int[size];
		int* char_displs = new int[size];
		
		int* key_lengths;
	
		char* chars;
		int* values;
		
		
		int recv_num_keys = 0;
		int* recv_bucket_sizes = new int[size];
		int* recv_bucket_displs  = new int[size];;
		
		
		int recv_num_chars = 0;
		int* recv_bucket_num_chars = new int[size];
		int* recv_char_displs = new int[size];
		
		
		int* recv_key_lengths;
		
		
		char* recv_chars;
		int* recv_values;
		
		
		
		for(int i = 0; i < size; i++)
		{
			bucket_sizes[i] =  buckets[i].size();
			bucket_displs[i] = num_keys;
			num_keys += bucket_sizes[i];
			
			bucket_num_chars[i] = 0;
			char_displs[i] = num_chars;
			
			std::unordered_map<std::string, int>:: iterator itr; 
			for (itr = buckets[i].begin(); itr != buckets[i].end(); itr++) 
			{
				bucket_num_chars[i] += itr->first.size();
			} 
			num_chars += bucket_num_chars[i];
			
		}
		
		key_lengths = new int[num_keys];
		chars = new char[num_chars];
		
		values = new int[num_keys];
		
		{
			int i_key = 0;
			char* current_key = chars;
			int* current_value = values;
			for(int i = 0; i < size; i++) {
				std::unordered_map<std::string, int>::iterator itr;
				for(itr = buckets[i].begin(); itr != buckets[i].end(); itr++) {
					std::string key = itr->first;
					int value = itr->second;
					
					key.copy(current_key, key.size());
					
					
					key_lengths[i_key] = key.size(); 
					
					*current_value = value;
					
					current_key += key.size();
					
					i_key++;
					current_value++;
				}
			}
		}
		
		delete[] buckets;
		
		MPI_Alltoall(bucket_num_chars, 1, MPI_INT, recv_bucket_num_chars, 1, MPI_INT, MPI_COMM_WORLD);
		
		MPI_Alltoall(bucket_sizes, 1, MPI_INT, recv_bucket_sizes, 1, MPI_INT, MPI_COMM_WORLD);
		
		
		
		for(int i = 0; i < size; i++) {
			recv_char_displs[i] = recv_num_chars; 
			recv_num_chars += recv_bucket_num_chars[i];
		}
		recv_num_keys = 0;
		for(int i = 0; i < size; i++) {
			recv_bucket_displs[i] = recv_num_keys;
			recv_num_keys += recv_bucket_sizes[i];
		}
		
		recv_chars = new char[recv_num_chars];
		recv_key_lengths = new int[recv_num_keys];
		recv_values = new int[recv_num_keys];
		
		
		
		MPI_Alltoallv(chars, bucket_num_chars, char_displs, MPI_CHAR, recv_chars, recv_bucket_num_chars, recv_char_displs, MPI_CHAR, MPI_COMM_WORLD);
		
		MPI_Alltoallv(key_lengths, bucket_sizes, bucket_displs, MPI_INT, recv_key_lengths, recv_bucket_sizes, recv_bucket_displs, MPI_INT, MPI_COMM_WORLD);
		
		MPI_Alltoallv(values, bucket_sizes, bucket_displs, MPI_INT, recv_values, recv_bucket_sizes, recv_bucket_displs, MPI_INT, MPI_COMM_WORLD);
	
		delete[] chars;
		delete[] values;
		delete[] bucket_sizes;
		delete[] bucket_displs;
		delete[] bucket_num_chars;
		delete[] char_displs;
		delete[] key_lengths;
		
		
		{
			// pointers at the current data
			char* current_key = recv_chars;
			
			// 
			for(int i = 0; i < recv_num_keys; i++)
			{
				int key_length = recv_key_lengths[i];
				std::string key(current_key, key_length);
				
				int value = recv_values[i];
				
				key_value_pairs_received.push_back(Pair(key, value));
				current_key += key_length;
			}
		}
		delete[] recv_chars;
		delete[] recv_values;
		delete[] recv_bucket_sizes;
		delete[] recv_bucket_displs;
		delete[] recv_bucket_num_chars;
		delete[] recv_char_displs;
		delete[] recv_key_lengths;
	}
	
	std::vector<Pair>& received = key_value_pairs_received;
	
	//====================================
	//		Reduce
	//====================================
	
	std::unordered_map<std::string, int> map;
	
	for (int i = 0; i < received.size(); i++) {
		Pair tup = received[i];
		if (map.find(tup.key) == map.end()) {
			map.insert(make_pair(tup.key, tup.value));
			
		} else {
			map.find(tup.key)->second = reduce(map.find(tup.key)->second, tup.value);	
		}
	}
	
	std::unordered_map<std::string, int>:: iterator itr; 

	//convert map to string
	std::string toWrite = "";
	for (itr = map.begin(); itr != map.end(); itr++) {
		toWrite.append(itr->first);
		toWrite.append(" ");
		toWrite.append(std::to_string(itr->second));
		//toWrite.append(", ");
		toWrite.append("\n");
	}

	char toWriteChar[toWrite.length() + 1];
	strcpy(toWriteChar, toWrite.c_str());

	int pre = 0; 
	int* prefix = &pre;

	int localLength = toWrite.length();
	int* localLengthPtr = &localLength;
	
	MPI_Exscan(localLengthPtr, prefix, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
	MPI_File_open(MPI_COMM_WORLD, config.output, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &config.outputFile);
	MPI_File_set_view(config.outputFile, *prefix, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
	MPI_File_write_all(config.outputFile, toWriteChar, localLength, MPI_CHAR, MPI_STATUS_IGNORE);
	MPI_File_close(&config.outputFile);

}


void cleanup() {
	//free everything
}

