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
    //set dimensions
    int len = 10000;
    int h_a = len;
    int w_a = len;
    int h_b = len;
    int w_b = len;
    // these are simulating the command line arguements - 100x100 matrices, with shorts as the data type (2-byte fixed)
    
    int data_len = 8;

    // the number of values we can fit into a matrix for smart cache multiplication
    int n_values = 8;

    // make matrices: a and b are random, c is initialized to be zeros, program runs a*b=c
    //float test_mass[len2][len2];
    float** mat_a = (float**) malloc(h_a*sizeof(float*));
    float** mat_b = (float**) malloc(h_b*sizeof(float*));
    float** mat_o = (float**) malloc(h_a*sizeof(float*)); //height from a

    for(int m_row = 0; m_row<len; m_row++){
        mat_a[m_row] = (float*) aligned_alloc(32,w_a*sizeof(float));
        mat_o[m_row] = (float*) aligned_alloc(32,w_b*sizeof(float)); //width from b
    }

    for(int m_row = 0; m_row<len; m_row++){
        mat_b[m_row] = (float*) aligned_alloc(32,w_b*sizeof(float));
    }

    //put random values into a and b
    for (int row = 0; row < len; row++){
        for (int col = 0; col < len; col++){
            mat_a[row][col] = my_RandomFloat();//row*16 + col;//((float) (row*len2 + col)) / ((float) len2*len2);//my_RandomFloat();
            mat_b[row][col] = my_RandomFloat();//row*16 + col;//(float) (row*len2 + col) / (float) len2*len2;//my_RandomFloat();
            mat_o[row][col] = 0;
        }
    }
   
    // now we need to iterate through a (top to bottom) and b (left to right) to do the following:
    // c[0:8, 0:8] += a[0:8, i:i+16] * b[i:i+16, 0:8] for (i = 0; i < len, i += 8)
    // each 8x8 block for c will be computed (each value of i gives a new block) using SIMD instructions, then will be added to what exists in memory

    alignas(32) __m256 b[8];
    alignas(32) __m256 rowOut;
    alignas(32) __m256 mulVec;
    // the index of a's col and b's row should always be the same for squares to be multiplied
    //calculate endpoints
    int n_tile_row = h_a/8; //Height of A
    int n_tile_col = w_b/8; //Width of B
    int n_tile_stack = h_b/8; //Width A, Height B

    //allocate tracers
    int row_addr;
    int col_addr;
    int stack_offset;

    //begin calculation. Do row >> col >> stack
    for(int t_row = 0; t_row < n_tile_row; t_row++){
        row_addr = t_row*8;
        cout << t_row << endl;

        for(int t_col = 0; t_col <n_tile_col; t_col++){
            col_addr = t_col*8;

            for(int t_stack = 0; t_stack < n_tile_stack; t_stack++){
                stack_offset = t_stack*8;

                for(int row = 0; row < 8; row++){
                    //load tile values of B into registers
                    //stacks travel vertically in b
                    b[row] = _mm256_load_ps(&mat_b[stack_offset + row][col_addr]);
                }
                for(int out_row = 0; out_row < 8; out_row++){
                    //compute output 1 row at a time
                    //initialize output register as values in memory
                    rowOut = _mm256_load_ps(&mat_o[row_addr + out_row][col_addr]);
                    //Perform multiplication
                    for(int b_row = 0; b_row < 8; b_row++){
                        //load current value
                        mulVec = _mm256_set1_ps(mat_a[row_addr+out_row][stack_offset + b_row]);
                        //multiply and accumulate
                        rowOut = _mm256_fmadd_ps (mulVec, b[b_row], rowOut);
                    }
                //store values to out to memory
                 _mm256_store_ps(&mat_o[row_addr + out_row][col_addr], rowOut);
                }
            }
        }
    }

//CHECKING//
////////////////////////////////////////////////////////////////////
    float** mul = (float**) malloc(len*sizeof(float*));
    for(int m_row = 0; m_row<len; m_row++){
        mul[m_row] = (float*) aligned_alloc(32,len*sizeof(float));
        for(int i = 0; i<len;i++){
            mul[m_row][i]=0;
        }
    }
    
    cout<<"multiply of the matrix=\n";    
    for(int i=0;i<len;i++){    
        for(int k=0;k<len;k++){ 
            for(int j=0;j<len;j++){
                mul[i][k]+=mat_a[i][j]*mat_b[j][k];
                
            }    
        }    
    }
    
    for(int i=0;i<len;i++){
        for(int j=0;j<len;j++){
            if(((mul[i][j] - mat_o[i][j]) > 0.001) | ((mul[i][j] - mat_o[i][j]) < -0.001)){

                cout << mul[i][j] << " " << mat_o[i][j] << endl;
                //cout << mat_o[i][j] << endl;  
            }
        }    
    } 
    
            
        
    
    cout << "Program finished" << endl;

    return 0;
}