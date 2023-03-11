BPlusTree.h is a class that makes a B+ tree: will make things such as finding if a string exists in the dictionary very fast
b_tree_test.cpp is a file that will use multithreading to make an encoded dictionary, given an input file.  It is currently functional, but is only tested on my teams.txt variants - not the columns given by prof.  Each thread takes 50 data values, makes a mini encoded dictionary, and returns it.  The main function then adds this dictionary with that of all of the other threads.

My current encoding will replace each string with an integer, corresponding to the earliest line (closest to 0) where that value first appeared.  Naturally, the dictionary takes the form (value:encoded_value).  By replacing it with the smallest integer, it hopefully decompresses data.

I did not implement any functionality other than reading in a data txt file, making an encoded dictionary via threading, and putting the values of this dictionary into the B+ tree.  We still need to actually convert the data to the encoded versions, use SIMD instructions to span it, and use the B+ tree functionality to search it, per the instructions of the assignment.
