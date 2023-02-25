# Introduction


# Queueing Theory Overview

emphasis on:

utilizaiton = arrival rate * average time to service a task (for same utilization, latency and bandwidth are inversly correlated)
and 
L_queue = util^2/(1-util) (for varying utilizaiton, latency and bandwidth are directly correlated)

# Simulation Environment and Settings

It is important to note that all simulations were performed on the same hardware device.  The specifications for this hardware device can be seen below.

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

Two simulation softwares were used to perform experiments.  The first of which is Intel's Memory Latency Checker (MLC).  MLC is a tool used to measure memory latencies and b/w, and how they change with increasing load on the system.  It was used for all memory simulations for this project.  It was downloaded at https://software.intel.com/content/www/us/en/develop/articles/intelr-memory-latency-checker.html.  MLC has several commands and capabilities, but this project was concerned on measuring the relationship between latency and bandwidth.  The only command capable of doing this was the --loaded_latency command.

The second simulation software used was Flexible I/O tester (FIO).  FIO is able to simulate a given I/O workload as specified by the user in a job file.  Details on downloading FIO can be found at https://github.com/axboe/fio.  FIO is significantly more flexible than MLC, and includes options such as holding latency or bandwidth constant, using different ioengines, etc.  It is important to note that the main results were generated on the libaio engine.  THis does not come installed with the FIO package, and needs to be manually installed to linux.  An example of how to do this is shown below.

[in the folder with FIO installed]

sudo apt update
sudo apt install libaio-dev
sudo make clean
sudo make

It is important to note that it is recommended to partition a portion of your hard drive when using FIO.  The partitioned drive used for these simulations was a 10 GB segment of the main hard drive.

# Simulation Commands

As previously mentioned, the only relevant command available in MLC is --loaded_latency.  This function measures how latency changes as bandwidth demand increases.  It does this by first loading the system as much as allowed to determine the maximum bandwidth.  It then injects artifical delays, throttling the bandwidth and effectively decreasing system utilization.  This is to simulate the system under different loads.  As injected delay approaches infinity, latency values approach idle values.  Injecting delay is equivalent to decreasing the iodepth in FIO.  Unfortunately, there is no way to hardset the latency or bandwidth values.  Additionally, server utilization is not known (it is just known how it changes with delay), so comparing throughput and latency at the same delay points on different runs does not gaurantee that they were measured at the same server utilization.  This makes it impossible to view how latency changes with changes in bandwidth at a constant utiliztion.  For this reason, the only trend attempted to be captured was if both latency and throughput decrease with decreased utilzation (more delay).  As per the project requirements, this was done for different sizes and read intesity ratios.  

MLC automatically injects delay, so no options had to be toggled to have this occur.  It was found in documentation that the -bX flag will perform the --loaded_latency command on data size of X (X=4k for a 4KB file).  Additionally, it was found that the -WX flag will alter the read intensity ratio.  Excluding this flag will result in 100% read, including -W3 will result in 75% read, including -W5 will result in 50% read, and including -W6 will result in 0% read (100% write).  An example of a MLC command for 75% read on a 4 KB file is shown below.  Another flag worth mentioning is the -d0 flag.  When this is observed, no delay is injected in the run, and only one latency/throuput combination is measured.  This is useful for observing supossed maximum bandwidth.

./mlc.exe --loaded_latency -b4k -W3

A range of commands were run, varying the data file size, read intensity ratio, and injected delay.  The bash script used to generate these runs is included as mlc_run.sh.  The resulting output from MLC is included in mlc_run_log and the important results are shown in the Experimental Results section.

Opposite of the little control given to the user by MLC, FIO allowed for nearly everything to be an option.  Particularly, server utilization could be set by the means of setting iodepth: increasing the iodepth increases the storage access queue depth.  Increasing this, according to queueing theory, should increase the bandwidth and latency of the run as well.  These results can be easily observed using FIO.  Additionally, FIO allows for read intensity ratio to be set.  Therefore, the MLC simulations could be mimicked with increased iodepth control.

To perform a FIO simulation, a job file had to be created.  The job file used can be found as ssd_run_fio_ini_file.txt.  It is important to note that the contents of this file were pasted as a .ini file, the extension required for FIO job files.  In this job file, all runs used the ioengine libaio - this is because default ioengines do not allow the measurement of bandwidth and latency at the same time, but libaio does.  The reading/writing sequence was set to randomly perform a mix of reads and writes, restricted to the specified ratio.  An example job file using the libaio engine, random read/writes with 75% read intensity, and the minimum iodpeth on a 4KB file is shown below.

[global]
ioengine=libaio
rw=randrw

[job1]
size=4k
iodepth=1
rwmixread=75

It is important to note that a minimum file size of 16 KB was used for FIO simulations - this is because for smaller files, the simulation did not reliably perform the needed reads and writes for a mixed intensity.  With larger file sizes, however, this issue was remedied.

# Experimental Results

As a baseline test, memory performance was compared to SSD performance.  It is known that even the fastest SSD has a latency 1000 times longer than DDR4.  Therefore, running the same command on both simulations should reflect this.  The run performed was a 100% read on a 4KB file.  The commands run are as shown below:

Fio: 

[global]

rw=randread

[job1]
size=4k

MLC:
./mlc.exe --loaded_latency -b4k -d0

The MLC results gave a latency of 1.5ns with a bandwidth of 292.54 GB/s.  The FIO results gave a latency of 1.425ms with a bandwidth of 2048KB/s.  As expected, the MLC (memory) latency was at least three orders of magnitude less than the FIO (SSD) latency.  Additionally, it is known that utilization=arrival rate * average response time, from which we can infer that, assuming roughly equal server utilization, as throughput increaes, latency will decrease.  It is important to note that this is not the case when server utilization is not held constant (which will be investigated later).  This explains the significantly higher bandwidth seen for memory as well.

This baseline test revealed that both simulators performed as expected.  With this confirmation, the following results were collected.

**SSD Data Obtained Using FIO**
| job# | Read BW (KB/s) | Read Lat (us) | Write BW (KB/s)| Write Lat (ns) | Read Intensity Ratio (%Read) | IO Depth | Data Size (KB) | AVG BW  | AVG LAT     | UTIL        |
|------|---------|-----------|----------|-----------|-------|----------|-----------|---------|-------------|-------------|
| 1    | 1024    | 3965.84   | 0        | 0         | 100   | 1        | 16        | 1024    | 3965840     | 4061020160  |
| 2    | 546     | 6976.39   | 546      | 12262     | 75    | 1        | 16        | 546     | 5235358     | 2858505468  |
| 3    | 315     | 12656.985 | 945      | 18045     | 50    | 1        | 16        | 630     | 6337515     | 3992634450  |
| 4    | 0       | 0         | 16400    | 13227     | 0     | 1        | 16        | 16400   | 13227       | 216922800   |
| 5    | 1092    | 4815.75   | 0        | 0         | 100   | 4        | 16        | 1092    | 4815750     | 5258799000  |
| 6    | 945     | 5130.27   | 315      | 932966    | 75    | 4        | 16        | 787.5   | 4080944     | 3213743400  |
| 7    | 372     | 10037.168 | 1117     | 6717560   | 50    | 4        | 16        | 744.5   | 8377364     | 6236947498  |
| 8    | 0       | 0         | 16400    | 36573.25  | 0     | 4        | 16        | 16400   | 36573.25    | 599801300   |
| 9    | 1024    | 5598.36   | 0        | 0         | 100   | 16       | 16        | 1024    | 5598360     | 5732720640  |
| 10   | 1024    | 4530.04   | 341      | 43926     | 75    | 16       | 16        | 853.25  | 3408511.5   | 2908312437  |
| 11   | 630     | 6748.52   | 630      | 12700380  | 50    | 16       | 16        | 630     | 9724450     | 6126403500  |
| 12   | 0       | 0         | 16400    | 22402.25  | 0     | 16       | 16        | 16400   | 22402.25    | 367396900   |
| 13   | 2849    | 1397.48   | 0        | 0         | 100   | 1        | 64        | 2849    | 1397480     | 3981420520  |
| 14   | 2587    | 1571.47   | 862      | 10171.25  | 75    | 1        | 64        | 2155.75 | 1181145.313 | 2546254007  |
| 15   | 2185    | 1741.51   | 2185     | 13893.5   | 50    | 1        | 64        | 2185    | 877701.75   | 1917778324  |
| 16   | 0       | 0         | 65500    | 7672.88   | 0     | 1        | 64        | 65500   | 7672.88     | 502573640   |
| 17   | 2979    | 3087.45   | 0        | 0         | 100   | 4        | 64        | 2979    | 3087450     | 9197513550  |
| 18   | 2731    | 3020.25   | 910      | 1279970   | 75    | 4        | 64        | 2275.75 | 2585180     | 5883223385  |
| 19   | 2409    | 2523.67   | 1446     | 1178270   | 50    | 4        | 64        | 1927.5  | 1850970     | 3567744675  |
| 20   | 0       | 0         | 65500    | 32804.69  | 0     | 4        | 64        | 65500   | 32804.69    | 2148707195  |
| 21   | 3277    | 6097.41   | 0        | 0         | 100   | 16       | 64        | 3277    | 6097410     | 19981212570 |
| 22   | 2891    | 4468.62   | 964      | 6441300   | 75    | 16       | 64        | 2409.25 | 4961790     | 11954192558 |
| 23   | 2048    | 4151.7    | 2633     | 2738300   | 50    | 16       | 64        | 2340.5  | 3445000     | 8063022500  |
| 24   | 0       | 0         | 65500    | 22342.56  | 0     | 16       | 64        | 65500   | 22342.56    | 1463437680  |
| 25   | 5461    | 745.86    | 0        | 0         | 100   | 1        | 128       | 5461    | 745860      | 4073141460  |
| 26   | 4282    | 916.59    | 1676     | 7925.89   | 75    | 1        | 128       | 3630.5  | 689423.9725 | 2502953732  |
| 27   | 3234    | 1177.44   | 3665     | 13926.18  | 50    | 1        | 128       | 3449.5  | 595683.09   | 2054808819  |
| 28   | 0       | 0         | 131000   | 6889.06   | 0     | 1        | 128       | 131000  | 6889.06     | 902466860   |
| 29   | 4855    | 2206.78   | 0        | 0         | 100   | 4        | 128       | 4855    | 2206780     | 10713916900 |
| 30   | 4096    | 2147.67   | 1365     | 1311840   | 75    | 4        | 128       | 3413.25 | 1938712.5   | 6617310441  |
| 31   | 3891    | 1963.42   | 2662     | 921550    | 50    | 4        | 128       | 3276.5  | 1442485     | 4726302103  |
| 32   | 0       | 0         | 131000   | 25266.38  | 0     | 4        | 128       | 131000  | 25266.38    | 3309895780  |
| 33   | 5461    | 5860.01   | 0        | 0         | 100   | 16       | 128       | 5461    | 5860010     | 32001514610 |
| 34   | 4282    | 5458.57   | 1676     | 4197070   | 75    | 16       | 128       | 3630.5  | 5143195     | 18672369448 |
| 35   | 4096    | 4412.48   | 2458     | 4512480   | 50    | 16       | 128       | 3277    | 4462480     | 14623546960 |
| 36   | 0       | 0         | 131000   | 27488.91  | 0     | 16       | 128       | 131000  | 27488.91    | 3601047210  |

**Memory Data Obtained Using MLC**
| Run # | Data Size (KB) | Read Intensity Ratio (% Read) | Delay  | Latency (ns) | BW (MB/sec) |
|-------|------|-------|--------|--------------|-------------|
| 1     | 4  | 100   | 0      | 1.63         | 283075.7    |
| 2     | 4  | 100   | 2      | 4.16         | 129797.4    |
| 3     | 4  | 100   | 8      | 5.61         | 80507.2     |
| 4     | 4  | 100   | 15     | 6.06         | 50775.7     |
| 5     | 4  | 100   | 50     | 6.4          | 24392.3     |
| 6     | 4  | 100   | 100    | 6.91         | 18367.1     |
| 7     | 4  | 100   | 200    | 5.01         | 18598       |
| 8     | 4  | 100   | 300    | 6.77         | 13469.2     |
| 9     | 4  | 100   | 400    | 6.14         | 13473.7     |
| 10    | 4  | 100   | 500    | 4.29         | 17373.3     |
| 11    | 4  | 100   | 700    | 2.24         | 30204.2     |
| 12    | 4  | 100   | 1000   | 4.72         | 14738.3     |
| 13    | 4  | 100   | 1300   | 4.28         | 15965.8     |
| 14    | 4  | 100   | 1700   | 4.47         | 15069       |
| 15    | 4  | 100   | 2500   | 4.55         | 14536.7     |
| 16    | 4  | 100   | 3500   | 4.31         | 15260.3     |
| 17    | 4  | 100   | 5000   | 4.9          | 13333.1     |
| 18    | 4  | 100   | 9000   | 5.4          | 11988.1     |
| 19    | 4  | 100   | 20000  | 5.34         | 12047.2     |
| 20    | 4  | 75    | 0      | 1.55         | 435194      |
| 21    | 4  | 75    | 2      | 3.28         | 208227.4    |
| 22    | 4  | 75    | 8      | 4.91         | 130909.4    |
| 23    | 4  | 75    | 15     | 5.01         | 121049.3    |
| 24    | 4  | 75    | 50     | 5.19         | 59971.9     |
| 25    | 4  | 75    | 100    | 4.5          | 49991.8     |
| 26    | 4  | 75    | 200    | 5.17         | 33824.5     |
| 27    | 4  | 75    | 300    | 4.59         | 29927.6     |
| 28    | 4  | 75    | 400    | 5.01         | 24042.5     |
| 29    | 4  | 75    | 500    | 4.3          | 24869.1     |
| 30    | 4  | 75    | 700    | 5.04         | 19587.5     |
| 31    | 4  | 75    | 1000   | 5.14         | 17329       |
| 32    | 4  | 75    | 1300   | 4.28         | 18993.2     |
| 33    | 4  | 75    | 1700   | 5.18         | 15149.3     |
| 34    | 4  | 75    | 2500   | 4.33         | 16887.3     |
| 35    | 4  | 75    | 3500   | 5.09         | 13984.4     |
| 36    | 4  | 75    | 5000   | 4.24         | 16190.7     |
| 37    | 4  | 75    | 9000   | 3.99         | 16606.5     |
| 38    | 4  | 75    | 20000  | 4.71         | 13827.6     |
| 39    | 4  | 50    | 0      | 1.54         | 514657.8    |
| 40    | 4  | 50    | 2      | 2.95         | 265696.3    |
| 41    | 4  | 50    | 8      | 5.18         | 139816.1    |
| 42    | 4  | 50    | 15     | 4.5          | 90399.8     |
| 43    | 4  | 50    | 50     | 5.36         | 39372.4     |
| 44    | 4  | 50    | 100    | 5.24         | 31467.2     |
| 45    | 4  | 50    | 200    | 5.28         | 23277       |
| 46    | 4  | 50    | 300    | 4.35         | 23134.5     |
| 47    | 4  | 50    | 400    | 5.23         | 17958.2     |
| 48    | 4  | 50    | 500    | 5.38         | 16501.2     |
| 49    | 4  | 50    | 700    | 5.57         | 14830.1     |
| 50    | 4  | 50    | 1000   | 4.86         | 15736.2     |
| 51    | 4  | 50    | 1300   | 5.15         | 14367.3     |
| 52    | 4  | 50    | 1700   | 5.32         | 13445.3     |
| 53    | 4  | 50    | 2500   | 5.12         | 13544.7     |
| 54    | 4  | 50    | 3500   | 5.12         | 13223.9     |
| 55    | 4  | 50    | 5000   | 5.39         | 12376       |
| 56    | 4  | 50    | 9000   | 5.6          | 11727.3     |
| 57    | 4  | 50    | 20000  | 5.27         | 12272.8     |
| 58    | 4  | 0     | 0      | 1.66         | 105518.4    |
| 59    | 4  | 0     | 2      | 3.81         | 60597.4     |
| 60    | 4  | 0     | 8      | 5.08         | 38392.7     |
| 61    | 4  | 0     | 15     | 4.81         | 23860.7     |
| 62    | 4  | 0     | 50     | 3.66         | 21380.5     |
| 63    | 4  | 0     | 100    | 3.51         | 20853.9     |
| 64    | 4  | 0     | 200    | 3.93         | 17740.7     |
| 65    | 4  | 0     | 300    | 4.34         | 15736.5     |
| 66    | 4  | 0     | 400    | 4.57         | 14767.4     |
| 67    | 4  | 0     | 500    | 4.69         | 14274.8     |
| 68    | 4  | 0     | 700    | 3.55         | 18526.3     |
| 69    | 4  | 0     | 1000   | 3.99         | 16401.6     |
| 70    | 4  | 0     | 1300   | 4.77         | 13639.2     |
| 71    | 4  | 0     | 1700   | 4.47         | 14487.7     |
| 72    | 4  | 0     | 2500   | 4.96         | 13021.6     |
| 73    | 4  | 0     | 3500   | 3.57         | 18009.6     |
| 74    | 4  | 0     | 5000   | 4.73         | 13601.4     |
| 75    | 4  | 0     | 9000   | 4.79         | 13394.5     |
| 76    | 4  | 0     | 20000  | 4.31         | 14859.2     |
| 77    | 64 | 100   | 0      | 4.25         | 234198.6    |
| 78    | 64 | 100   | 2      | 8.98         | 106459.4    |
| 79    | 64 | 100   | 8      | 13.76        | 57371       |
| 80    | 64 | 100   | 15     | 12.56        | 49147.4     |
| 81    | 64 | 100   | 50     | 5.28         | 26751.7     |
| 82    | 64 | 100   | 100    | 11.7         | 15595.2     |
| 83    | 64 | 100   | 200    | 14.67        | 9673.3      |
| 84    | 64 | 100   | 300    | 13.4         | 8800.9      |
| 85    | 64 | 100   | 400    | 13.87        | 7615        |
| 86    | 64 | 100   | 500    | 12.79        | 7618.7      |
| 87    | 64 | 100   | 700    | 12.6         | 7000.7      |
| 88    | 64 | 100   | 1000   | 14.12        | 5774.6      |
| 89    | 64 | 100   | 1300   | 14.36        | 5372.3      |
| 90    | 64 | 100   | 1700   | 13.89        | 5337.4      |
| 91    | 64 | 100   | 2500   | 13.79        | 5150        |
| 92    | 64 | 100   | 3500   | 12.91        | 5336        |
| 93    | 64 | 100   | 5000   | 14.03        | 4802.9      |
| 94    | 64 | 100   | 9000   | 14.27        | 4620.4      |
| 95    | 64 | 100   | 20000  | 13.72        | 4727.4      |
| 96    | 64 | 75    | 0      | 4.28         | 356202.2    |
| 97    | 64 | 75    | 2      | 9.38         | 167810.6    |
| 98    | 64 | 75    | 8      | 16.71        | 129622.8    |
| 99    | 64 | 75    | 15     | 12.98        | 108401.4    |
| 100   | 64 | 75    | 50     | 12.53        | 52672.7     |
| 101   | 64 | 75    | 100    | 14.65        | 36980       |
| 102   | 64 | 75    | 200    | 15.02        | 23797.1     |
| 103   | 64 | 75    | 300    | 14.99        | 18179.3     |
| 104   | 64 | 75    | 400    | 15.98        | 14419       |
| 105   | 64 | 75    | 500    | 14.93        | 13098.6     |
| 106   | 64 | 75    | 700    | 11.36        | 12025.7     |
| 107   | 64 | 75    | 1000   | 14.38        | 9139        |
| 108   | 64 | 75    | 1300   | 13.62        | 8615.4      |
| 109   | 64 | 75    | 1700   | 10.15        | 9536.8      |
| 110   | 64 | 75    | 2500   | 10.86        | 8090.1      |
| 111   | 64 | 75    | 3500   | 11.55        | 7079.2      |
| 112   | 64 | 75    | 5000   | 11.69        | 6570.2      |
| 113   | 64 | 75    | 9000   | 7.36         | 9312.2      |
| 114   | 64 | 75    | 20000  | 13.34        | 5033.3      |
| 115   | 64 | 50    | 0      | 4.29         | 400018      |
| 116   | 64 | 50    | 2      | 8.11         | 200304.7    |
| 117   | 64 | 50    | 8      | 8.58         | 130807.6    |
| 118   | 64 | 50    | 15     | 12.1         | 87516.2     |
| 119   | 64 | 50    | 50     | 12.63        | 32137.9     |
| 120   | 64 | 50    | 100    | 11.82        | 24062.6     |
| 121   | 64 | 50    | 200    | 12.33        | 16862.7     |
| 122   | 64 | 50    | 300    | 8.39         | 15375.6     |
| 123   | 64 | 50    | 400    | 12.72        | 11023.1     |
| 124   | 64 | 50    | 500    | 13.75        | 9193.8      |
| 125   | 64 | 50    | 700    | 12.53        | 8553.3      |
| 126   | 64 | 50    | 1000   | 12.78        | 7381.5      |
| 127   | 64 | 50    | 1300   | 13.49        | 6542.6      |
| 128   | 64 | 50    | 1700   | 12.51        | 6565.9      |
| 129   | 64 | 50    | 2500   | 12.47        | 6136        |
| 130   | 64 | 50    | 3500   | 13.61        | 5390.4      |
| 131   | 64 | 50    | 5000   | 13.23        | 5317.3      |
| 132   | 64 | 50    | 9000   | 11.65        | 5774.7      |
| 133   | 64 | 50    | 20000  | 12.22        | 5365.5      |
| 134   | 64 | 0     | 0      | 4.45         | 78938.1     |
| 135   | 64 | 0     | 2      | 8.66         | 49205.3     |
| 136   | 64 | 0     | 8      | 11.85        | 33432       |
| 137   | 64 | 0     | 15     | 13.71        | 14730.8     |
| 138   | 64 | 0     | 50     | 7.64         | 11819.9     |
| 139   | 64 | 0     | 100    | 13.84        | 6921.3      |
| 140   | 64 | 0     | 200    | 13.22        | 6167.3      |
| 141   | 64 | 0     | 300    | 10.05        | 7356.2      |
| 142   | 64 | 0     | 400    | 13.68        | 5381.3      |
| 143   | 64 | 0     | 500    | 13.54        | 5299.4      |
| 144   | 64 | 0     | 700    | 13.19        | 5301.3      |
| 145   | 64 | 0     | 1000   | 10.51        | 6437.3      |
| 146   | 64 | 0     | 1300   | 8.6          | 7730.8      |
| 147   | 64 | 0     | 1700   | 10.42        | 6352.4      |
| 148   | 64 | 0     | 2500   | 14.29        | 4610.2      |
| 149   | 64 | 0     | 3500   | 15.13        | 4318.9      |
| 150   | 64 | 0     | 5000   | 16.23        | 4005        |
| 151   | 64 | 0     | 9000   | 13.68        | 4711.8      |
| 152   | 64 | 0     | 20000  | 11.82        | 5431.7      |

# Analysis
After physical testing, the latency between DRAM and solid state storage is shown to be 1000x longer. This is consistent with estimates previously mentioned above. Beyond this, these two mediums need to be analyzed separately. 

## SSD Performance

First, from the performance data, it can be seen that the write-only IOPS are significantly higher than that of the enterprise-grade Intel D7-P5600. This can be attributed to the fact that this enterprise-grade SSD is most likely designed for several more important characteristics. For instance, data reliability, physical resilience (specifically heat), rewriteability/lifespan could all be design areas more important to an enterprize application than the speed of an individual drive. For the consumer, however, speed is often the only specification considered and therefore very important to a manufacturer on consumer products. This would explain the higher IOPS observed

With FIO, there were many different elements which could be tweaked to get interesting and relevant data. However, with so many knobs to turn, there are many overlapping effects which need to be isolated. The this section will walk through all different elements and explain what effect each variable had. For comparison, utilization is approximated by taking the latency - bandwidth product. This is because these are both indicators of usage.

First, as shown below, utilization scaled with file size. This is fairly logical and is consistent with expectations. Larger file sizes will naturally be able to utilize the resources available, therefore increasing the overall utilization. 

![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/SSD_Util_vs_Size.png?raw=true)

Next, utilization scaled with increased read %. This is likely due to the writes being handled somewhat transparently by the ssd. This would allow for writes to "occur" in a fraction of their actual time. Reads, however, cannot be handled transparently because they need to actually access the contents and the file is not in memory.

![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/SSD_Util_vs_Read_D4.png?raw=true)

Below are several graphs showing the utilization vs the IO depth (effectively queue size per thread) isolated by read percentage. As anticipated, greater queue size increases the uilization (except for pure writes). In the case of pure writes, as seen below, it peaks and then decreases for an IO depth of 16. The most logical explanation is that the ability of the SSD to handle writes transparently was saturated and thus became limited in the longer trial of more writes.

![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/SSD_Util_vs_Depth_R100.png?raw=true)
![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/SSD_Util_vs_Depth_R50.png?raw=true)
![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/SSD_Util_vs_Depth_R0.png?raw=true)

Finally, it is important to view the latency as IO depth increases (and thus queue length). Below are the graphs showing the latency trend as IO depth increases again isolated by different read percentages. As can be anticipated, in the cases where queue length is larger and utilization is higher (previous graphs), latency is also increasing. Again, though at reading 0%, the behavior is unexpected and is likely tied to the transparent behavior described prior.

![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/SSD_Latency_vs_Depth.png?raw=true)

## DRAM Performance
With MLC, there are far fewer knobs to turn when it comes to testing. The main means of variance comes from the delay injection, which effectively decreases lambda, or the rate of requests being made. As shown below, by increasing the delay, the bandwidth decreases significantly, which is fairly logical as the throughput of the device will be much lower if the memory is relatively inactive. Interestingly, there seemed to be no particular correlation between delay and latency in either size case. This could be due to memory bandwidth never getting fully saturated (able to service all requests in the same timeframe because they didn't cause a queue). Perhaps a larger size would yield different results. In any case latency seemed unaffected in all cases.

![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/DRAM_BW_vs_Delay.png?raw=true)
![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/DRAM_Latency_vs_Delay.png?raw=true)

Another difference compared to the SSD is the effect of read percentage on bandwidth and latency. First, as shown below, for both 4B and 64B, the latency is effectively uncorrelated with the read percentage. There are certainly some variances, but they can't be easily attributed to the read percentage. For bandwidth, however, we can see that the bandwidth peaks with a mixed read/write setting. This can be reasoned to be due to potential parallelizations that can occur or better optimizations the memory controller can make when handling both requests. In either case, it becomes clear that a mixed queue raises the average IO bandwidth.

![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/DRAM_Latency_vs_Read.png?raw=true)
![alt text](https://github.com/bots000/Advanced_Computer_Systems_Shared/blob/main/project3/Graphs/DRAM_BW_vs_Read.png?raw=true)

# Conclusion

Queueing Theory can explain much of the behavior exhibited by these devices. In many cases, when queue size increased, average bandwidth also increased. In cases where a queue was able to form, latency grew. However, certain functionalities can lead to potentially counter-intuitive behavior. Transparent functionality in the SSD could lead to results that are inconsistent with trends and cause activities to not scale uniformly. As well, memory not getting saturated by requests could mean that latency is not observed to be affected, despite the fact that it will have to at some point suffer greater delays. Overall, this project was successful in correlating the theory behind Queueing Theory to real, empirical data. 

