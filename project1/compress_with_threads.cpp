#include <iostream>
#include <zstd.h>
#include <pthread.h>
#include <cstring>
#include <fstream>
#include <math.h>
#include "common.h" 
#define BSIZE 16384
using namespace std;

// There needs to be some function called by the thread creation that will run some compression

struct datPack{    
    void* fBuff;
    int len;
    void* cBuff;
    size_t cSize;
};

struct outPack{
    void* cBuff;
    size_t cSize;
};

void* compress_data(void* dIn){
    datPack* dat = (datPack*) dIn;
    size_t cBuffSize = ZSTD_compressBound((*dat).len);
    
    
    void* cBuff = malloc_orDie(cBuffSize);
    
    (*dat).cBuff = cBuff;
    
    size_t cSize = ZSTD_compress(cBuff, cBuffSize, (*dat).fBuff, (*dat).len, 1);
    free(((*dat).fBuff));
    
    CHECK_ZSTD(cSize);
    (*dat).cSize = cSize;
    pthread_exit((void*) dat);
}

int main(int argc, char *argv[]) {
    
    // declare variables
    int num_threads = atoi(argv[1]);
    char* input_file = argv[2];
    char* output_file = argv[3];
    num_threads = num_threads-1;

    //string input_file = "ACS_P1_source_file";
    //string output_file = "ACS_P1_compressed_file";

    pthread_t thread_array[num_threads];
    datPack datArr[num_threads];
 
    long counter;
    long end_num;
    int len;
    int index;
    int ret;

    // open the source file, get the length
    ifstream file (input_file);
    ofstream outfile (output_file);
    char * buffer;
    file.seekg(0, ios::end);
    long file_size = file.tellg();
    file.seekg (0, ios::beg);

    //holder for every datpack
    datPack* tempPack;
    datPack* dummyPack;
    for (int i = 0; i<num_threads; i++){
        datPack initPack;
        datArr[i] = initPack;
    }

    end_num = ceil(((double) file_size)/BSIZE);
    for (counter = 0; counter < end_num + num_threads; counter++){

        if (counter + num_threads == end_num){ len = file_size % BSIZE;
        } else{ len = BSIZE;}

        index = counter % num_threads;

        if (counter >= num_threads){//unpack threads
            //set new tempPack 
            //tempPack = datArr[index];
            //join due thread
            ret = pthread_join(thread_array[index], (void**)&dummyPack);
            char* ret_join2 = (char*) (*dummyPack).cBuff;
            //cout << (*tempPack).cSize << endl;
            //cout << (*dummyPack).cSize << endl;
            // write out these compressed contents to the output file
            outfile.write((char*)(*dummyPack).cBuff, (*dummyPack).cSize);
            free(((*dummyPack).cBuff));
            //free((*dOut).cBuff);
        }

        if (counter <= end_num){ //make threads when not at end
            buffer = new char [len];
            //set new tempPack and free past fBuff
            tempPack = &(datArr[index]);
            //free((*tempPack).fBuff);
            //package data into tempPack
            (*tempPack).fBuff = buffer;
            (*tempPack).len = len;
            file.read(buffer, len);

            // make the thread and add it to the correct spot of the array
            pthread_t temp_thread;
            ret = pthread_create(&temp_thread, NULL, compress_data, tempPack);
            thread_array[index] = temp_thread;
        }
        

    }
    return 0;
}