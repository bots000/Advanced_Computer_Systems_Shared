#include <immintrin.h>
#include <iostream>
#include <random>
using namespace std;

struct ArrStruct {
    alignas(32) float arr[8][8];
};


// generates a random float
float my_RandomFloat() {
    float a = 0.0;
    float b = 10.0;
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

// this is going to temporarily going to be a function to get the substrings that I want from a matrix
ArrStruct my_submatrix(float matrix[768][768], int row_begin, int row_end, int col_begin, int col_end) {
    ArrStruct subM;
    for (int r = row_begin, i=0; r < row_end; r++, i++){
        for (int c = col_begin, j= 0; c < col_end; c++, j++){
            subM.arr[i][j] = matrix[r][c];
        }
    }
    return subM;
}

// the function that takes in a and b as 8x8 and returns the product of the two using smart cache and SIMD instructions
ArrStruct c_block (float mat_a[8][8], float mat_b[8][8]){

    ArrStruct mat_o;

    //initialize matrix b into vector format (occupies 8 registers)
    alignas(32) __m256 b[8];
    for(int row = 0; row < 8; row++){
        b[row] = _mm256_load_ps(&mat_b[row][0]);
    }

    alignas(32) __m256 rowOut;
    alignas(32) __m256 mulVec;
    for(int out_row = 0; out_row < 8; out_row++){
        //compute output 1 row at a time
        //initialize output register as 0
        rowOut = _mm256_setzero_ps();
        //Perform multiplication
        for(int b_row = 0; b_row < 8; b_row++){
            //load current value
            mulVec = _mm256_set1_ps(mat_a[out_row][b_row]);
            //multiply and accumulate
            rowOut = _mm256_fmadd_ps (mulVec, b[b_row], rowOut);
        }
    //store values to out to memory
     _mm256_store_ps(mat_o.arr[out_row], rowOut);
    }
    return mat_o;
}

int main(){

    /*// set up random number generator
    random_device rd;     // Only used once to initialise (seed) engine
    mt19937 rng(rd());    // Random-number engine used (Mersenne-Twister in this case)
    uniform_int_distribution<int> uni(-10,10); // Guaranteed unbiased*/
    cout << "very first" << endl;

    // these are simulating the command line arguements - 100x100 matrices, with shorts as the data type (2-byte fixed)
    int len = 768;
    int data_len = 8;

    // the number of values we can fit into a matrix for smart cache multiplication
    int n_values = 8;

    // make matrices: a and b are random, c is initialized to be zeros, program runs a*b=c
    float mat_a[768][768];
    float mat_b[768][768];
    float mat_o[768][768];


    //put random values into a and b
    for (int row = 0; row < len; row++){
        for (int col = 0; col < len; col++){
            mat_a[row][col] = my_RandomFloat();
            mat_b[row][col] = my_RandomFloat();
            mat_o[row][col] = 0;
        }
    }
    // now we need to iterate through a (top to bottom) and b (left to right) to do the following:
    // c[0:8, 0:8] += a[0:8, i:i+16] * b[i:i+16, 0:8] for (i = 0; i < len, i += 8)
    // each 8x8 block for c will be computed (each value of i gives a new block) using SIMD instructions, then will be added to what exists in memory

    for (int a_col_b_row = 0; a_col_b_row < len; a_col_b_row += n_values){

        for (int a_row = 0; a_row < len; a_row += n_values){
                
            // need to go through and assign mini_a[a_row:a_row+n_values][a_col:a_col+n_values]
            alignas(32) float mini_a[8][8];
            ArrStruct mini_a_struct = my_submatrix(mat_a, a_row, a_row+n_values, a_col_b_row, a_col_b_row+n_values);
            copy(&mini_a_struct.arr[0][0], &mini_a_struct.arr[0][0] + 8 * 8, &mini_a[0][0]);

            for (int b_col = 0; b_col < len; b_col += n_values){

                // need to go through and assign mini_b[b_row:b_row+n_values][b_col:b_col+n_values]
                alignas(32) float mini_b[8][8];
                ArrStruct mini_b_struct = my_submatrix(mat_b, a_col_b_row, a_col_b_row+n_values, b_col, b_col+n_values);
                copy(&mini_b_struct.arr[0][0], &mini_b_struct.arr[0][0] + 8 * 8, &mini_b[0][0]);

                // now that we have mini_a and mini_b, we need to perform the multiplication
                alignas(32) float tmp[8][8];
                ArrStruct tmp_struct = c_block(mini_a, mini_b);
                copy(&tmp_struct.arr[0][0], &tmp_struct.arr[0][0] + 8 * 8, &tmp[0][0]);

                //cout << t << endl;*/
                // now we need to add this result to the needed spot in mat_o
                for (int i = 0; i < 8; i++){
                    for (int j=0; j < 8; j++){
                        mat_o[i+a_row][j+b_col] += tmp[i][j];
                    }
                }
            }
        }
    }

    // going to employ the native solution for comparison now

    float mul[768][768];    
    for(int i=0;i<len;i++){    
        for(int j=0;j<len;j++){    
            mul[i][j]=0;    
            for(int k=0;k<len;k++){    
                mul[i][j]+=mat_a[i][k]*mat_b[k][j];    
            }    
        }    
    }

    // since I sometimes have errors of e-5 or e-7 for each number, == cannot be used for comparison
    // instead, I found the total error on all numbers, and divide by the number of numbers to get the average error
    // I also record the maximum error found

    float diff_sum = 0;
    float max_error = 0;
    for (int row = 0; row < 8; row++){
        for (int col = 0; col < 8; col++){
            float error = mat_o[row][col] - mul[row][col];
            diff_sum += error;
            if (error > max_error){
                max_error = error;
            }
        }
    }
    float t = diff_sum / (float) (768*768);

    cout << t << endl;
    cout << max_error << endl;
    cout << "Program finished" << endl;

    return 0;
}