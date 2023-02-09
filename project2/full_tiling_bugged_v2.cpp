#include <immintrin.h>
#include <iostream>
#include <random>
using namespace std;

// generates a random float
float my_RandomFloat() {
    float a = 0.0;
    float b = 10.0;
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

int main(){

    /*// set up random number generator
    random_device rd;     // Only used once to initialise (seed) engine
    mt19937 rng(rd());    // Random-number engine used (Mersenne-Twister in this case)
    uniform_int_distribution<int> uni(-10,10); // Guaranteed unbiased*/
    cout << "very first" << endl;

    // these are simulating the command line arguements - 100x100 matrices, with shorts as the data type (2-byte fixed)
    int len = 880;
    int data_len = 8;

    // the number of values we can fit into a matrix for smart cache multiplication
    int n_values = 8;

    // make matrices: a and b are random, c is initialized to be zeros, program runs a*b=c
    float mat_a[880][880];
    float mat_b[880][880];
    float mat_o[880][880];


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

    alignas(32) float mini_a[8][8];
    alignas(32) float mini_b[8][8];
    alignas(32) float tmp[8][8];

    for (int a_col_b_row = 0; a_col_b_row < len; a_col_b_row += n_values){

        for (int a_row = 0; a_row < len; a_row += n_values){
                
            // need to go through and assign mini_a[a_row:a_row+n_values][a_col:a_col+n_values]
            
            for (int r = a_row, i = 0; r < a_row+8; r++, i++){
                for (int c = a_col_b_row, j=0; c < a_col_b_row+8; c++, j++){
                    mini_a[i][j] = mat_a[r][c];
                }
            }

            for (int b_col = 0; b_col < len; b_col += n_values){

                // need to go through and assign mini_b[b_row:b_row+n_values][b_col:b_col+n_values]

                for (int r = a_col_b_row, i=0; r < a_col_b_row+8; r++, i++){
                    for (int c = b_col, j=0; c < b_col+8; c++, j++){
                        mini_b[i][j] = mat_b[r][c];
                    }
                }

                // now that we have mini_a and mini_b, we need to perform the multiplication
                
                
                //initialize matrix b into vector format (occupies 8 registers)
                alignas(32) __m256 b[8];
                for(int row = 0; row < 8; row++){
                    b[row] = _mm256_load_ps(&mini_b[row][0]);
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
                        mulVec = _mm256_set1_ps(mini_a[out_row][b_row]);
                        //multiply and accumulate
                        rowOut = _mm256_fmadd_ps (mulVec, b[b_row], rowOut);
                    }
                //store values to out to memory
                 _mm256_store_ps(tmp[out_row], rowOut);
                }

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
    cout << "Program finished" << endl;

    return 0;
}