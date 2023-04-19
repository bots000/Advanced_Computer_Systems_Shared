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
    string path = "/mnt/c/Users/Kyle/Documents/School Folder/Advanced Computer Systems/Final Project/stb/images";
    int path_len = path.length() + 1;
    int total_initial_duration = 0;
    int total_threshold_duration = 0;
    int total_second_duration = 0;
    for (const auto & entry : fs::directory_iterator(path)){

        const char* image_path = entry.path().c_str();
        unsigned char* rgb_image = stbi_load(image_path, &width, &height, &bpp, 3);
        unsigned char* filt_img = (unsigned char*) aligned_alloc(32,3*width * height*sizeof(unsigned char));

        int red_counts[256] = {0};
        int green_counts[256] = {0};
        int blue_counts[256] = {0};

        auto start_initial_run = high_resolution_clock::now();

        RgbThresh(rgb_image, width, height, red_counts, green_counts, blue_counts);

        auto stop_initial_run = high_resolution_clock::now();
        auto duration_initial_run = duration_cast<microseconds>(stop_initial_run - start_initial_run);
        total_initial_duration += (int) duration_initial_run.count();

        auto start_threshold_run = high_resolution_clock::now();
        int red_threshold;
        int blue_threshold;
        int green_threshold;
        int max = distance(begin(red_counts), max_element(begin(red_counts), end(red_counts)));
        red_threshold = max * .7;
        max = distance(begin(green_counts), max_element(begin(green_counts), end(green_counts)));
        blue_threshold = max * .8;
        max = distance(begin(blue_counts), max_element(begin(blue_counts), end(blue_counts)));
        green_threshold = max * .85;

        auto stop_threshold_run = high_resolution_clock::now();
        auto duration_threshold_run = duration_cast<microseconds>(stop_threshold_run - start_threshold_run);
        total_threshold_duration += (int) duration_threshold_run.count();

        auto start_second_run = high_resolution_clock::now();

        maskHiLoRGB(rgb_image, filt_img, 0, red_threshold, 0 , blue_threshold, 0, green_threshold, width*height);


    

        auto stop_second_run = high_resolution_clock::now();
        auto duration_second_run = duration_cast<microseconds>(stop_second_run - start_second_run);
        total_second_duration += (int) duration_second_run.count();

        string image_path2(image_path);
        string img = image_path2.substr(path_len, image_path2.length()-path_len);

        char seg[]  = "/mnt/c/Users/Kyle/Documents/School Folder/Advanced Computer Systems/Final Project/stb/segmented_colored/";
        const char* new_filename = strcat(seg, img.c_str());

        stbi_write_png(new_filename, width, height, 3, filt_img, width*3);
        stbi_image_free(filt_img); 
        stbi_image_free(rgb_image); 
    }
    cout << total_initial_duration/1000 << endl;
    cout << total_threshold_duration/1000 << endl;
    cout << total_second_duration/1000 << endl;

    return 0;
}
    

    