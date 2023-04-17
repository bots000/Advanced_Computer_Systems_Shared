#include <immintrin.h>
#include <iostream>
#include <pthread.h>


struct datPack{    
    char* d_in;
    char* wb;
};

void* wb_thread(void* dpack){
    datPack* d_loc = (datPack*) dpack;
    (*d_loc).wb[0] = (*d_loc).d_in[0];
    return nullptr;
    //pthread_exit();
}

__m256i loadStrided(char* Dstart, int stride, int pixels){
    //char items[32];
    char* items = (char*) aligned_alloc(32,32*sizeof(char));
    for(int i =0; i<32; i++){
        if(i<pixels){
            items[i] = Dstart[stride*i];
        }else{
            items[i] = 0;
        }
    }
    return _mm256_load_si256((__m256i*) items);
}

void storeStridedST(char* Dstart, __m256i vec, int stride, int pixels){
    char* tdataIn = (char*) aligned_alloc(32,32*sizeof(char));
    _mm256_store_si256((__m256i*) tdataIn, vec);
    for(int i = 0; i < pixels; i++){
        Dstart[i*stride] = tdataIn[i];
    }
    return;

}
void storeStridedMT(char* Dstart, __m256i vec, int stride, int pixels){
    char* tdataIn = (char*) aligned_alloc(32,32*sizeof(char));
    _mm256_store_si256((__m256i*) tdataIn, vec);
    pthread_t thread_array[16];
    datPack datArr[16];
    for(int i = 0; i < pixels*2; i++){
        if(i<pixels){
            datPack tempPack;
            tempPack.d_in = tdataIn+i;
            tempPack.wb = Dstart+i*stride;

            pthread_t temp_thread;
            pthread_create(&temp_thread, NULL, wb_thread, (void*) &tempPack);
            thread_array[i%pixels] = temp_thread;
        }else{
            pthread_join(thread_array[i%pixels], NULL);
        }
    }
    return;
}


void maskHiLo(char* dataIn, char* dataOut, char t_lo, char t_hi, int stride, int pixels){
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

    for(int i = 0; i*32 < pixels; i++){
        //Load [based on stride] several vectors
        memOffset = i*32*stride;
        cur_vec = loadStrided(dataIn+memOffset,stride,pixels);
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
        //_mm256_store_si256((__m256i*) dataOut + memOffset, cur_vec);
        storeStridedST(dataOut+memOffset, cur_vec, stride, pixels);

    }
    
    
    
    
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
        memOffset = i*32;
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
    maskHiLo(tdataIn,tdataOut,4,10,1,32);
    std::cout << tdataOut[6] << std::endl;
    return 0;
}