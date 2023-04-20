#include <immintrin.h>
#include <iostream>
#include <pthread.h>


struct datPack{    
    unsigned char* d_in;
    unsigned char* wb;
};

void* wb_thread(void* dpack){
    datPack* d_loc = (datPack*) dpack;
    (*d_loc).wb[0] = (*d_loc).d_in[0];
    return nullptr;
    //pthread_exit();
}

__m256i loadStrided(unsigned char* Dstart, int stride, int pixels){
    //char items[32];
    char* items = (char*) aligned_alloc(32,32*sizeof(char));
    for(int i =0; i<32; i++){
        if(i<pixels){
            items[i] = Dstart[stride*i];
        }else{
            items[i] = 0;
        }
    }
    //delete items;
    return _mm256_load_si256((__m256i*) items);
}

void storeStridedST(unsigned char* Dstart, __m256i vec, int stride, int pixels){
    //char* tdataIn = (char*) aligned_alloc(32,32*sizeof(char));
    unsigned char tdataIn[32];
    _mm256_storeu_si256((__m256i*) tdataIn, vec);
    for(int i = 0; i < pixels; i++){
    //printf("before\n");
        Dstart[i*stride] = tdataIn[i];
    //printf("after\n");
    }
    //delete tdataIn;
    return;

}
void storeStridedMT(unsigned char* Dstart, __m256i vec, int stride, int pixels){
    unsigned char* tdataIn = (unsigned char*) aligned_alloc(32,32*sizeof(char));
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
    delete tdataIn;
    return;
}


void maskHiLo(unsigned char* dataIn, unsigned char* dataOut, char t_lo, char t_hi, int stride, int pixels){
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

    int cur_pixels = 32;
    printf("Pixels: %d\n",pixels);
    for(int i = 0; i*32 < pixels; i++){
        printf("Cur pixel: %d\r",i);
        if(pixels - i*32 < 32){
            cur_pixels = pixels - i*32 < 32;
        }
        //Load [based on stride] several vectors
        memOffset = i*32*stride;
        cur_vec = loadStrided(dataIn+memOffset,stride,cur_pixels);
        //Compare and make mask for each loaded
        //isUnderHi = _mm256_cmpgt_epi8(c_hi,cur_vec);
        isUnderHi = _mm256_cmpeq_epi8( c_hi, _mm256_max_epu8(c_hi,cur_vec));
        //printf("%X\n",_mm256_movemask_epi8(isUnderHi));
        //isAboveLo = _mm256_cmpgt_epi8(cur_vec,c_lo);
        isAboveLo = _mm256_cmpeq_epi8( cur_vec, _mm256_max_epu8(cur_vec,c_lo));
        //printf("%X\n",_mm256_movemask_epi8(isAboveLo));
        inRange = _mm256_and_si256(isUnderHi, isAboveLo);
        //printf("%X\n",_mm256_movemask_epi8(inRange));
        //Mask the vector so everything in range remains
        cur_vec = _mm256_and_si256(cur_vec, inRange);
        //Store entire vector to the specific memory location
        //_mm256_store_si256((__m256i*) dataOut + memOffset, cur_vec);
        storeStridedST(dataOut+memOffset, cur_vec, stride, cur_pixels);

    }
}

void maskHiLoRGB(unsigned char* dataIn, unsigned char* dataOut, char r_lo, char r_hi, char g_lo, char g_hi, char b_lo, char b_hi, int pixels){
    //Process:
    //Create hi and lo comparison vectors
    int stride = 3;
    __m256i c_lo;
    __m256i c_hi;
    

    //BEGIN LOOP
    //int vecPerLoop = 4;
    __m256i R;
    __m256i G;
    __m256i B;
    __m256i isUnderHi;
    __m256i isAboveLo;
    __m256i inRange;

    long memOffset;

    int cur_pixels = 32;
    printf("Pixels: %d\n",pixels);

    for(int i = 0; i*32 < pixels; i++){
        printf("Cur pixel: %d\r",i);
        if(pixels - i*32 < 32){
            cur_pixels = pixels - i*32 < 32;
        }

        //Load [based on stride] several vectors
        memOffset = i*32*stride;
        R = loadStrided(dataIn+memOffset,stride,cur_pixels);
        G = loadStrided(dataIn+memOffset+1,stride,cur_pixels);
        B = loadStrided(dataIn+memOffset+2,stride,cur_pixels);

        //////////Compare R Threshold//////////////////
        c_lo = _mm256_set1_epi8(r_lo);
        c_hi = _mm256_set1_epi8(r_hi);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpeq_epi8( c_hi, _mm256_max_epu8(c_hi,R));
        isAboveLo = _mm256_cmpeq_epi8(R, _mm256_max_epu8(R,c_lo));
        inRange = _mm256_and_si256(isUnderHi, isAboveLo); //Initial val of inRange, set not modified
        ///////////////////////////////////////////////

        //////////Compare G Threshold//////////////////
        c_lo = _mm256_set1_epi8(g_lo);
        c_hi = _mm256_set1_epi8(g_hi);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpeq_epi8( c_hi, _mm256_max_epu8(c_hi,G));
        isAboveLo = _mm256_cmpeq_epi8(G, _mm256_max_epu8(G,c_lo));
        inRange = _mm256_and_si256(inRange, isAboveLo); //modify running mask
        inRange = _mm256_and_si256(isUnderHi, inRange); //modify running mask
        ///////////////////////////////////////////////

        //////////Compare B Threshold//////////////////
        c_lo = _mm256_set1_epi8(b_lo);
        c_hi = _mm256_set1_epi8(b_hi);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpeq_epi8( c_hi, _mm256_max_epu8(c_hi,B));
        isAboveLo = _mm256_cmpeq_epi8(B, _mm256_max_epu8(B,c_lo));
        inRange = _mm256_and_si256(inRange, isAboveLo); //modify running mask
        inRange = _mm256_and_si256(isUnderHi, inRange); //modify running mask
        ///////////////////////////////////////////////

        //Mask the vectors so everything in range remains
        R = _mm256_and_si256(R, inRange);
        G = _mm256_and_si256(G, inRange);
        B = _mm256_and_si256(B, inRange);
        

        //Store entire vector to the specific memory location
        storeStridedST(dataOut+memOffset, R, stride, cur_pixels);
        storeStridedST(dataOut+memOffset+1, G, stride, cur_pixels);
        storeStridedST(dataOut+memOffset+2, B, stride, cur_pixels);

    }
}


void maskHiLoGscale(unsigned char* dataIn, unsigned char* dataOut, char t_lo, char t_hi, int length){
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
    int cur_pixels = 32;
    for(int i = 0; i*32 < length; i++){
        if(length - i*32 < 32){
            cur_pixels = length - i*32 < 32;
        }
        //Load [based on stride] several vectors
        memOffset = i*32;
        cur_vec = _mm256_load_si256((__m256i*) dataIn+memOffset);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpgt_epi8(c_hi,cur_vec);
        //printf("%X\n",_mm256_movemask_epi8(isUnderHi));
        isAboveLo = _mm256_cmpgt_epi8(cur_vec,c_lo);
        //printf("%X\n",_mm256_movemask_epi8(isAboveLo));
        inRange = _mm256_and_si256(isUnderHi, isAboveLo);
        //printf("%X\n",_mm256_movemask_epi8(inRange));
        //Mask the vector so everything in range remains
        cur_vec = _mm256_and_si256(cur_vec, inRange);
        //Store entire vector to the specific memory location
        _mm256_store_si256((__m256i*) dataOut + memOffset, cur_vec);

    }
    
    
    
    
}

void maskHiLoRGB2(unsigned char* dataIn, unsigned char* dataOut, char r_lo, char r_hi, char g_lo, char g_hi, char b_lo, char b_hi, int pixels){
    //Process:
    //Create hi and lo comparison vectors
    int stride = 3;
    __m256i c_lo; //0
    __m256i c_hi; //1
    unsigned char tdataR[32];
    unsigned char tdataG[32];
    unsigned char tdataB[32];
    
    

    //BEGIN LOOP
    //int vecPerLoop = 4;
    __m256i R; //2
    __m256i G; //3
    __m256i B; //4
    __m256i t1; //5
    __m256i t2; //6
    __m256i t3; //7
    __m256i isUnderHi; //8
    __m256i isAboveLo; //9
    __m256i inRange; //10
    __m256i mask_1; //11
    __m256i mask_2; //12
    __m256i shuffle1; //13
    __m256i shuffle2; //14
    __m256i shuffle3; //15

    mask_1 = _mm256_setr_epi8(0,255,255,0,255,255,0,255,255,0,255,255,0,255,255,0,255,255,0,255,255,0,255,255,0,255,255,0,255,255,0,255);
    mask_2 = _mm256_setr_epi8(0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0,255,0,0);
    shuffle1 = _mm256_setr_epi8(0,3,6,9,12,15,255,255,255,255,255,1,4,7,10,13,
                                0,3,6,9,12,15,255,255,255,255,255,1,4,7,10,13);
    shuffle2 = _mm256_setr_epi8(255,255,255,255,255,255,2,5,8,11,14,255,255,255,255,255,
                                255,255,255,255,255,255,2,5,8,11,14,255,255,255,255,255);

    long memOffset;

    int cur_pixels = 32;
    int new_pixels = pixels/32;
    new_pixels = new_pixels*32;
    printf("Pixels: %d\n",pixels);

    for(int i = 0; i*32 < pixels; i++){
        //printf("Cur pixel: %d\r",i*32);
        memOffset = i*96;
        if(pixels - i*32 < 32){
            cur_pixels = pixels - i*32 < 32;
            printf("HIT");
            maskHiLoRGB(&dataIn[memOffset],&dataOut[memOffset],r_lo,r_hi,g_lo,g_hi,b_lo,b_hi,cur_pixels);
            break;
        }

        //printf("Loading start\n");
        
        R = _mm256_loadu_si256((__m256i*) &dataIn[memOffset]); //32 bytes, mixed, starting at R
        B = _mm256_loadu_si256((__m256i*) &dataIn[memOffset+32]); //32 bytes, mixed, starting at B
        G = _mm256_loadu_si256((__m256i*) &dataIn[memOffset+64]); //32 bytes, mixed, starting at G

        //printf("Loading done\n");

        
        t1 = _mm256_blendv_epi8(R,B,mask_1); //
        t2 = _mm256_blendv_epi8(G,R,mask_1);
        t3 = _mm256_blendv_epi8(B,G,mask_1);

        R = _mm256_blendv_epi8(t1,t3,mask_2); // [0,12,23,1,13,24,2,...] | [0-5,12-16,23-27]
        G = _mm256_blendv_epi8(t2,t1,mask_2); // [22,0,12,23,1,13,24,...]
        B = _mm256_blendv_epi8(t3,t2,mask_2); // [11,22,0,12,23,1,13,...]


        //DO R
        //Can only shuffle within 128 bit lanes, so set up the two components necessary
        shuffle1 = _mm256_setr_epi8(0,3,6,9,12,15,255,255,255,255,255,1,4,7,10,13,
                                0,3,6,9,12,15,255,255,255,255,255,1,4,7,10,13);
        shuffle2 = _mm256_setr_epi8(255,255,255,255,255,255,2,5,8,11,14,255,255,255,255,255,
                                255,255,255,255,255,255,2,5,8,11,14,255,255,255,255,255);

        t1 = _mm256_shuffle_epi8(R,shuffle1); //[0,1,2,3,4,5|16,23,24,25,26,27|12,13,14,15]||[6|17,18,19,20,21,22|7,8,9,10,11|28,29,30,31]
        t2 = _mm256_shuffle_epi8(R,shuffle2); //[16,,2,3,4,5|16,23,24,25,26,27|12,13,14,15]||[6|17,18,19,20,21,22|7,8,9,10,11|28,29,30,31]
        R = _mm256_or_si256(t1,_mm256_permute4x64_epi64 (t2, 78));

        _mm256_storeu_si256((__m256i*) tdataR, R);


        //DO G
        shuffle1 = _mm256_setr_epi8(1,4,7,10,13,255,255,255,255,255,255,2,5,8,11,14,
                                1,4,7,10,13,255,255,255,255,255,255,2,5,8,11,14);
        shuffle2 = _mm256_setr_epi8(255,255,255,255,255,0,3,6,9,12,15,255,255,255,255,255,
                                255,255,255,255,255,0,3,6,9,12,15,255,255,255,255,255);

        t1 = _mm256_shuffle_epi8(G,shuffle1); //[0,1,2,3,4,5|16,23,24,25,26,27|12,13,14,15]||[6|17,18,19,20,21,22|7,8,9,10,11|28,29,30,31]
        t2 = _mm256_shuffle_epi8(G,shuffle2); //[16,,2,3,4,5|16,23,24,25,26,27|12,13,14,15]||[6|17,18,19,20,21,22|7,8,9,10,11|28,29,30,31]
        G = _mm256_or_si256(t1,_mm256_permute4x64_epi64 (t2, 78));

        //DO B
        shuffle1 = _mm256_setr_epi8(2,5,7,11,14,255,255,255,255,255,0,2,6,9,12,15,
                                2,5,7,11,14,255,255,255,255,255,0,2,6,9,12,15);
        shuffle2 = _mm256_setr_epi8(255,255,255,255,255,1,4,7,10,13,255,255,255,255,255,255,
                                255,255,255,255,255,1,4,7,10,13,255,255,255,255,255,255);

        t1 = _mm256_shuffle_epi8(B,shuffle1); //[0,1,2,3,4,5|16,23,24,25,26,27|12,13,14,15]||[6|17,18,19,20,21,22|7,8,9,10,11|28,29,30,31]
        t2 = _mm256_shuffle_epi8(B,shuffle2); //[16,,2,3,4,5|16,23,24,25,26,27|12,13,14,15]||[6|17,18,19,20,21,22|7,8,9,10,11|28,29,30,31]
        B = _mm256_or_si256(t1,_mm256_permute4x64_epi64 (t2, 78));



        //////////Compare R Threshold//////////////////
        c_lo = _mm256_set1_epi8(r_lo);
        c_hi = _mm256_set1_epi8(r_hi);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpeq_epi8( c_hi, _mm256_max_epu8(c_hi,R));
        isAboveLo = _mm256_cmpeq_epi8(R, _mm256_max_epu8(R,c_lo));
        inRange = _mm256_and_si256(isUnderHi, isAboveLo); //Initial val of inRange, set not modified
        ///////////////////////////////////////////////

        //////////Compare G Threshold//////////////////
        c_lo = _mm256_set1_epi8(g_lo);
        c_hi = _mm256_set1_epi8(g_hi);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpeq_epi8( c_hi, _mm256_max_epu8(c_hi,G));
        isAboveLo = _mm256_cmpeq_epi8(G, _mm256_max_epu8(G,c_lo));
        inRange = _mm256_and_si256(inRange, isAboveLo); //modify running mask
        inRange = _mm256_and_si256(isUnderHi, inRange); //modify running mask
        ///////////////////////////////////////////////

        //////////Compare B Threshold//////////////////
        c_lo = _mm256_set1_epi8(b_lo);
        c_hi = _mm256_set1_epi8(b_hi);
        //Compare and make mask for each loaded
        isUnderHi = _mm256_cmpeq_epi8( c_hi, _mm256_max_epu8(c_hi,B));
        isAboveLo = _mm256_cmpeq_epi8(B, _mm256_max_epu8(B,c_lo));
        inRange = _mm256_and_si256(inRange, isAboveLo); //modify running mask
        inRange = _mm256_and_si256(isUnderHi, inRange); //modify running mask
        ///////////////////////////////////////////////

        //Mask the vectors so everything in range remains
        R = _mm256_and_si256(R, inRange);
        G = _mm256_and_si256(G, inRange);
        B = _mm256_and_si256(B, inRange);
        

        //Store entire vector to the specific memory location
        //printf("3\n");
shuffle1 = _mm256_setr_epi8(0,1,2,3,4,5,11,12,13,14,15,6,7,8,9,10,
                                    0,1,2,3,4,5,11,12,13,14,15,6,7,8,9,10);
        R = _mm256_shuffle_epi8(R,shuffle1);

        shuffle1 = _mm256_setr_epi8(5,6,7,8,9,10,0,1,2,3,4,11,12,13,14,15,
                                    5,6,7,8,9,10,0,1,2,3,4,11,12,13,14,15);
        G = _mm256_shuffle_epi8(G,shuffle1);
        shuffle1 = _mm256_setr_epi8(10,11,12,13,14,15,5,6,7,8,9,0,1,2,3,4,
                                    10,11,12,13,14,15,5,6,7,8,9,0,1,2,3,4);
        B = _mm256_shuffle_epi8(B,shuffle1);

        shuffle3 = _mm256_setr_epi8(0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,
                                    0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255);
        
        t1 = _mm256_blendv_epi8(R,G,shuffle3);
        t2 = _mm256_blendv_epi8(B,R,shuffle3);
        t3 = _mm256_blendv_epi8(G,B,shuffle3);

        shuffle3 = _mm256_setr_epi8(255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,
                                    255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0);

        t1 = _mm256_blendv_epi8(B,t1,shuffle3);
        t2 = _mm256_blendv_epi8(G,t2,shuffle3);
        t3 = _mm256_blendv_epi8(R,t3,shuffle3);

        shuffle2 = _mm256_setr_epi8(0,6,11,1,7,12,2,8,13,3,9,14,4,10,15,5,
                                0,6,11,1,7,12,2,8,13,3,9,14,4,10,15,5);
        
        t1 = _mm256_shuffle_epi8(t1,shuffle2);
        t2 = _mm256_shuffle_epi8(t2,shuffle2);
        t3 = _mm256_shuffle_epi8(t3,shuffle2);

        shuffle3 = _mm256_setr_epi8(255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
        R = _mm256_blendv_epi8(_mm256_permute4x64_epi64 (t3, 78),t1,shuffle3);
        G = _mm256_blendv_epi8(t1,t2,shuffle3);
        B = _mm256_blendv_epi8(t2,_mm256_permute4x64_epi64 (t3, 78),shuffle3);

        //printf("Storing start\n");

        _mm256_storeu_si256((__m256i*) &dataOut[memOffset] , R);
        _mm256_storeu_si256((__m256i*) &dataOut[memOffset+32], G);
        _mm256_storeu_si256((__m256i*) &dataOut[memOffset+64], B);

        //printf("Storing end\n");

    }
}


