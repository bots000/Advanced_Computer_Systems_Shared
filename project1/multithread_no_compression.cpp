#include <iostream>
#include <zstd.h>
#include <pthread.h>
#include <cstring>
#include <fstream>
using namespace std;

// There needs to be some function called by the thread creation that will run some compression

void *worker_thread(void *buffer){
    // provided that data is a 16 KB block, the zstd library needs to be used to compress and return it

    char* b = (char*) buffer;
    char * ch = new char[strlen(b)];
    ch = b;

    // this is going to be the return statement
    pthread_exit ((void *) (ch));
}

int main() {
    
    // declare variables
    int num_threads;
    num_threads = 16;
    num_threads = num_threads-1;

    //string input_file = "ACS_P1_source_file";
    //string output_file = "ACS_P1_compressed_file";

    pthread_t thread_array[num_threads];

    int counter;

    // open the source file, get the length
    ifstream file ("example.txt");
    char * buffer;
    file.seekg(0, ios::end);
    int file_size = file.tellg();
    file.seekg (0, ios::beg);

    // some other initializations that are used in the loops below
    int ret;
    int length;

    // go through and assign threads for compression
    for (counter = 0; counter < num_threads; counter++){

        // determine if we should continue, and what the length of the next block is (16384 or some smaller clip)
        if (counter * 16000 > file_size){
            break;
        } else if ((counter+1)*16000 > file_size){
            length = file_size - (counter*16000);
        } else{
            length = 16384;
        }

        // need to get the next 16KB of the input file and assign it to buffer
        buffer = new char [length];
        file.read(buffer, length);

        // make the thread and add it to the correct spot of the array
        pthread_t temp_thread;
        ret = pthread_create(&temp_thread, NULL, worker_thread, (void*) buffer);
        thread_array[counter] = temp_thread;
    }

    // we need to test if the entire source file has been compressed or not
    bool while_cond = true;
    // determine if we should continue
        if (counter * 16000 > file_size){
            while_cond = false;
        } else if ((counter+1)*16000 > file_size){
            length = file_size - (counter*16000);
        } else{
            length = 16000;
        }

    // open up the output file
    ofstream outfile ("output.txt");

    // while there is still some file for us to read
    while (while_cond){

        // determine the oldest thread in the array
        int index = counter % num_threads;

        // read the compressed contents from the thread in that index
        void *ret_join = NULL;
        ret = pthread_join(thread_array[index], &ret_join);
        char* ret_join2 = (char*) ret_join;

        // write out these compressed contents to the output file
        outfile.write(ret_join2, strlen(ret_join2));

        // read in the next part of the input file
        buffer = new char [length];
        file.read(buffer, length);

        // make the thread and add it to the correct spot of the array
        pthread_t temp_thread;
        ret = pthread_create(&temp_thread, NULL, worker_thread, buffer);
        thread_array[index] = temp_thread;

        // incremement and re-evaluate while_cond
        counter++;
        if (counter * 16000 > file_size){
            while_cond = false;
        } else if ((counter+1)*16000 > file_size){
            length = file_size - (counter*16000);
        } else{
            length = 16000;
        }
    }

    // the while loop will stop when writing should stop, but at that point, there are num_thread unwritten compressed files stored in the array
    // this for loop will go through, read them, and write them out to the outfile
    for (int i = 0; i< num_threads; i++){

        // just continuously get contents from oldest thread in array, and write the corresponding compressed data to the output file
        int index = counter % num_threads;
        void *ret_join;
        ret = pthread_join(thread_array[index], &ret_join);
        char* ret_join2 = (char*) ret_join;

        outfile.write(ret_join2, strlen(ret_join2));
        counter++;
    }

    file.close();
    outfile.close();

    return 0;
}