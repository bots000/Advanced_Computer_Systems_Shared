#include <iostream>
#include <zstd.h>
#include <pthread.h>
#include <cstring>
#include <fstream>
#include "common.h" 
using namespace std;
/*
static void compress_orDie(const char* fname, const char* oname)
{
    size_t fSize;
    void* const fBuff = mallocAndLoadFile_orDie(fname, &fSize);
    size_t const cBuffSize = ZSTD_compressBound(fSize);
    void* const cBuff = malloc_orDie(cBuffSize);

    /* Compress.
     * If you are doing many compressions, you may want to reuse the context.
     * See the multiple_simple_compression.c example.
     *//*
    size_t const cSize = ZSTD_compress(cBuff, cBuffSize, fBuff, fSize, 1);
    CHECK_ZSTD(cSize);

    saveFile_orDie(oname, cBuff, cSize);

    /* success 
    printf("%25s : %6u -> %7u - %s \n", fname, (unsigned)fSize, (unsigned)cSize, oname);

    free(fBuff);
    free(cBuff);
}

void compress_block(void* cBuff, void* bBuff, size_t bSize){
    size_t const cBuffSize = ZSTD_compressBound(bSize);
    void* const cBuff = malloc_orDie(cBuffSize);
    size_t const cSize = ZSTD_compress(cBuff, cBuffSize, bBuff, bSize, 1);
    CHECK_ZSTD(cSize);
}
*/
int main() {
    
    // declare variables
    int num_threads;
    num_threads = 16;
    num_threads = num_threads-1;

    //string input_file = "ACS_P1_source_file";
    //string output_file = "ACS_P1_compressed_file";

    pthread_t thread_array[num_threads];

    // open the source file, get the length
    ifstream file ("words.txt", ios::binary);
    char * buffer;
    file.seekg(0, ios::end);
    int file_size = file.tellg();
    file.seekg (0, ios::beg);

    int ret;
    int length;

    bool while_cond = true;
    int counter = 0;

    ofstream outfile ("output.txt.zst",ios::binary);

    while (while_cond){
        if (counter * 16384 > file_size){
            while_cond = false;
            break;
        } else if ((counter+1)*16384 > file_size){
            length = file_size - (counter*16384);
        } else{
            length = 16384;}
        //printf("example\n");
        cout << counter << endl;
        buffer = new char [length];
        file.read(buffer, length);

        
        size_t cBuffSize = ZSTD_compressBound(length);
        void* cBuff = malloc_orDie(cBuffSize);
        cout << length << endl;
        cout << cBuffSize << endl;
        size_t cSize = ZSTD_compress(cBuff, cBuffSize, buffer, length, 1);
        CHECK_ZSTD(cSize);

        outfile.write((char*)cBuff, cSize);
        //cout << strlen((char*) buffer) << endl;
        cout << cSize << endl;
        counter++;
    }

    // some other initializations that are used in the loops below
    return 0;
}