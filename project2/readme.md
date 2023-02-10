# Introduction

The task presented to the students was to, in c/c++, create a program to perform square-multiplication with increased efficiency.  In order to make it more efficient, the students were specifically tasked with using SIMD (single instruction multiple data) commands and utilizing smart-cache access techniques.  The program is to support a configurable matrix size (square matrices of the same size are being multiplied) and both 4-byte floating point numbers as well as 2-byte fixed point numbers.  Matrix multiplication is used for countless applications in the realm of computer science, engineering, and mathematics, making this project qualified for real-world applications.  Additionally, smart-cache accessing is important for reduced latency during runtime and demonstrates knowledge of data accessing.

# Code Structure

The final implementation is split between three c++ files: mat_mult_main.cpp, mat_mult_float.cpp, and mat_mult_uint16.cpp.

mat_mult_main.cpp simply takes in the command line arguments, the size n of the matrices to be multiplied and the data type (either 4-byte floating point and 2-byte fixed point) to multiply, then calls one of the other two files.  mat_mult_float.cpp is called for when 4-byte floating point numbers are specified, and mat_mult_uint16 is call for when 2-byte fixed point numbers are specified.  It is import to note that "2-byte fixed point" data was interpretted to mean the uint16 data type.  These two files have different SIMD instructions specific to either 32-bit floats or 16-bit integers.  Other than these commands, which will be listed, the code structure of these two files are the same.  The code structure is as follows:

- Allocate space for matrix A, matrix B, and matrix C
- Fill matrices A and B with random data of the correct data type
- Break matrices A and B into 8x8 blocks
- Correctly "tile" the matrix multiplication (choose which blocks of A and B need to be multiplied and put into the C)
- Run a 8x8 * 8x8 sub-routine for each blocks of A and B that should be multiplied (determined by the tiling process)
- Add the results of each sub-multiplication to the correct spot in C
- Verify results of this multiplication with a naive implementation

Matrix tiling, or matrix blocking, is a strategy used in matrix multiplication that reduces cache misses, which in turn reduces memory accesses, which in turn reduces runtime lantency.  In this implementation of matrix tiling, A is primarily traversed in rows while B is primarily traversed in columns.  The k'th 8x8 block in the i'th row of matrix A is multiplied by the k'th 8x8 block in the j'th column of matrix B.  This result is then added to the contents of the 8x8 block in the i'th row and j'th column of matrix C.  This is similar to the naive solution, however instead of multiplying one index of A by one index of B and adding it to C, entire blocks are multiplied and added at a time.  The tiling was implemented such that each 8x8 block of A is only called from memory one time, and stored in cache afterwards.  This is done by loading the first block from A, and performing all needed computations from it (traverse through matrix B and get all blocks that should be multiplied with this block from A) before accessing the next block from A.  This ensures that no blocks from A are called from memory, then replaced in cache by a different block from A just to be called from memory again later.  Ensuring that this doesn't happen removes un-needed memory accesses, utilizing smart cache management, and reduces overall runtime.

Entire 8x8 sub-matrices can be multiplied together as opposed to just single elements because of the functionality of SIMD commands. In general, SIMD commands are commands that perform some arithmetic in an element-wise fashion on an entire array (or on two arrays) of data.  For example, through SIMD commands, [1, 2, 3] .* [4, 5, 6] can be performed to obtain a product of [4, 10, 18].  As matrix multiplication is just element-wise multiplication and then summing of rows/columns, SIMD commands allow us to shortcut this task and perform many computations at once.  Using the AVX package, 16 SIMD registers, each containing 256 bits, were available.  This allowed for 8x8 multiplication: one register held the A block being multiplied, 8 registers held the 8 B blocks it needs to be multiplied by, and one register held the C block where the product was to be stored.  Even though this leaves some registers unused, 8x8 blocks were chosen as 4 bytes of data corresponds to 32 bits, and 256/32 = 8.  Therefore, no more than 8 4-byte floating point elements can be included in a register.

It is important to note that if a matrix size that is not a multiple of 8 is passed in, the code will pad the made matrices with 0's to make it sized to be the next largest multiple of 8.  For example, if 999 is inputted, the code will fill A and B with 999x999 random values, and one additional row and column of 0's.  Multipliing with 0's padded in the matrices is equivalent to if they were not there, expect the output will have the same padded 0's on it (which can be ignored or dropped), and it allows the code to not have to adjust the multiplying sub-routine if the size is not a multiple of 8.

The following SIMD commands were used for the 4-byte floating point number multiplication (for details on what they do, see  https://software.intel.com/sites/landingpage/IntrinsicsGuide/):
- _mm256_load_ps
- _mm256_set1_ps
- _mm256_fmadd_ps
- _mm256_store_ps

The following SIMD commands were used for the 2-byte fixed point number multiplication (for details on what they do, see  https://software.intel.com/sites/landingpage/IntrinsicsGuide/):
- _mm_insert_epi16
- _mm_set1_epi16
- _mm_mullo_epi16
- _mm_add_epi16
- _mm_extract_epi16

# How to Run

To compile this code, be sure to have the three main files,  mat_mult_main.cpp, mat_mult_float.cpp, and mat_mult_uint16.cpp, downloaded and saved in the same directory.  mat_mult_float.cpp and mat_mult_uint16.cpp are included in mat_mult_main.cpp, so only one file needs to be compiled.  To compile this project, g++ was used.  Certain flags need to be set during compiling to ensure that the AVX library of SIMD commands can properly run.  An example of these flags, and how to compile, is shown below.

g++ -march=native -mavx mat_mult_main.cpp -o outfile

mat_mult_main.cpp takes two command line arguments.  The first is an integer n, representing the size of the matrices to be multiplied (nxn*nxn multiplication is performed).  The second is an integer to represent what sort of data is being multipled.  Entering 1 will use four-byte floating point data, entering 2 will use two-byte fixed point data, and entering anything else will return a message saying no computations were performed (as an invalid option was selected).  An example of how to run this project is shown below (it will mutliply two 10000x10000 matrices containing floats).

./outfile 10000 1

# Experimental Results

The program was run on several different sized matrices using both data types.  The amount of time taken to run, as well as if the naive solution and implemented solution returned the same result, were noted.  These statistics can be seen in the table below.

It is worth noting that this data was obtained on a machine with the following hardware characteristics: i7 4 core 8 threads, 320 K L1, 5M L2, 12M L3 with 32 GB RAM, 2.92GHz clock speed.

**16 KB Block Data**
| Data Type | Size of Matrices | Student-Implemented Multiplication Runtime (seconds) | Naive-Implemented Multiplication Runtime (seconds) | Successful Multiplication (yes/no) |
| --- | ----- | ----- | ----- | --- | 
| 2 Byte Fixed Point | 1000x1000 | 2.15188 | 4.82357 | Yes |
| 4 Byte Floating Point | 1000x1000 | .823078 | 6.59371 | Yes |
| 2 Byte Fixed Point | 3000x3000 | 62.857 | 218.439 | Yes |
| 4 Byte Floating Point | 3000x3000 | 27.6944 | 247.826 | Yes |
| 2 Byte Fixed Point | 5000x5000 | 287.44 | 1028.41 | Yes |
| 4 Byte Floating Point | 5000x5000 | 145.775 | 1168.04 | Yes |
| 2 Byte Fixed Point | 10000x10000 | 2676.35 | n/a* | Yes |
| 4 Byte Floating Point | 10000x10000 | 1291.98 | n/a* | Yes |

n/a*: these runs took longer than multiple hours, and didn't finish running within a time reasonable enough to record on a personal device.
# Analysis



# Conclusion


