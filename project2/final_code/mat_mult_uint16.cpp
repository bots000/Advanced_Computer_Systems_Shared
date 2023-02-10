#include <immintrin.h>
#include <iostream>
#include <random>
#include <chrono>
using namespace std;

void two_byte(int len){

    random_device rd;     // Only used once to initialise (seed) engine
    mt19937 rng(rd());    // Random-number engine used (Mersenne-Twister in this case)
    uniform_int_distribution<int> uni(0,65535); // Guaranteed unbiased

    int leftover = 8-(len % 8);

    // make matrices: a and b are random, c is initialized to be zeros, program runs a*b=c
    uint16_t** mat_a = (uint16_t**) malloc((len+leftover)*sizeof(uint16_t*));
    uint16_t** mat_b = (uint16_t**) malloc((len+leftover)*sizeof(uint16_t*));
    uint16_t** mat_o = (uint16_t**) malloc((len+leftover)*sizeof(uint16_t*));

    // used for naive checking at the end
    uint16_t** mul = (uint16_t**) malloc((len+leftover)*sizeof(uint16_t*));

    for(int m_row = 0; m_row<(len+leftover); m_row++){
        mat_a[m_row] = (uint16_t*) aligned_alloc(32,(len+leftover)*sizeof(uint16_t));
        mat_o[m_row] = (uint16_t*) aligned_alloc(32,(len+leftover)*sizeof(uint16_t));
        mat_b[m_row] = (uint16_t*) aligned_alloc(32,(len+leftover)*sizeof(uint16_t));
        mul[m_row] = (uint16_t*) aligned_alloc(32,(len+leftover)*sizeof(uint16_t));
    }

    //put random values into a and b
    for (int row = 0; row < len; row++){
        for (int col = 0; col < len; col++){
            mat_a[row][col] = (uint16_t) uni(rng);//row*16 + col;//((float) (row*len2 + col)) / ((float) len2*len2);//my_RandomFloat();
            mat_b[row][col] = (uint16_t) uni(rng);//row*16 + col;//(float) (row*len2 + col) / (float) len2*len2;//my_RandomFloat();
            mat_o[row][col] = 0;
            mul[row][col] = 0;
        }
        for (int col = len; col < len+leftover; col++){
            mat_a[row][col] = 0;//row*16 + col;//((float) (row*len2 + col)) / ((float) len2*len2);//my_RandomFloat();
            mat_b[row][col] = 0;//row*16 + col;//(float) (row*len2 + col) / (float) len2*len2;//my_RandomFloat();
            mat_o[row][col] = 0;
            mul[row][col] = 0;
        }
    }

    for (int row = len; row < len+leftover; row++){
        for (int col = 0; col < len+leftover; col++){
            mat_a[row][col] = 0;//row*16 + col;//((float) (row*len2 + col)) / ((float) len2*len2);//my_RandomFloat();
            mat_b[row][col] = 0;//row*16 + col;//(float) (row*len2 + col) / (float) len2*len2;//my_RandomFloat();
            mat_o[row][col] = 0;
            mul[row][col] = 0;
        }
    }
   
    // now we need to iterate through a (top to bottom) and b (left to right) to do the following:
    // c[0:8, 0:8] += a[0:8, i:i+16] * b[i:i+16, 0:8] for (i = 0; i < len, i += 8)
    // each 8x8 block for c will be computed (each value of i gives a new block) using SIMD instructions, then will be added to what exists in memory

    alignas(32) __m128i b[8];
    alignas(32) __m128i rowOut;
    alignas(32) __m128i mulVec;
    // the index of a's col and b's row should always be the same for squares to be multiplied
    //calculate endpoints
    int n_tile_row = (len+leftover)/8; //Height of A
    int n_tile_col = (len+leftover)/8; //Width of B
    int n_tile_stack = (len+leftover)/8; //Width A, Height B

    //allocate tracers
    int row_addr;
    int col_addr;
    int stack_offset;

    auto t0 = std::chrono::steady_clock::now();

    //begin calculation. Do row >> col >> stack
    for(int t_row = 0; t_row < n_tile_row; t_row++){
        row_addr = t_row*8;

        for(int t_col = 0; t_col <n_tile_col; t_col++){
            col_addr = t_col*8;

            for(int t_stack = 0; t_stack < n_tile_stack; t_stack++){
                stack_offset = t_stack*8;

                for(int row = 0; row < 8; row++){
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr], 0);
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr+1], 1);
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr+2], 2);
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr+3], 3);
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr+4], 4);
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr+5], 5);
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr+6], 6);
                    b[row] = _mm_insert_epi16(b[row], mat_b[stack_offset + row][col_addr+7], 7);
                }

                for(int out_row = 0; out_row < 8; out_row++){
                    //compute output 1 row at a time
                    //initialize output register as values in memory
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr], 0);
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr+1], 1);
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr+2], 2);
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr+3], 3);
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr+4], 4);
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr+5], 5);
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr+6], 6);
                    rowOut = _mm_insert_epi16(rowOut, mat_o[row_addr + out_row][col_addr+7], 7);



                    //Perform multiplication
                    for(int b_row = 0; b_row < 8; b_row++){
                        //load current value
                        mulVec = _mm_set1_epi16(mat_a[row_addr+out_row][stack_offset + b_row]);
                        //multiply and accumulate

                        mulVec = _mm_mullo_epi16 (mulVec, b[b_row]);
                        rowOut = _mm_add_epi16(rowOut, mulVec);
                    }

                    //store values to out to memory
                    mat_o[row_addr + out_row][col_addr + 0] = _mm_extract_epi16 (rowOut, 0);
                    mat_o[row_addr + out_row][col_addr + 1] = _mm_extract_epi16 (rowOut, 1);
                    mat_o[row_addr + out_row][col_addr + 2] = _mm_extract_epi16 (rowOut, 2);
                    mat_o[row_addr + out_row][col_addr + 3] = _mm_extract_epi16 (rowOut, 3);
                    mat_o[row_addr + out_row][col_addr + 4] = _mm_extract_epi16 (rowOut, 4);
                    mat_o[row_addr + out_row][col_addr + 5] = _mm_extract_epi16 (rowOut, 5);
                    mat_o[row_addr + out_row][col_addr + 6] = _mm_extract_epi16 (rowOut, 6);
                    mat_o[row_addr + out_row][col_addr + 7] = _mm_extract_epi16 (rowOut, 7);

                }
            }
        }
    }

    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = t1-t0;

    cout << "Implemented multiplication finished" << endl;
    cout << "Implemented multiplication took " << elapsed_seconds.count() << " seconds" << endl;

    //CHECKING AGAINST THE NAIVE SOLUTION
    ////////////////////////////////////////////////////////////////////

    t0 = std::chrono::steady_clock::now();
    
    // compute the naive solution     
    for(int i=0;i<len+leftover;i++){    
        for(int k=0;k<len+leftover;k++){ 
            for(int j=0;j<len+leftover;j++){
                mul[i][k]+=mat_a[i][j]*mat_b[j][k];
                
            }    
        }    
    }

    t1 = std::chrono::steady_clock::now();
    elapsed_seconds = t1-t0;

    cout << "Naive multiplication finished" << endl;
    cout << "Naive multiplication took " << elapsed_seconds.count() << " seconds" << endl;
    
    // output any irregularities
    for(int i=0;i<len+leftover;i++){
        for(int j=0;j<len+leftover;j++){
            if((abs(mul[i][j] - mat_o[i][j]) > 0.1)){

                cout << mul[i][j] - mat_o[i][j] << endl;
                //cout << mat_o[i][j] << endl;  
            }
        }    
    }
}