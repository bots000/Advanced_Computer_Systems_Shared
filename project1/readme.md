Introduction

The goal for this project was to create an applicable c++ program that demonstrated the understanding of multi-thread programming and the use of the z-standard compression library.  To do this, students were to use one thread to read in a data file, dispatch 16KB blocks of data to worker threads to be compressed using the z-standard library, and have the compressed blocks returned to the original worker thread, where they are streamed to an output file.  The program should work with a configurable number of worker threads.

How To Run

This project assumes that the z-standard compression library is properly installed (see https://github.com/facebook/zstd for instructions).  Compile mt_n_n.cpp using the -lzstd flag.  An example compiling command is g++ mt_n_n.cpp -o outfile -lzstd.  

When running the code, three command line arguments should be passed in.  The first is the number of worker threads to be used at a time during runtime.  The second is the name of the file (including file location) of the file to be compressed.  The third is the filename (including file location) that should be given to the newly compressed version of the input file.  It is advisable not to give it the same name in the same location as the input file.


