#include <immintrin.h>
#include <iostream>

void maskHiLo(float* dataIn, float* dataOut, float t_lo, float t_hi, int stride, int length){
    //Process:
    //Create hi and lo comparison vectors
    //BEGIN LOOP
    //Load [based on stride] several vectors
    //Compare and make mask for each loaded
    //MaskStore to the temp specific memory location
    //Launch worker thread to handle the memory unpacking while next operations happen
    //REPEAT LOOP
}

void maskHiLoGscale(char* dataIn, char* dataOut, char t_lo, char t_hi, int length){
    //VecReg Usage:
    //16 available
    //0: lo | 1: hi| 2: cur_vec | 3: isUnderHi
    //4: isAboveLo | 5: inRange
    //Process:
    //Create hi and lo comparison vectors
    __m256i c_lo;
    __m256i c_hi;
    c_lo = _mm256_set1_epi8(t_lo);
    c_hi = _mm256_set1_epi8(t_hi);

    //BEGIN LOOP
    //int vecPerLoop = 4;
    __m256i cur_vec;
    __m256i isUnderHi;
    __m256i isAboveLo;
    __m256i inRange;

    long memOffset;

    for(int i = 0; i*32 < length; i++){
        //Load [based on stride] several vectors
        memOffset = i*8;
        cur_vec = _mm256_load_si256((__m256i*) dataIn+memOffset);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpgt_epi8(c_hi,cur_vec);
        printf("%X\n",_mm256_movemask_epi8(isUnderHi));
        isAboveLo = _mm256_cmpgt_epi8(cur_vec,c_lo);
        printf("%X\n",_mm256_movemask_epi8(isAboveLo));
        inRange = _mm256_and_si256(isUnderHi, isAboveLo);
        printf("%X\n",_mm256_movemask_epi8(inRange));
        //Mask the vector so everything in range remains
        cur_vec = _mm256_and_si256(cur_vec, inRange);
        //Store entire vector to the specific memory location
        _mm256_store_si256((__m256i*) dataOut + memOffset, cur_vec);

    }
    
    
    
    
}

int main(){
    char testvector[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    char* tdataIn = (char*) aligned_alloc(32,32*sizeof(char));
    char* tdataOut = (char*) aligned_alloc(32,32*sizeof(char));
    for(int i = 0; i< 32; i++){
        tdataIn[i] = i;
    }
    maskHiLoGscale(tdataIn,tdataOut,4,10,32);
    std::cout << tdataOut[6] << std::endl;
    return 0;
}