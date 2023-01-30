# Introduction

The goal for this project was to create an applicable c++ program that demonstrated the understanding of multi-thread programming and the use of the z-standard compression library.  To do this, students were to use one thread to read in a data file, dispatch 16KB blocks of data to worker threads to be compressed using the z-standard library, and have the compressed blocks returned to the original worker thread, where they are streamed to an output file.  The program should work with a configurable number of worker threads.

# How To Run

This project assumes that the z-standard compression library is properly installed (see https://github.com/facebook/zstd for instructions).  It is also assumed that the *common.h* is downloaded along side *compress_with_threads*, the primary C++ file.  Compile *compress_with_threads.cpp* using the *-lzstd* flag.  An example compiling command is *g++ compress_with_threads.cpp -o outfile -lzstd*.  

When running the code, three command line arguments should be passed in.  The first is the number of worker threads to be used at a time during runtime.  The second is the name of the file (including file location) of the file to be compressed.  The third is the filename (including file location) that should be given to the newly compressed version of the input file.  It is advisable not to give it the same name in the same location as the input file.  An example of how to run the program would be ./outfile 6 inputfile.txt outputfile.txt.zst, which would run using 1 main thread and 5 worker threads.

If automated results want to be generated for variable number of threads, *test_compression.sh* can be used.  The number of threads and name of input files need to be altered as desired.

# Experimental Results

The program was run on the same two input files using a variable number of worker threads.  The first input file is a 3.5GB text file.  The second input is a 9GB .mp4 file.  The amount of time it took each file/worker thread combination to run was recorded, as well as if the compressed file could be successfully recovered via decompression (without distorting, losing, or corrupting data).

It is worth noting that this data was obtained on a machine with 32 GB RAM and an 11th generation Intel core processor (operating at 2.9 GHz).

**16 KB Block Data**
| File type | Number of worker threads | Runtime (seoncds) | Successful Compression (yes/no) |
| --- | ----- | ----- | --- |
| .txt file (3.5 GB) | 25 | 1:03 | Yes |
| .mp4 file (9 GB) | 25 | 4:59 | Yes |
| .txt file (3.5 GB) | 50 | 1:59 | Yes |
| .mp4 file (9 GB) | 50 | 5:32 | Yes |
| .txt file (3.5 GB) | 100 | 2:01 | Yes |
| .mp4 file (9 GB) | 100 | 5:22 | Yes |
| .txt file (3.5 GB) | 200 | 2:06 | Yes |
| .mp4 file (9 GB) | 200 | 5:12 | Yes |
| .txt file (3.5 GB) | 350 | 1:55 | Yes |
| .mp4 file (9 GB) | 350 | 4:58 | Yes |
| .txt file (3.5 GB) | 500 | 1:35 | Yes |
| .mp4 file (9 GB) | 500 | 4:04 | Yes |
| .txt file (3.5 GB) | 750 | 1:03 | Yes |
| .mp4 file (9 GB) | 750 | 4:09 | Yes |
| .txt file (3.5 GB) | 1000 | 1:47 | Yes |
| .mp4 file (9 GB) | 1000 | 4:54 | Yes |


**32 KB Block Data**
| File type | Number of worker threads | Runtime (seoncds) | Successful Compression (yes/no) |
| --- | ----- | ----- | --- |
| .txt file (3.5 GB) | 3 | 0:35 | Yes |
| .mp4 file (9 GB) | 3 | 2:05 | Yes |
| .txt file (3.5 GB) | 10 | 0:37 | Yes |
| .mp4 file (9 GB) | 10 | 2:15 | Yes |
| .txt file (3.5 GB) | 25 | 0:55 | Yes |
| .mp4 file (9 GB) | 25| 2:23 | Yes |
| .txt file (3.5 GB) | 50 | 0:45 | Yes |
| .mp4 file (9 GB) | 50 | 2:18 | Yes |
| .txt file (3.5 GB) | 100 | 0:45 | Yes |
| .mp4 file (9 GB) | 100 | 2:18 | Yes |
| .txt file (3.5 GB) | 200 | 0:48 | Yes |
| .mp4 file (9 GB) | 200 | 2:17 | Yes |
| .txt file (3.5 GB) | 300 | 0:55 | Yes |
| .mp4 file (9 GB) | 300 | 2:19 | Yes |
| .txt file (3.5 GB) | 450 | 0:53 | Yes |
| .mp4 file (9 GB) | 450 | 2:17 | Yes |
| .txt file (3.5 GB) | 600 | 0:48 | Yes |
| .mp4 file (9 GB) | 600 | 2:22 | Yes |
| .txt file (3.5 GB) | 750 | 0:44 | Yes |
| .mp4 file (9 GB) | 750 | 2:20 | Yes |

# Analysis

It can be observed that with minimal worker threads, the .txt compression took approximately 2 minutes, while the .mp4 compression took approximately 5 minutes.  As the number of worker threads increased slightly, the compression time slightly increased.  This is due to 16KB being a block of data so small that compression time is less than the amount of time it took to stream the input data and manage the worker threads.  Simply put, the compression was significantly faster than the work done by the root thread, and a smaller number of worker threads was not efficient.

As the number of worker threads increased to more substantial numbers, minimally 350, the time of creating and managing them was no longer greater than the benefit that the parallel programming provided.  However, if the number of worker threads became too large, the total runtime began to increase again.  This is seen with the 1000 worker threads example.  The peak number of worker threads, given this limited data set, was approximately 750 worker threads.  It is also worth noting that the optimal number differs based on the input file size, as the 9GB .mp4 file required less worker threads than the 3.5GB .txt file.

In an effort to showcase that the optimal number of worker threads was directly affected by the size of the blocks being compressed (16KB), the same program was run using 32KB blocks instead.  It can be seen that using worker threads, this program ran much faster than its 16KB counterpart.  This is because the worker threads individually run longer (as it takes longer to compress more data), meaning that more time is saved when they are processed in parallel.  However, it can be seen that the number of threads did not have a significant effect on the total runtime.  This could be due to the increased time taken to stream the input data in comparison to the work done by the worker threads.  The optimal time for the .txt file dipped from approximately 1 minute to approximately 45 seconds, and the optimal time for the .mp4 file dipped from approximately 4 minutes to approximately 1.3 minutes.

It is important to note that small fluctations in runtime between runs is expected, and explains the small fluctations seen in the 32KB block runs.

# Conclusion

In conclusion, the program showcased the understanding and appliance of multi-thread programming and the z-standard compression library.  Files were successfully compressed, and were not damaged or corrupted upon decompression.  It was showcased that while using multiple threads can speed up a process, too many/few threads can be detrimental depending on how long the multi-threaded process takes to run.  There is also evidence to support that the optimal number of threads can be affected by factors such as input file size and the size of the blocks in which input data is read.




