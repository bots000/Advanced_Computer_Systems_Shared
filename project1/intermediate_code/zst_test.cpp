#include <iostream>
#include <zstd.h>
#include <pthread.h>
#include <string>
#include <fstream>
using namespace std;

// There needs to be some function called by the thread creation that will run some compression

void *worker_thread(void *data){
    // provided that data is a 16 KB block, the zstd library needs to be used to compress and return it

    // need to type cast data to be in some data format -- whatever that should be !!!!!!!!!!????????????

    // the actual compression itself

    // the destination is compressed_data, compression is size of data or error code
    size_t const compression = ZSTD_compress(compressed_data, compressed_size, og_data, og_size, 1);

    if (ZSTD_isError(compression)){
        cout << "Error encountered, error code " << compression << endl;
    }


    // this is going to be the return statement
    pthread_exit ((void *) (compressed_data));
}

int main() {
    
    // declare variables
    int num_threads;
    num_threads = 5;
    num_threads = num_threads-1;

    string input_file = "ACS_P1_source_file";
    string output_file = "ACS_P1_compressed_file";

    pthread_t thread_array[num_threads];

    int counter;

    // open the source file !!!!!!!!!???????????
    fstream source (input_file, ios::binary);
    source.open(input_file);

    // go through and assign threads for compression
    for (counter = 0; counter < num_threads; count++){

        // need to get the next 16KB of the input file and assign it to data ??????????!!!!!!!!!!!
        pthread_t temp_thread;
        ret = pthread_create(&temp_thread, NULL, worker_thread, void *data);
        thread_array[counter] = &temp_thread;
    }

    // we need to test if the entire source file has been compressed or not
    bool while_cond;
    while (while_cond){

        int index = counter % num_threads;
        void *ret_join;
        ret = pthread_join(thread_array[index], &ret_join);

        // put the buffer into output file

        pthread_t temp_thread;
        ret = pthread_create(&temp_thread, NULL, worker_thread, void *data);
        thread_array[index] = &temp_thread;

        counter++;
        // re-evaluate while_cond
    }

    return 0;
}
