#include <iostream>
#include "BPlusTree.h"
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <math.h>
#include <unistd.h>
#include <algorithm>
#include <typeinfo>
#include <cstring>
#include <tuple>
#include <pthread.h>
#include <vector>
#include <immintrin.h>

using namespace std;

vector<size_t> findIndices(size_t* array, size_t query_value, int length){
    __m256i vectorArr[8];
    __m256i mask;
    __m256i compareVec;
    int mask_as_int;
    unsigned int index_offset;
    int index_counter = 0;
    vector<size_t> indices;
    //indices = (int*) malloc(50*sizeof(int));

    int full_blocks = length / 32;
    int excess_blocks = length % 32;
    int full_vectors = excess_blocks / 4;
    int excess_pieces = excess_blocks % 4;

    cout << query_value << endl;

    compareVec = _mm256_set1_epi64x(query_value);
    cout << compareVec[0] << endl;
    for(int compare_blocks = 0; compare_blocks <= full_blocks; compare_blocks++){
        //load 8 blocks of 4
        if((compare_blocks < full_blocks)){
            for(int i = 0; i < 8; i++){ //32 bytes per vector 256 bit, 
                vectorArr[i] = _mm256_load_si256( (__m256i const *) (array + 4*i + (4*8)*compare_blocks));
            }
        }else if(compare_blocks == full_blocks){
            for(int i = 0; i <= full_vectors; i++){ //32 bytes per vector 256 bit, 
                if ((i < full_vectors)){
                    vectorArr[i] = _mm256_load_si256( (__m256i const *) (array + 4*i + (4*8)*compare_blocks));
                } else if ((excess_pieces != 0)&&(i == full_vectors)){
                    int e0 = array[4*i + (4*8)*compare_blocks];
                    int e1 = 0;
                    if (excess_pieces >= 2){
                        e1 = array[4*i + (4*8)*compare_blocks + 1];
                    }
                    int e2;
                    if (excess_pieces >= 2){
                        e2 = array[4*i + (4*8)*compare_blocks + 2];
                    }
                    int e3 = 0;
                    vectorArr[i] = _mm256_setr_epi64x(e3, e2, e1, e0);
                }
            }
        }
        
        //compare all 8
        int endval = 8;
        if (compare_blocks == full_blocks){
            endval = full_vectors;
            if (excess_pieces != 0){
                endval ++;
            }
        }
        for(int i = 0; i < endval; i++){
            mask = _mm256_cmpeq_epi64(vectorArr[i], compareVec);
            mask_as_int = _mm256_movemask_pd(_mm256_castsi256_pd(mask)); //mask as int -> 0x28 [1/0 for val 3] [1/0 for val 2] [1/0 for val 1] [1/0 for val 0]
            while(mask_as_int != 0){
                index_offset = _tzcnt_u32(mask_as_int);// for 0010, tzcnt = 1, for tzcnt
                indices.push_back(compare_blocks*32 + i*4 + index_offset);
                cout << "found 1 at " << indices[index_counter] << endl;
                index_counter++;
                mask_as_int &= ~(1 << index_offset); //set the index as 0 to get next if in there
            }

        }
    }
    //_mm256_load_si256 (__m256i const * mem_addr) for load
    //__m256i _mm256_cmpeq_epi64 (__m256i a, __m256i b) for compare
    return indices;
}

struct datPack {
    char* str;
    int num_lines;
    size_t* encoded_vals;
};


struct outPack{
    map<string, size_t> temp_dict;
    size_t* encoded_vals;
};

// the function that is called for each worker thread
void* encode(void* dIn){

    // convert input back to relevant data types

   /*datPack* dat = (datPack*) dIn;
   cout << (*dat).count_start << endl;
   cout << *((*dat).teams) << endl;*/
    //cout << "e0" << endl;
    map<string, size_t> temp_dict;
    //cout << "e1" << endl;
    hash<string> hash_fn;

    stringstream a(((struct datPack*)dIn)->str);
    string t;
    size_t* local_encoded = ((struct datPack*)dIn)->encoded_vals;
    //cout << "e2" << endl;
    int i = 0;
    size_t hash;
    while ( a >> t){
        hash = hash_fn(t);
        temp_dict.insert(pair<string, size_t>(t, hash));
        local_encoded[i] = hash;
        i++;
    }
    //cout << "e3" << endl;

    outPack* out = new outPack;//(outPack*) malloc(sizeof(outPack));
    //cout << "e4" << endl;
    (*out).temp_dict = temp_dict;
    //(*out).encoded_vals = local_encoded;
    //cout << "e5" << endl;
    /*for(auto it = (*out).temp_dict.cbegin(); it != (*out).temp_dict.cend(); ++it)
{
    cout << it->first << " " << it->second << "\n";
} */

    // return compressed data pointer and size of this data
    pthread_exit(out);
}

int main(int argc, char *argv[]) {

	
    
    // read in input
    /*int num_threads = atoi(argv[1]);
    char* input_file = argv[2];
    char* output_file = argv[3];*/
    int num_threads = 400;

    // initalize tree and dictionary
    BPlusTree<string> bpt(6);
    map<string, size_t> master_dict;

    cout << "Attempting to ifstream the file" << endl;

    // get new instance of the file so that each line can be read in for its value
    ifstream txt ("Column.txt");

    cout << "Starting the first sequential read" << endl;

    //int num_lines = count(file_contents.str().begin(), file_contents.str().end(), '\n');
    int num_lines = 0;
    string line;
    while (txt >> line) {
        num_lines++;
    }

    cout << "Finished the first read, nym_lines = " << num_lines << endl;
    
    size_t* encoded_array = (size_t*) aligned_alloc(32,sizeof(size_t)*num_lines);
    int end_num = num_lines + num_threads;

    string t;
    ifstream txt2 ("Column.txt");

    // initialize the array of threads and array of data, including dummy threads
    pthread_t thread_array[num_threads];
    datPack* datArr[num_threads];
    datPack* tempPack;
    datPack* tmp;
    outPack* dummyPack;


    int index;
    int ret;

    for (int i = 0; i<num_threads; i++){
        
        datArr[i] = (datPack*) malloc(sizeof(datPack));
    }

    // loop through every line in the file
    int counter = 0;
    int elements_per_thread = 400;

    end_num = ceil(((double) num_lines)/elements_per_thread);

    cout << "Going through the file now" << endl;

    // loop through the entire streaming of the input file
    for (int count = 0; count < end_num + num_threads; count++){
        cout << "On loop " << count << "/" << end_num + num_threads << endl;

        int num_read = min(elements_per_thread, num_lines-counter);

    	// get the index of the array for this line
    	index = (count) % num_threads;

    	if (count >= num_threads){//unpack threads
            //cout << "Going to attempt to close a thread" << endl;
            ret = pthread_join(thread_array[index], (void**)&dummyPack);
            map<string, size_t> temp_dict = ((struct outPack*)dummyPack)->temp_dict;
            master_dict.insert(temp_dict.begin(), temp_dict.end());
            //cout << "Thread is closed" << endl;
        }

        string to_encode; // to_encode = the next 50 lines 
        int c = 0;
        while (txt2 >> t){

            to_encode += t + "\n";
            counter++;
            c++;
            if (c == num_read){
                break;
            }
        }

        // fill the thread back up with the new line's data

        if (count < end_num){

            //cout << "Starting making a new thread, counter is " << counter << "/" << num_lines << endl;

            datPack initpack;
            tmp = datArr[index];


            const int length = to_encode.length();
            char* char_array = new char[length + 1];
            strcpy(char_array, to_encode.c_str());

            //struct datPack *tmp = (struct datPack *)malloc(sizeof(struct datPack));

            tmp->str = char_array;
            tmp->num_lines = length;
            tmp->encoded_vals = (size_t*) (encoded_array+(counter-num_read));

            //cout << (*tempPack).teams << endl;
            
            // make the thread and add it to the correct spot of the array
            pthread_t temp_thread;
            ret = pthread_create(&temp_thread, NULL, encode, (void*)tmp);
            thread_array[index] = temp_thread;

            //cout << "Successfully made a new thread" << endl;
        }
    }


    // Go through and construct the B tree from the data
    map<string, size_t>::iterator i;
    cout << "Making B tree" << endl;
	for (i=master_dict.begin(); i!=master_dict.end(); ++i){
		bpt.insert((*i).first, (*i).second);
	}

    cout << "Finished making B tree" << endl;

    bpt.bpt_print();

    cout << "Printed B tree" << endl;

    vector<size_t> indices;
    cout << "Finding indices of " << (*master_dict.begin()).first << endl;
    Node<string>* first_element = bpt.BPlusTreeSearch(bpt.getroot(), (*master_dict.begin()).first);
    int index_of_node = bpt.find_index(first_element->item, (*master_dict.begin()).first, num_lines);
    cout << "This corresponds to " << first_element->encode[index_of_node] << endl;
    cout << (*master_dict.begin()).second << endl;
    indices = findIndices(encoded_array, first_element->encode[index_of_node], num_lines);

    return 0;
}