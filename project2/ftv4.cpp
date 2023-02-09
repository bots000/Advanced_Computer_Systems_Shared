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
    int len = 10000;
    int data_len = 8;

    // the number of values we can fit into a matrix for smart cache multiplication
    int n_values = 8;

    // make matrices: a and b are random, c is initialized to be zeros, program runs a*b=c
    int len2 = 10000;
    //float test_mass[len2][len2];
    float** mat_a = (float**) malloc(len2*sizeof(float*));
    float** mat_b = (float**) malloc(len2*sizeof(float*));
    float** mat_o = (float**) malloc(len2*sizeof(float*));
    for(int m_row = 0; m_row<len2; m_row++){
        mat_a[m_row] = (float*) aligned_alloc(32,len2*sizeof(float));
        mat_b[m_row] = (float*) aligned_alloc(32,len2*sizeof(float));
        mat_o[m_row] = (float*) aligned_alloc(32,len2*sizeof(float));
    }
    //cout << "al" << endl;
    //static float mat_a[80][80];
    //static float mat_b[80][80];
    //static float mat_o[80][80];
    //cout << "inits" << endl;

    //put random values into a and b
    for (int row = 0; row < len; row++){
        for (int col = 0; col < len; col++){
            //cout << col << endl;
            //cout << row << endl;
            mat_a[row][col] = my_RandomFloat();
            mat_b[row][col] = my_RandomFloat();
            mat_o[row][col] = 0;
            
        }
    }
   
    //cout << "val" << endl;
    // now we need to iterate through a (top to bottom) and b (left to right) to do the following:
    // c[0:8, 0:8] += a[0:8, i:i+16] * b[i:i+16, 0:8] for (i = 0; i < len, i += 8)
    // each 8x8 block for c will be computed (each value of i gives a new block) using SIMD instructions, then will be added to what exists in memory

    alignas(32) float mini_a[8][8];
    alignas(32) float mini_b[8][8];
    alignas(32) float tmp[8][8];
    //cout << "1" << endl;

    alignas(32) __m256 b[8];
    alignas(32) __m256 rowOut;
    alignas(32) __m256 mulVec;
    // the index of a's col and b's row should always be the same for squares to be multiplied
    //calculate endpoints
    int n_tile_row = len2/8; //Height of A
    int n_tile_col = len2/8; //Width of B
    int n_tile_stack = len2/8; //Width A, Height B
    //allocate tracers
    int row_addr;
    int col_addr;
    int stack_offset;
    
    //begin calculation. Do row >> col >> stack
    for(int t_row = 0; t_row < n_tile_row; t_row++){
        row_addr = t_row*8;
        for(int t_col = 0; t_col <n_tile_col; t_col++){
            col_addr = t_col*8;
            for(int t_stack = 0; t_stack < n_tile_stack; t_stack++){

                for(int row = 0; row < 8; row++){
                    //load tile values of B into registers
                    //stacks travel vertically in b
                    b[row] = _mm256_load_ps(&mat_b[row_addr + row][col_addr]);
                }
                for(int out_row = 0; out_row < 8; out_row++){
                    //compute output 1 row at a time
                    //initialize output register as values in memory
                    rowOut = _mm256_load_ps(&mat_o[row_addr + out_row][col_addr]);
                    //Perform multiplication
                    for(int b_row = 0; b_row < 8; b_row++){
                        //load current value
                        mulVec = _mm256_set1_ps(mat_a[row_addr+out_row][b_row]);
                        //multiply and accumulate
                        rowOut = _mm256_fmadd_ps (mulVec, b[b_row], rowOut);
                    }
                //store values to out to memory
                 _mm256_store_ps(&mat_o[row_addr + out_row][col_addr], rowOut);
                }
            }
        }
    }
    
            
        
    
    cout << "Program finished" << endl;

    return 0;
}