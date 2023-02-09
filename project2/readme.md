# Introduction



# How To Run



# Experimental Results

The program was run on the same two input files using a variable number of worker threads.  The first input file is a 3.5GB text file.  The second input is a 9GB .mp4 file.  The amount of time it took each file/worker thread combination to run was recorded, as well as if the compressed file could be successfully recovered via decompression (without distorting, losing, or corrupting data).

It is worth noting that this data was obtained on a machine with 32 GB RAM and an 11th generation Intel core processor (operating at 2.9 GHz).

**16 KB Block Data**
| Data Type | Size of Matrices | Student-Implemented Multiplication Runtime | Naive-Implemented Multiplication Runtime | Successful Compression (yes/no) |
| --- | ----- | ----- | ----- | --- | 
| 2 Byte Fixed Point | 1000x1000 | : | : | Yes |
| 4 Byte Floating Point | 1000x1000 | : | : | Yes |
| .txt file (3.5 GB) | 3000x3000 | : | : | Yes |
| .mp4 file (9 GB) | 3000x3000 | : | : | Yes |
| .txt file (3.5 GB) | 5000x5000 | : | : | Yes |
| .mp4 file (9 GB) | 5000x5000 | : | : | Yes |
| .txt file (3.5 GB) | 10000x10000 | : | : | Yes |
| .mp4 file (9 GB) | 10000x10000 | : | : | Yes |

# Analysis



# Conclusion


