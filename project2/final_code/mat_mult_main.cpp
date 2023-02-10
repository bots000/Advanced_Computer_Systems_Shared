#include <immintrin.h>
#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include "mat_mult_float.cpp"
#include "mat_mult_uint16.cpp"
using namespace std;

int main(int argc, char *argv[]) {
    
    int len = atoi(argv[1]);
    string type = argv[2];

    if (type == "1"){
    	four_byte(len);
    }
    else if (type == "2"){
    	two_byte(len);
    }
    else{
    	cout << "Not a valid data type option entered - enter 1 for 4-byte floating point and 2 for 2-byte fixed point." << endl;
    	cout << type << endl;
    }

    return 0;

}