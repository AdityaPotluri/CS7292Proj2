# CS7292 Lab2: Prime + Probe 

## Task-1: Histogram of Cache Hit/Miss Latency and LLC Configuration. (2 Points)
The two goals of this task are:
- [A] To figure out the LLC miss latency of the system 
- [B] To figure out the configuration of the LLC in the system.

For Part-A you will execute 10,000 L1 Hits and 10,000 LLC Misses and measure the latency for each access. Using the results, you will create and submit a histogram in your report. The starter code for this is available in `histogram.c` and `histogram.h`. At a high level you need to flush the data from LLC (using `clflush`), load an address and measure its time using `rdtsc/rdtscp`.

For Part-B you will have to figure out the configuration of your LLC. In particular we want you to populate the folowing table and add it in your report. You can get this information using `sysfs` in linux. You need to figure out how to find this information on a linux distro.


| Description | Value |
| -------- | ------- |
| Associativity |  |
| Sets |  |
| Number of Slices | |


** On Intel CPUs, each slice also has a subslice. The number of sets we are expecting in the response are per subslice.

** Read the background section of [Theory and Practice of Finding Eviction Sets](https://arxiv.org/abs/1810.01497) to learn the typical configuration of LLC.

** Based on the information of Task-1.B update `processor.h`

## Task-2: Build Eviction Sets using Group Testing. (6 Points)
The goal of this task is to build eviction sets using the group testing algorithm described in [Theory and Practice of Finding Eviction Sets](https://arxiv.org/abs/1810.01497).

The template code for this is given in `find_evsets.cpp`. You need to modify the `inflate` and the `reduce` function in `allocator.cpp`.

Additionally, you need to understand and explain how and why `evcition_set.cpp` accesses the elements in a specific way? Why can't it just accesses the elements by reading them once?


## Task-3: Use Prime+Probe to build a Covert Channel. (2 Points)

TODO.