#!/bin/bash
sub_folder="compression_data/"
under="_"
output="output"
zst=".zst"
creating="Creating "
for num_threads in 4 11 16 21 31 41 76 101
do
	for input_file in "large_text.txt" "large_video.mp4"
	do
		out_name="$sub_folder$output$under$num_threads$under$input_file$zst"
		g++ compress_with_threads.cpp -o outfile -lzstd
		echo "$creating$out_name"
		time ./outfile $num_threads $input_file $out_name
	done
done
exit 0
