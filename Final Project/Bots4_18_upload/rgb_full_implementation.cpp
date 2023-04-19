#include <stdint.h>
#include <immintrin.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "simd_func.h"
#include "RGB_threshold.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <chrono>
using namespace std::chrono;
using namespace std;



int main() {
    
    int width, height, bpp;

    unsigned char* rgb_image = stbi_load("106.png", &width, &height, &bpp, 3);
    unsigned char* filt_img = (unsigned char*) aligned_alloc(32,3*width * height*sizeof(unsigned char));

    int red_counts[256] = {0};
    int green_counts[256] = {0};
    int blue_counts[256] = {0};

    RgbThresh(rgb_image, width, height, red_counts, green_counts, blue_counts);

    int red_threshold;
    int blue_threshold;
    int green_threshold;
    int max = distance(begin(red_counts), max_element(begin(red_counts), end(red_counts)));
    red_threshold = (255-max)*1.2;
    max = distance(begin(green_counts), max_element(begin(green_counts), end(green_counts)));
    blue_threshold = (255-max)*1.2;
    max = distance(begin(blue_counts), max_element(begin(blue_counts), end(blue_counts)));
    green_threshold = (255-max)*1.2;

    cout << red_threshold << ' ' << blue_threshold << ' ' << green_threshold << ' ' << endl;

    red_threshold = 200;
    blue_threshold = 200;
    green_threshold = 200;

    maskHiLoRGB(rgb_image, filt_img, 0, red_threshold, 0 , green_threshold, 150, blue_threshold, width*height);
    cout << "finished masking" << endl;

    stbi_write_png("try_filter_greyscale2.png", width, height, 3, filt_img, width*3);


    return 0;
}