# Introduction

For this project, the students were tasked with implementing a dictionary codec using multithreading; the real-world applications of this are data compression and sped search/scan operations.  All three of these functions are implemented in this project.  This project demonstrates the use of a B tree and of SIMD instructions.  This report will detail how this was programmed, how to run the programs, experimental results gathered from running the programs, and the analysis of these results.

# Program Explanation

There are three main components of this project; the encoding, the search, and the B tree.  First, the process taken to encode the dictionary will be explained.  This was achieved using multithreading.  The main function will loop through the raw data provided, and dispatch chunks of it (defined by a set number of data items to be dispatched, referred to as a per_thread count) to the thread worker function.  This function goes through the dispatched data chunks item by item, creates a hashed value for each raw data item, and adds it to a local dictionary (c++ map).  These hashed values are generated using a built-in c++ hashing function, and are representative of the encoded values of the raw data.  Once this local dictionary is populated with all of the dispatched raw data items and the corresponding hashed (encoded) values, it is returend by the worker thread function.  When retrieved by the main function, this local dictionary is merged with a "master" dictionary, which will hold the raw data/encoded data parings for all raw data items.

Once all data is encoded through the process detailed above, the raw data/encoded data pairings are added to a B+ tree.  The foundation of the code used to create such a tree was found at https://github.com/solangii/b-plus-tree, and proper credit is given to the author of this code.  Group members of this project edited this code so that it could also hold an encoded value, but was still sorted on the raw data value.  Additionally, functions were added/deleted as needed for the desired functionality for this project.  Key functions include .insert(data, encoded_data), .find_index(array, find_data, len_of_array), and .range_search(starting_value, ending_value, result_data, result_array_length).  These aid in populating the B tree, finding if an element exists with it, and finding all elements within a certain range.  It is important to note that these find/scan functions take in the raw data as input, not encoded data.  Delete functions were not implemented, as this project uses a static B-tree (after all ementas are inserted), not a dynamic one.  The purpose of implementing the B-tree was to speed up the search/scan operations on the encoded dictionary; after its creation, finding if an element exists and finding all elements within a certain range (the two processes detailed by the prject instructions), become as trivial as calling the two above functions.

# How to Run


# Experimental Results
## Encoding Speed Results

## Single Data Item Search Speed Results

## Prefix Scan Speed Results


# Analysis
## Encoding Speed Analysis

## Single Data Item Search Speed Results

## Prefix Scan Speed Results

# Conclusion



