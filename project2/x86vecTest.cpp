#include <immintrin.h>
#include <iostream>

int main(void){
    alignas(32) float mat_a[8][8];
    alignas(32) float mat_b[8][8];
    alignas(32) float mat_o[8][8];
    //put in dummy values in a and b
    for (int row = 0; row < 8; row++){
        for (int col = 0; col < 8; col++){
            mat_a[row][col] = (float) (row*8 + col);
            mat_b[row][col] = (float) (row*8 + col);
        }
    }

    alignas(32) __m256 b[8];
    for(int row = 0; row < 8; row++){
        //initialize matrix b into vector format (occupies 8 registers)
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
     _mm256_store_ps(mat_o[out_row], rowOut);
    }
    std::cout << mat_o[0][0] << std::endl;


    return 0;
}
