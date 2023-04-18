#include <stdint.h>
#include <immintrin.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <chrono>
using namespace std::chrono;
using namespace std;

// first parameter: r5_b4_g4_r4_b3_g3_r3_b2_g2_r2_b1_g1_r1_b0_g0_r0
// second parameter: b7_g7_r7_b6_g6_r6_b5_g5
// last three: r,g,b passed by reference
void GatherRGBx8(const __m128i elements16, const __m128i elements8, __m128i &red, __m128i &green, __m128i &blue){
    //Shuffle mask for gathering 4 R elements, 4 G elements and 4 B elements (also set last 4 elements to duplication of first 4 elements).
    const __m128i shuffle_mask = _mm_set_epi8(9,6,3,0, 11,8,5,2, 10,7,4,1, 9,6,3,0);

    __m128i four_through_seven = _mm_alignr_epi8(elements8, elements16, 12);

    //Gather 4 R elements, 4 G elements and 4 B elements.
    //Remark: As I recall _mm_shuffle_epi8 instruction is not so efficient (I think execution is about 5 times longer than other shuffle instructions).
    __m128i zero_through_three_red_repeat = _mm_shuffle_epi8(elements16, shuffle_mask);
    __m128i four_through_seven_red_repeat = _mm_shuffle_epi8(four_through_seven, shuffle_mask);

    //Put 8 R elements in lower part.
    __m128i lower_red = _mm_alignr_epi8(four_through_seven_red_repeat, zero_through_three_red_repeat, 12);

    //Put 8 G elements in lower part.
    __m128i green_0_to_3_hi = _mm_slli_si128(zero_through_three_red_repeat, 8);
    __m128i green_4_to_7_lo = _mm_srli_si128(four_through_seven_red_repeat, 4);
    __m128i lower_green = _mm_alignr_epi8(green_4_to_7_lo, green_0_to_3_hi, 12);

    //Put 8 B elements in lower part.
    __m128i blue_0_to_3_hi = _mm_slli_si128(zero_through_three_red_repeat, 4);
    __m128i blue_4_to_7_lo = _mm_srli_si128(four_through_seven_red_repeat, 8);
    __m128i lower_blue = _mm_alignr_epi8(blue_4_to_7_lo, blue_0_to_3_hi, 12);

    //Unpack uint8 elements to uint16 elements.
    red = _mm_cvtepu8_epi16(lower_red);
    green = _mm_cvtepu8_epi16(lower_green);
    blue = _mm_cvtepu8_epi16(lower_blue);
}


//Y = 0.2989*R + 0.5870*G + 0.1140*B 
__m128i Rgb2Yx8(__m128i red, __m128i green, __m128i blue){
    //Each coefficient is expanded by 2^15, and rounded to int16 (add 0.5 for rounding).
    const __m128i r_coef = _mm_set1_epi16((short)(0.2989*32768.0 + 0.5));  //8 coefficients - R scale factor.
    const __m128i g_coef = _mm_set1_epi16((short)(0.5870*32768.0 + 0.5));  //8 coefficients - G scale factor.
    const __m128i b_coef = _mm_set1_epi16((short)(0.1140*32768.0 + 0.5));  //8 coefficients - B scale factor.

    //Multiply input elements by 64 for improved accuracy.
    red = _mm_slli_epi16(red, 6);
    green = _mm_slli_epi16(green, 6);
    blue = _mm_slli_epi16(blue, 6);

    //Use the special intrinsic _mm_mulhrs_epi16 that calculates round(r*r_coef/2^15).
    //Calculate Y = 0.2989*R + 0.5870*G + 0.1140*B (use fixed point computations)
    __m128i grey = _mm_add_epi16(_mm_add_epi16(_mm_mulhrs_epi16(red, r_coef),_mm_mulhrs_epi16(green, g_coef)),_mm_mulhrs_epi16(blue, b_coef));

    //Divide result by 64.
    grey = _mm_srli_epi16(grey, 6);

    return grey;
}

//Convert single row from RGB to Grayscale (use SSE intrinsics).
//I0 points source row, and J0 points destination row.
//I0 -> rgbrgbrgbrgbrgbrgb...
//J0 -> yyyyyy
static void Rgb2GraySingleRow_useSSE(const unsigned char I0[], const int image_width, unsigned char J0[], int grey_counts[255])
{
    int x;      //Index in J0.
    int srcx;   //Index in I0.
    __m128i red;
    __m128i green;
    __m128i blue;

    srcx = 0;

    //Process 8 pixels per iteration.
    for (x = 0; x < image_width; x += 8)
    {
        //Load 8 elements of each color channel R,G,B from first row.
        __m128i elements16 = _mm_loadu_si128((__m128i*)&I0[srcx]);     //Unaligned load of 16 uint8 elements
        __m128i elements8  = _mm_loadu_si128((__m128i*)&I0[srcx+16]);  //Unaligned load of (only) 8 uint8 elements (lower half of XMM register).

        //Separate RGB, and put together R elements, G elements and B elements (together in same XMM register).
        //Result is also unpacked from uint8 to uint16 elements.
        GatherRGBx8(elements16, elements8, red, green, blue);

        //Calculate 8 Y elements.
        __m128i grey = Rgb2Yx8(red,green,blue);
        grey_counts[(int) _mm_extract_epi16(grey, 0)] += 1;
        grey_counts[(int) _mm_extract_epi16(grey, 1)] += 1;
        grey_counts[(int) _mm_extract_epi16(grey, 2)] += 1;
        grey_counts[(int) _mm_extract_epi16(grey, 3)] += 1;
        grey_counts[(int) _mm_extract_epi16(grey, 4)] += 1;
        grey_counts[(int) _mm_extract_epi16(grey, 5)] += 1;
        grey_counts[(int) _mm_extract_epi16(grey, 6)] += 1;
        grey_counts[(int) _mm_extract_epi16(grey, 7)] += 1;


        //Pack uint16 elements to 16 uint8 elements (put result in single XMM register). Only lower 8 uint8 elements are relevant.
        __m128i new_image_row = _mm_packus_epi16(grey, grey);

        //Store 8 elements of Y in row Y0, and 8 elements of Y in row Y1.
        _mm_storel_epi64((__m128i*)&J0[x], new_image_row);

        srcx += 24; //Advance 24 source bytes per iteration.
    }
}


void Rgb2Gray_useSSE(unsigned char I[], int image_width,  int image_height, unsigned char J[], int grey_counts[255]){
    //I0 points source image row.
    unsigned char *I0;  //I0 -> rgbrgbrgbrgbrgbrgb...

    //J0 points destination image row.
    unsigned char *J0;  //J0 -> YYYYYY

    int y;  //Row index

    //Process one row per iteration.
    for (y = 0; y < image_height; y ++)
    {
        I0 = &I[y*image_width*3];       //Input row width is image_width*3 bytes (each pixel is R,G,B).

        J0 = &J[y*image_width];         //Output Y row width is image_width bytes (one Y element per pixel).

        //Convert row I0 from RGB to Grayscale.
        Rgb2GraySingleRow_useSSE(I0, image_width, J0, grey_counts);
    }

}