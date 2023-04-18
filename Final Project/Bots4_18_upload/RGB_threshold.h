#include <stdint.h>
#include <immintrin.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <chrono>
using namespace std::chrono;
using namespace std;
#include <filesystem>
namespace fs = std::filesystem;

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

//Convert single row from RGB to Grayscale (use SSE intrinsics).
//I0 points source row, and J0 points destination row.
//I0 -> rgbrgbrgbrgbrgbrgb...
//J0 -> yyyyyy
static void RGB_extraction(const unsigned char I0[], const int image_width, int red_counts[256], int green_counts[256], int blue_counts[256])
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

        
        // take care of the counts for each color
        red_counts[(int) _mm_extract_epi16(red, 0)] += 1;
        red_counts[(int) _mm_extract_epi16(red, 1)] += 1;
        red_counts[(int) _mm_extract_epi16(red, 2)] += 1;
        red_counts[(int) _mm_extract_epi16(red, 3)] += 1;
        red_counts[(int) _mm_extract_epi16(red, 4)] += 1;
        red_counts[(int) _mm_extract_epi16(red, 5)] += 1;
        red_counts[(int) _mm_extract_epi16(red, 6)] += 1;
        red_counts[(int) _mm_extract_epi16(red, 7)] += 1;

        green_counts[(int) _mm_extract_epi16(green, 0)] += 1;
        green_counts[(int) _mm_extract_epi16(green, 1)] += 1;
        green_counts[(int) _mm_extract_epi16(green, 2)] += 1;
        green_counts[(int) _mm_extract_epi16(green, 3)] += 1;
        green_counts[(int) _mm_extract_epi16(green, 4)] += 1;
        green_counts[(int) _mm_extract_epi16(green, 5)] += 1;
        green_counts[(int) _mm_extract_epi16(green, 6)] += 1;
        green_counts[(int) _mm_extract_epi16(green, 7)] += 1;

        blue_counts[(int) _mm_extract_epi16(blue, 0)] += 1;
        blue_counts[(int) _mm_extract_epi16(blue, 1)] += 1;
        blue_counts[(int) _mm_extract_epi16(blue, 2)] += 1;
        blue_counts[(int) _mm_extract_epi16(blue, 3)] += 1;
        blue_counts[(int) _mm_extract_epi16(blue, 4)] += 1;
        blue_counts[(int) _mm_extract_epi16(blue, 5)] += 1;
        blue_counts[(int) _mm_extract_epi16(blue, 6)] += 1;
        blue_counts[(int) _mm_extract_epi16(blue, 7)] += 1;

        srcx += 24; //Advance 24 source bytes per iteration.
    }
}


void RgbThresh(unsigned char I[], int image_width,  int image_height, int red_counts[256], int green_counts[256], int blue_counts[256]){
    //I0 points source image row.
    unsigned char *I0;  //I0 -> rgbrgbrgbrgbrgbrgb...
    int y;  //Row index

    //Process one row per iteration.
    for (y = 0; y < image_height; y ++)
    {
        I0 = &I[y*image_width*3];       //Input row width is image_width*3 bytes (each pixel is R,G,B).

        //Convert row I0 from RGB to Grayscale.
        RGB_extraction(I0, image_width, red_counts, green_counts, blue_counts);
    }

}