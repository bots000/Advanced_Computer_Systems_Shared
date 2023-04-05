#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

    unsigned char* rgb_image = stbi_load("example_png_copy.png", &width, &height, &bpp, 3);
    unsigned char gray_img[width * height];
    cout << (sizeof(gray_img)) << endl;
    int grey_counts[255];

    
    cout << width << endl;
    cout << height << endl;
    cout << bpp << endl;
    int r;
    int g;
    int b;

    auto start_initial_run = high_resolution_clock::now();
    cout << "clock started" << endl;
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            const stbi_uc *p = rgb_image + (3 * (x * width + y));
            r = int(p[0]);
            g = int(p[1]);
            b = int(p[2]);
            int g = int(0.299*r+0.587*g+0.114*b);
            //cout << g << endl;
            //cout << x*width+y << endl;
            gray_img[x*width+y] = (unsigned char) g;
            grey_counts[g]++;
            
        }
    }

    cout << "first run done" << endl;
    auto stop_initial_run = high_resolution_clock::now();
    auto duration_initial_run = duration_cast<microseconds>(stop_initial_run - start_initial_run);

    stbi_write_png("example_grey.png", width, height, 1, gray_img, width);

    auto start_second_run = high_resolution_clock::now();

    int threshold;
    int max = distance(begin(grey_counts), max_element(begin(grey_counts), end(grey_counts)));
    cout << max << endl;
    if (max > int(255/2)){
        threshold = int(max - ((max) * .1));
    }
    else{
        threshold = int(max + ((255-max) * .1));
    }
    cout << threshold << endl;

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if (int(gray_img[y*width+x]) < threshold){
                gray_img[y*width+x] = (unsigned char) 255;
            }
            else{
                gray_img[y*width+x] = (unsigned char) 0;
            }
            
        }
    }

    auto stop_second_run = high_resolution_clock::now();
    auto duration_second_run = duration_cast<microseconds>(stop_second_run - start_second_run);

    cout << "Time of initial run (us): " << duration_initial_run.count() << endl;
    cout << "Time of initial run (us): " << duration_second_run.count() << endl;

    stbi_write_png("example_grey_segment.png", width, height, 1, gray_img, width);

    

    stbi_image_free(rgb_image);

    return 0;
}