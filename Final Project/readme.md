# Introduction

This project was a student-picked project, with the criteria of using either SIMD instructions or multithreading to efficiently solve some applicable problem.  The group decided to focus their efforts on improving the efficiency of image segmentation using SIMD calcuations to speed up operations that typically need to be performed on a pixel-by-pixel basis.

Image segmentation is concerned with automatically obtaining information about an image by processing its contents by edges, shape, size, color, or some other means of comparison.  Doing so allows certain objects in the image to be singled out from the background.  This is commonly used in industry today, with applications including medical image analysis, satellite imaging, facial recognition, and autonmous vehicles.  This project focuses on the medical aspect, and uses a data set comprised of CT scans of multiple myeloma cells.  The data set was obtained from an IEEE grand challenge, and is available to be viewed online.  A sample image from the data set is shown below.

<img src="https://user-images.githubusercontent.com/89858703/233716971-34f0444b-f366-4014-a7f0-c3f2b70985a8.png"  width="512" height="384">

This project segments images such as this one using color thresholding.  Color thresholding is a process by which a "threshold" value is determined based on the color compilation of an image.  Based on which pixel color is most common, this color and similar colors can be filtered out of the image by ignoring all pixels at or above a threshold value around that most common color.  The end result is an image with a blacked out background, highlighting certain items within the image.  This projct aimed to highlight the nucelus of the multiple myeloma cells (darkest purple circles in the images) and black out everything else.  This thresholding was attempted by both converting the image to grayscale first and by using direct RGB thresholding.  SIMD instructions were used for the conversion to grayscale and for comparing pixel values to the given threshold.  A naive solution was also implemented for a performance comparison.

# Program Explanation

There are many components involved in this project's codebase, including: image reading and writing, grayscale conversion and thresholding, RGB thresholding, SIMD masking for grayscale threshold comparison, and SIMD masking for RGB threshold comparison.  The final product will perform the naive implementation, grayscale implementation, naive RGB implementation, and advanced RGB implementation.  Performing each of these will result in a new subset of segmented images; it is important to note that both the naive RGB implementation and RGB implementation resulted in the same segmented images, just with a runtime difference, so one set of results is stored to represent both of them.


# How to Use

Before compiling and running, it is important to collect your dataset, and store the images to be segmented in /images/.  This is the directory that will be searched for image loading at the top of each main function.

It is important to note that to obtain the segmented images, you first need to create the directory within whcih they will be stored.  This includes creating /segmented/, /segmented_simple/, /segmented_colored/, and segmented_colored_simple/ directories (four output data sets).  Each directory is populated by running a different script, and each will use a different version of the segmentation.  To see what programs to compile and run to perform the different types of segmentation, see the table below.

| Segmentation Type | File to Run | Directory for Output Images |
| --- | --- | --- |
| Naive grayscale | simple_thresholding.cpp | /segmented_simple/ |
| SIMD grayscale | full_grayscale_implementation.cpp | /segmented/ |
| Naive RGB | simple_thresholding_rgb.cpp | /segmented_colored_simple/ |
| Naive SIMD RGB |  | /segmented_colored/ |
| Advanced SIMD RGB | rgb_full_implementation.cpp | /segmented_colored/ |

It is important to note that the reading and writing is done using the stb package, which can be downloaded from the following github: https://github.com/nothings/stb.  The SIMD intrinsics header file also needs to be downloaded and included for this project.  The following line can be used to compile the code in this project: g++ -march=native -mavx correct_cpp_file_here.cpp -o outfile.  This compile can then be run using time ./outfile.

When a file has completed running, two things will happen.  Firstly, three numbers will be printed to the terminal.  The first is the number of microseconds taken to go through the image and collect the statistics on its pixel coloring.  This could mean converting to grayscale and returning the number of each grayscale value, or simply returning the number of each red, blue, and green value in the image.  The second number is the number of microseconds taken to compute the value threshold based on these statistics.  This number is often 0.  The third number is the number of microseconds taken to apply the threshold to the image via SIMD masking techniques.  It is important to note that gathering the image, reading it in, and writing out the image are not timed as they are constant for all runs and are not process dependent.

The other thing that happens is that the segmented image will be written out to the output directory.  Each image in the /image/ directory will have a corresponding image in the correct output directory.  It is important to note that the name of the image will not be changed, it will just be located in a different sub-folder.  This sub-folder will house all of the image segmented using that process.  There is no prevention to overwrite the images in these sub-folders, so only run the programs if you intend to overwrite the existing segmented images.

# Experimental Results

It is important to note that the following results were obtained on a machine with the following hardware characteristics.

| Property | Value |
| --- | --- |
| CPU Model | 11th Gen Intel(R) Core(TM) i7-1195G7 CPU @ 2.90GHz |
| # Cores | 4 |
| # Threads | 8 |
| Max Turbo Frequency | 5 GHz |
| Base Frequency | 2.92 GHz |
| Cache L1 | 320K |
| Cache L2 | 5 MB |
| Cache L3 | 12 MB |
|Ram | 32 GB DDR4 2666 Hz |

The results of each segmentation can be found in the sub directories on this github page.  It is too many to include here, but the segmentations were largely successful, with a few individual occasions of non-optimal segmentation in the RGB thresholding case.  Reasons for why this may be are discussed in the Analysis section.

How long each version of the segmentations implented took to both segment and mask/create the segmented image were recorded.  These runtimes can be found in the table below.

| Segmentation Type | Segmentation Time (milliseconds) | Masking Time (milliseconds) |
| --- | --- | --- |
| Naive grayscale | 16.586 | 4.636 |
| SIMD grayscale | 10.798 | .272 |
| Naive RGB | 14.267 | 8.09 |
| Naive SIMD RGB | 10.964 | 43.783 |
| Advanced SIMD RGB | 10.964 | 14.957 |


# Analysis




# Conclusion



