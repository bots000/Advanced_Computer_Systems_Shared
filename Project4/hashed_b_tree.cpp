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
#include <chrono>
#include <immintrin.h>

using namespace std;

// a function used to precisely time the runtime of encoding
template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(chrono::time_point<clock_t, duration_t> const& start)
{
    return chrono::duration_cast<result_t>(clock_t::now() - start);
}

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

    compareVec = _mm256_set1_epi64x(query_value);
    //cout << compareVec[0] << endl;
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
                //cout << "found 1 at " << indices[index_counter] << endl;
                index_counter++;
                mask_as_int &= ~(1 << index_offset); //set the index as 0 to get next if in there
            }

        }
    }
    //_mm256_load_si256 (__m256i const * mem_addr) for load
    //__m256i _mm256_cmpeq_epi64 (__m256i a, __m256i b) for compare
    return indices;
}

string getEndpoint(string start){
    int len = start.length();
    string end;
    char curChar;
    char newChar;
    int i;
    for(i = len -1; i >= 0; i-1){
        curChar = start[i];
        if(curChar != 'z'){
            newChar = (char)(((int)curChar) + 1);
            //cout << "was " << curChar << " now " << newChar << endl;
            break;
        }
    }
    end = start;
    end.erase(i,len-i);
    end.push_back(newChar);
    //cout << start << " | " << end << endl;
    return end;
}

// the following two structs are used to pass values to/from the thread worker function
struct datPack {
    char* str;
    int num_lines;
    size_t* encoded_vals;
};


struct outPack{
    map<string, size_t> temp_dict;
    char* encoded_str;
    size_t* encoded_vals;
};

// the function that is called for each worker thread
void* encode(void* dIn){

    // the temp dictionary and hash_fn to be used for encoding
    map<string, size_t> temp_dict;
    hash<string> hash_fn;

    size_t* local_encoded = ((struct datPack*)dIn)->encoded_vals;
    string encoded_string = "";

    // read through and encode the values in the string
    stringstream a(((struct datPack*)dIn)->str);
    string t;
    int i = 0;
    size_t hash;
    while ( a >> t){

        // hash this string and add it to the encoded dictionary
        hash = hash_fn(t);
        temp_dict.insert(pair<string, size_t>(t, hash));

        // add the encoded value to the outfile
        local_encoded[i] = hash;
        encoded_string += to_string(hash) + "\n";
        i++;
    }

    // return the dictionary
    outPack* out = new outPack;//(outPack*) malloc(sizeof(outPack));
    (*out).temp_dict = temp_dict;

    const int length = encoded_string.length();
    char* char_array = new char[length + 1];
    strcpy(char_array, encoded_string.c_str());

    (*out).encoded_str = char_array;

    // return compressed data pointer and size of this data
    pthread_exit(out);
}

int main(int argc, char *argv[]) {

    // read in input --- ./out num_threads input_file output_file per_thread
    int num_threads = atoi(argv[1]);
    int per_thread = atoi(argv[4]);
    char* input_file = argv[2];
    char* output_file = argv[3];

    // initalize tree and dictionary
    BPlusTree<string> bpt(6);
    map<string, size_t> master_dict;

    // get new instance of the file so that each line can be read in for its value
    ifstream txt (input_file);

    //int num_lines = count(file_contents.str().begin(), file_contents.str().end(), '\n');
    cout << "Getting size" << endl;
    int num_lines = 0;
    string line;
    while (txt >> line) {
        num_lines++;
    }
    cout << "Begin Encoding" << endl;
    size_t* encoded_array = (size_t*) aligned_alloc(32,sizeof(size_t)*num_lines);
    int end_num = num_lines + num_threads;

    string t;
    ifstream txt2 (input_file);

    // initialize the array of threads and array of data, including dummy threads
    pthread_t thread_array[num_threads];
    datPack* datArr[num_threads];
    datPack* tempPack;
    datPack* tmp;
    outPack* dummyPack;

    // fill datArr
    for (int i = 0; i<num_threads; i++){
        datArr[i] = (datPack*) malloc(sizeof(datPack));
    }

    ofstream os(output_file);

    // initialize variables used in the for loop
    int index;
    int ret;

    // start timing the encoding speed
    auto start = chrono::steady_clock::now();

    // loop through every line in the file
    int counter = 0;
    end_num = ceil(((double) num_lines)/per_thread);

    // loop through the entire streaming of the input file
    for (int count = 0; count < end_num + num_threads; count++){
        
        // we will either read the number of lines specified, or the rest of the function
        int num_read = min(per_thread, num_lines-counter);

    	// get the index of the array for this line
    	index = (count) % num_threads;

    	if (count >= num_threads){//unpack threads
            
            ret = pthread_join(thread_array[index], (void**)&dummyPack);
            map<string, size_t> temp_dict = ((struct outPack*)dummyPack)->temp_dict;
            char* local_encoded_string = ((struct outPack*)dummyPack)->encoded_str;
            os << local_encoded_string;
            master_dict.insert(temp_dict.begin(), temp_dict.end());
        }

        string to_encode; // to_encode = the next num_read lines 
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

            // get the struct for the parameter for the worker thread function
            tmp = datArr[index];

            // convert our string to char*
            const int length = to_encode.length();
            char* char_array = new char[length + 1];
            strcpy(char_array, to_encode.c_str());

            // pass in the needed values to the thread worker function
            tmp->str = char_array;
            tmp->num_lines = length;
            tmp->encoded_vals = (size_t*) (encoded_array+(counter-num_read));
            
            // make the thread and add it to the correct spot of the array
            pthread_t temp_thread;
            ret = pthread_create(&temp_thread, NULL, encode, (void*)tmp);
            thread_array[index] = temp_thread;
        }
    }


    // Go through and construct the B tree from the data
    map<string, size_t>::iterator i;
	for (i=master_dict.begin(); i!=master_dict.end(); ++i){
		bpt.insert((*i).first, (*i).second);
	}

    cout << "Elapsed(s)=" << ((double) since(start).count())/1000 << endl;
    os.close();

    cout << "Begin Query Testing" << endl;
    ofstream outfile1 ("SIMDTest.csv",ofstream::binary);
    int num_characters;
    char* buffer = new char[150];
    num_characters = sprintf(buffer, "Item,Index Matches,Time\n");
    outfile1.write(buffer,num_characters);
    double total_time;
    //test searching for the first 500 values in the encoded array
    for(int test_item = 0; test_item < 500; test_item++){
        auto start1 = chrono::steady_clock::now();
        vector<size_t> matches = findIndices(encoded_array, encoded_array[test_item],num_lines);
        int num_matches = matches.size();
        total_time = ((double) since(start1).count())/1000;
        num_characters = sprintf(buffer, "%d,%d,%f\n",test_item,num_matches,total_time);
        outfile1.write (buffer,num_characters);
    }
    outfile1.close();


    cout << "Begin Prefix Testing" << endl;
    ofstream outfile2 ("PrefixTest.csv",ofstream::binary);
    num_characters;
    num_characters = sprintf(buffer, "Item,Key Matches,Index Matches,Prefix Search Time,Encode Search Time,Total Time\n");
    outfile2.write(buffer,num_characters);
    
    string match_item[10] = {"byas","woci","cide","ooihn","pikgy","synk","jahk","vuk","jja","ter"};

    double prefix_time;
    double encode_search_time;
    
    
    for(int j = 0; j < 10; j++){
        auto start1 = chrono::steady_clock::now();
        auto start3 = chrono::steady_clock::now();
        size_t matches[1000];
        int pre_matches = 0;
        int enc_matches = 0;

        cout << "Searching for prefix: " << match_item[j] << endl;
        string endstr = getEndpoint(match_item[j]);
        //cout << "tp1" << endl;
        pre_matches = bpt.simpleRange(match_item[j],endstr, matches);

        prefix_time = ((double) since(start1).count())/1000;
        //cout << "tp3" << endl;
        cout << pre_matches << endl;
        
        auto start2 = chrono::steady_clock::now();
        for(int i = 0; i < pre_matches; i++){   
            //Node<string>* element = bpt.BPlusTreeSearch(bpt.getroot(), matches[i]);
            //int index_of_node = bpt.find_index(element->item, matches[i], num_lines);
            //cout << "This corresponds to " << element->item[index_of_node] << endl;
            //cout << "p1" << endl;
            //cout << "tp4" << endl;
            vector<size_t> i_matches = findIndices(encoded_array, matches[i], num_lines);
            //cout << "tp5" << endl;
            enc_matches = enc_matches + i_matches.size();
        }
        encode_search_time = ((double) since(start2).count())/1000;
        total_time = ((double) since(start3).count())/1000;
        num_characters = sprintf(buffer, "%d,%d,%d,%f,%f,%f\n",j,pre_matches,enc_matches,prefix_time,encode_search_time,total_time);
        
        //cout << buffer << endl;
        outfile2.write (buffer,num_characters);
    }
    outfile2.close();

    cout << "Begin Naive Search Testing" << endl;
    
    ifstream txt3 (input_file);
    num_lines = 0;
    line;
    string compare_line = "kszvdc";
    
    vector<int> matches;
    auto start4 = chrono::steady_clock::now();
    while (txt3 >> line) {
        if(line == compare_line){
            matches.push_back(num_lines);
        }
        num_lines++;
    }
    cout << "Found " << matches.size() << " items in " << ((double) since(start4).count())/1000 << " seconds" <<endl;


    cout << "Begin Naive Prefix Testing" << endl;
    cout << "Testing prefix byas" << endl;
    
    ifstream txt4 (input_file);
    num_lines = 0;
    string start_str = match_item[0];
    string end_str = getEndpoint(start_str);
    auto start5 = chrono::steady_clock::now();
    while (txt4 >> line) {
        if((line >= start_str)&&(line <= end_str)){
            matches.push_back(num_lines);
        }
        num_lines++;
    }
    cout << "Found " << matches.size() << " items in " << ((double) since(start5).count())/1000 << " seconds" <<endl;

    cout << "Testing prefix vuk" << endl;
    ifstream txt5 (input_file);
    num_lines = 0;
    start_str = match_item[7];
    end_str = getEndpoint(start_str);
    auto start6 = chrono::steady_clock::now();
    while (txt5 >> line) {
        if((line >= start_str)&&(line <= end_str)){
            matches.push_back(num_lines);
        }
        num_lines++;
    }
    cout << "Found " << matches.size() << " items in " << ((double) since(start6).count())/1000 << " seconds" <<endl;

    // now we need to write the encoded dictionary and encoded values to a file
    bpt.bpt_write(output_file);

    // this is some dynamic way to search for the indices of the first value in master_dict
    /*vector<size_t> indices;
    Node<string>* first_element = bpt.BPlusTreeSearch(bpt.getroot(), (*master_dict.begin()).first);
    int index_of_node = bpt.find_index(first_element->item, (*master_dict.begin()).first, num_lines);
    indices = findIndices(encoded_array, first_element->encode[index_of_node], num_lines);*/

    // this will find all data items with given prefix by seaching the tree to prefix:next ascii value up from prefix


    /*string prefix = "by";
    char last_part = prefix.back();
    int last_ascii_up_one = int(last_part) + 1;
    char new_up_one_char = char(last_ascii_up_one);
    string end_prefix = prefix;
    end_prefix.back() = new_up_one_char;

    int arr_length = num_lines;
    string results[arr_length] = {};
    int useless = bpt.range_search(prefix, end_prefix, results, arr_length);
    cout << results << endl;
    for (int i = 0; i < arr_length; i++){

        remove(results[i].begin(), results[i].end(), ' ');
        remove(results[i].begin(), results[i].end(), '\n');
        if (results[i].length() > 0){
            cout << results[i] << endl;
        }
        
    }*/


    return 0;
}