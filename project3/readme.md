# Introduction


# Queueing Theory Overview


# Simulation Environment and Settings


# Sample Commands


# Experimental Results

Plans for going through results:
  - initially compare MLC vs. FIO (SSD takes at least 1000 times longer - consistent with what we got)
  - give FIO results
    - talk about the overall product increase as IO depth increases
    - talk about how BW increases and latency decreases as the file size increases at the same IO
    - talk about the differences that can be seen for read/write
  - give MLC results
    - explain loaded_latency results: as injected delay increases, the system is bottlenecking itself and using less utilization.  Therefore, the product is expected to decrease
    - there is no way to hold system utilization the same as we did with FIO, but we can see with larger data accesses, more utilization is needed (latency*BW)



As a sanity check before obtaining offical results, the same test was run on the SSD simulator and the memory simulator.  The run used was a 100% read command on 

**Memory Data Obtained Using MLC**
| Data Access Size | Read Intensity Ratio (100% read) | Throughput | Latency |
| --- | ----- | ----- | ----- |
| 32 B | 100% | - | - |
|  | 80% | - | - |
|  | 75% | - | - |
|  | 67% | - | - |
|  | 50% | - | - |
|  | 0% | - | - |
| 64 B | 100% | - | - |
|  | 80% | - | - |
|  | 75% | - | - |
|  | 67% | - | - |
|  | 50% | - | - |
|  | 0% | - | - |
| 128 B | 100% | - | - |
|  | 80% | - | - |
|  | 75% | - | - |
|  | 67% | - | - |
|  | 50% | - | - |
|  | 0% | - | - |
| 256 B | 100% | - | - |
|  | 80% | - | - |
|  | 75% | - | - |
|  | 67% | - | - |
|  | 50% | - | - |
|  | 0% | - | - |

**SSD Data Obtained Using FIO**
| Data Access Size | Read Intensity Ratio (100% read) | Read Throughput (kB/s) | Read Latency (nsec) | Write Throughput (kB/s) | Write Latency (nsec) | Average Throughput (kB/s) | Average Latency (nsec) |
| --- | ----- | ----- | ----- | --- | ----- | ----- | ----- |
| 16 KB | 100% | 3277 | 1144380 | 0 | 0 | 3277 | 1144380 |
|  | 80% | 2048 | 1722500 | 2048 | 9993.50 | 2048 | ----- |
|  | 75% | 1024 | 2919941 | 3072 | 11584 | ----- | ----- |
|  | 67% | - | - | --- | ----- | ----- | ----- |
|  | 50% | 2048 | 1912970 | 2048 | 12062.50 | 2048 | ----- |
|  | 0% | 0 | 0 | 16400 | 4522 | 16400 | 4522 |
| 32 KB | 100% | 5461 | 669610 | 0 | 0 | 5461 | 669610 |
|  | 80% | 4096 | 922530 | 2458 | 7163 | ----- | ----- |
|  | 75% | 4096 | 959100 | 4096 | 6218 | ----- | ----- |
|  | 67% | 4915 | 754950 | 1638 | 4046.50 | ----- | ----- |
|  | 50% | 3072 | 1096410 | 5120 | 5395.20 | ----- | ----- |
|  | 0% | 0 | 0 | 32800 | 2795.25 | 32800 | 2795.25 |
| 128 KB | 100% | 9362 | 425940 | 0 | 0 | 9362 | 425940 |
|  | 80% | 8937 | 440640 | 2979 | 5687.38 | ----- | ----- |
|  | 75% | 7282 | 512520 | 7282 | 4368 | 7282 | ----- |
|  | 67% | 7782 | 472900 | 5325 | 4140.92 | ----- | ----- |
|  | 50% | 7373 | 514910 | 5734 | 4826.57 | ----- | ----- |
|  | 0% | 0 | 0 | 131000 | 2472.50 | 131000 | 2472.500 |
| 256 KB | 100% | 10100 | 401340 | 0 | 0 | 10100 | 401340 |
|  | 80% | 10900 | 368350 | 3641 | 4969.63 | ----- | ----- |
|  | 75% | 9917 | 396130 | 3880 | 3685.39 | ----- | ----- |
|  | 67% | 8960 | 426760 | 7424 | 4251.10 | ----- | ----- |
|  | 50% | 9137 | 424540 | 11000 | 3792 | ----- | ----- |
|  | 0% | 0 | 0 | 250000 | 2587.05 | 250000 | 2587.05 |
| 1024 KB | 100% | 10900 | 370670 | 0 | 0 | 10900 | 370670 |
|  | 80% | 11100 | 361340 | 2500 | 6048.400 | ----- | ----- |
|  | 75% | 11000 | 361730 | 4155 | 6044.1 | ----- | ----- |
|  | 67% | 10900 | 361070 | 6245 | 5310.26 | ----- | ----- |
|  | 50% | 10800 | 367590 | 4718 | 5083.83 | ----- | ----- |
|  | 0% | 0 | 0 | 524000 | 4256.91 | - | - |



# Analysis



# Conclusion



