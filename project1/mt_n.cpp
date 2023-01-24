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
    outPack* datO = (outPack *) malloc(sizeof(outPack));
    (*datO).cBuff = cBuff;
    //cout << length << endl;
    //cout << cBuffSize << endl;
    size_t cSize = ZSTD_compress(cBuff, cBuffSize, (*dat).fBuff, (*dat).len, 1);
    free((*dat).fBuff);
    cout << cSize << endl;
    CHECK_ZSTD(cSize);
    (*datO).cSize = cSize;
    pthread_exit((void*) datO);
}

int main(int num_threads) {
    
    // declare variables
    //int num_threads;
    //num_threads = 4;
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
    ifstream file ("ubuntu.iso");
    ofstream outfile ("ubuntu.iso.zst");
    char * buffer;
    file.seekg(0, ios::end);
    long file_size = file.tellg();
    file.seekg (0, ios::beg);

    end_num = ceil(((double) file_size)/BSIZE);
    cout << end_num<< endl;
    for (counter = 0; counter <= end_num; counter++){

        if (counter + num_threads == end_num){ len = file_size % BSIZE;
        } else{ len = BSIZE;}

        index = counter % num_threads;

        if (counter >= num_threads){//unpack threads
            outPack* dOut;
            ret = pthread_join(thread_array[index], (void**)&dOut);
            char* ret_join2 = (char*) (*dOut).cBuff;
            cout << (*dOut).cSize << endl;
            // write out these compressed contents to the output file
            outfile.write((char*)(*dOut).cBuff, (*dOut).cSize);
            free((*dOut).cBuff);
        }

        if (counter <= end_num-num_threads){ //make threads when not at end
            buffer = new char [len];
            datPack dIn = datArr[index];
            dIn.fBuff = buffer;
            dIn.len = len;
            file.read(buffer, len);

            // make the thread and add it to the correct spot of the array
            pthread_t temp_thread;
            ret = pthread_create(&temp_thread, NULL, compress_data, &dIn);
            thread_array[index] = temp_thread;
        }
        

    }
    return 0;
}
