#!/bin/bash
read_percent_flags=('W3' 'W5' 'W6')
sizes=('b4k' 'b64k')
for size in "${sizes[@]}"
do
./mlc.exe --loaded_latency -$size
for percent_flag in "${read_percent_flags[@]}"
do
./mlc.exe --loaded_latency -$size -$percent_flag
done
done
