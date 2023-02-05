This code works for multiplying matrices that are not too large --- it can do a 640x640, but not a 768x768.
I don't think it will work if matrix size is not a multiple of 8.

The error gotten is a segmentation fault that occurs before the main function is run --- I think it must have to be due to the fact that at a certain point,
the data structures made are too large (hold too many pointers) that the allocated memory cannot be handled --- this is along the lines of something that
I saw online, but I do not completely understand it or why it is happening.  This is something I think we should talk about and look at together.

Other than that, though, the code is documented and runs well.  I put what you did in a function, then wrote code that will generate a bunch of these 8x8
matrices to multiply and add the values to the correct spots in the output matrix.  I might end up making too many variables for this to still be cache efficient
- the only thing that I truly wanted to have setup in memory was mat_o, but I ran into problems with getting sub-arrays of a_mat and b_mat, and then into problems
making what you had a function.  I think the move might be to figure out how to get sub-arrays with iterators (I once knew how but have forgotten) and take what you
wrote out of the function and just insert it back into main.
