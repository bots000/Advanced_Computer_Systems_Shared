#include <iostream>
#include "BPlusTree.h"
#include <fstream>
#include <string>
#include <map>
#include <unistd.h>
using namespace std;

struct datPack{    
    string team;
    int count;
};

map<string, int> team_dict;

// the function that is called for each worker thread
void* encode(void* dIn){

    // convert input back to relevant data types
    datPack* dat = (datPack*) dIn;

    //cout << (*dat).team << endl;

    team_dict[(*dat).team] = (*dat).count;

    // return compressed data pointer and size of this data
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

	
    
    // read in input
    /*int num_threads = atoi(argv[1]);
    char* input_file = argv[2];
    char* output_file = argv[3];*/

     cout << "Began" << endl;
    int num_threads = 6;

    // initalize tree
    BPlusTree<string> bpt(6);

    // read in file to get the total number of lines in count
    ifstream team_txt ("rand_teams.txt");

    int count = 0;
    string line;

    while (team_txt >> line){
        count++;
    }

    // get new instance of the file so that each line can be read in for its value
    string team;
    ifstream team_txt2 ("rand_teams.txt");

    // initialize the array of threads and array of data
    pthread_t thread_array[num_threads];
    datPack datArr[num_threads];
    datPack* tempPack;
    int index;
    int ret;

    for (int i = 0; i<num_threads; i++){
        datPack initPack;
        datArr[i] = initPack;
    }

	// find the value 0:

    // loop through every line in the file
    int counter = 0;
    while (team_txt2 >> team){

    	//sleep(.01);

    	
    	// get the index of the array for this line
    	index = (counter) % num_threads;

    	if (counter >= num_threads){//unpack threads
            
            // get the value of thread in current index
            ret = pthread_join(thread_array[index], NULL);
        }

        // fill the thread back up with the new line's data

        datPack initpack;
    	datPack* tempPack = &(initpack);

        //package data into tempPack
        (*tempPack).team = team;
        (*tempPack).count = count;
        
        // make the thread and add it to the correct spot of the array
        pthread_t temp_thread;
        ret = pthread_create(&temp_thread, NULL, encode, tempPack);
        thread_array[index] = temp_thread;
    	count--;
    	counter++;
    }

    map<string, int>::iterator i;
	for (i=team_dict.begin(); i!=team_dict.end(); ++i){
		bpt.insert((*i).first, (*i).second);
	}

    bpt.bpt_print();

    return 0;
}