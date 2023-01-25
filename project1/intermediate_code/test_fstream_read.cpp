#include <iostream>
#include <zstd.h>
#include <pthread.h>
#include <cstring>
#include <fstream>
#include <cctype>
using namespace std;

int main(){

	ifstream file ("example.txt");

	char * buffer = new char [16384];
	char * ch = new char [16834];
	file.read(buffer, 16384);
	ch = buffer;
	cout << ch;

	ofstream outfile ("output.txt");
	outfile.write(ch, strlen(ch));
	

	file.close();
	outfile.close();

	return 0;
}
