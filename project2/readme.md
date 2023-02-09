# Introduction



# How To Run



# Experimental Results

The program was run on the same two input files using a variable number of worker threads.  The first input file is a 3.5GB text file.  The second input is a 9GB .mp4 file.  The amount of time it took each file/worker thread combination to run was recorded, as well as if the compressed file could be successfully recovered via decompression (without distorting, losing, or corrupting data).

It is worth noting that this data was obtained on a machine with 32 GB RAM and an 11th generation Intel core processor (operating at 2.9 GHz).

**16 KB Block Data**
| Data Type | Size of Matrices | Student-Implemented Multiplication Runtime | Naive-Implemented Multiplication Runtime | Successful Compression (yes/no) |
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

# Analysis



# Conclusion


