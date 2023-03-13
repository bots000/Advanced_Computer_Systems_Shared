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
#include <immintrin.h>

using namespace std;

int* findIndeces(size_t* array){
    //_mm256_load_si256 (__m256i const * mem_addr) for load
    //__m256i _mm256_cmpeq_epi64 (__m256i a, __m256i b) for compare
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
    int num_threads = 8;

    // initalize tree and dictionary
    BPlusTree<string> bpt(6);
    map<string, size_t> master_dict;

    // get new instance of the file so that each line can be read in for its value
    ifstream txt ("rand_teams.txt");
    stringstream file_contents;
    file_contents << txt.rdbuf();

    //int num_lines = count(file_contents.str().begin(), file_contents.str().end(), '\n');
    int num_lines = 0;
    for (int i = 0; (i = file_contents.str().find('\n', i)) != string::npos; i++) {
        num_lines++;
    }
    
    size_t* encoded_array = (size_t*) aligned_alloc(32,sizeof(size_t)*num_lines);
    int end_num = num_lines + num_threads;

    string t;
    ifstream txt2 ("rand_teams.txt");

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

    end_num = ceil(((double) num_lines)/50);

    // loop through the entire streaming of the input file
    for (int count = 0; count < end_num + num_threads; count++){

        int num_read = min(50, num_lines-counter);

    	// get the index of the array for this line
    	index = (count) % num_threads;

    	if (count >= num_threads){//unpack threads
            ret = pthread_join(thread_array[index], (void**)&dummyPack);
            map<string, size_t> temp_dict = ((struct outPack*)dummyPack)->temp_dict;
            master_dict.insert(temp_dict.begin(), temp_dict.end());
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
        }

        if (counter == end_num){
            break;
        }
    }


    // Go through and construct the B tree from the data
    map<string, size_t>::iterator i;
	for (i=master_dict.begin(); i!=master_dict.end(); ++i){
		bpt.insert((*i).first, (*i).second);
	}

    bpt.bpt_print();

    return 0;
}