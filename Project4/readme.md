# Introduction

For this project, the students were tasked with implementing a dictionary codec using multithreading; the real-world applications of this are data compression and sped search/scan operations.  All three of these functions are implemented in this project.  This project demonstrates the use of a B tree and of SIMD instructions.  This report will detail how this was programmed, how to run the programs, experimental results gathered from running the programs, and the analysis of these results.

# Program Explanation

There are three main components of this project; the encoding, the search, and the B tree.  First, the process taken to encode the dictionary will be explained.  This was achieved using multithreading.  The main function will loop through the raw data provided, and dispatch chunks of it (defined by a set number of data items to be dispatched, referred to as a per_thread count) to the thread worker function.  This function goes through the dispatch data chunks item by item, creates a hashed value for each raw data item, and adds it to a local dictionary (c++ map).  These hashed values are generated using a built-in c++ hashing function, and are representative of the encoded values of the raw data.  Once this local dictionary is populated with all of the dispatched raw data items and the corresponding hashed (encoded) values, it is returend by the worker thread function.  When retrieved by the main function, this local dictionary is merged with a "master" dictionary, which will hold the raw data/encoded data parings for all raw data items.

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



