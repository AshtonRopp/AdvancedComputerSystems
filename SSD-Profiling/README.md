# SSD Profiling Project

## Project Highlights

- Optimized data access operations using access size and queueing length
- Developed Perl script for experiment orchestration
- Collected performance data using FIO tool
- Deployed Python script for graphing

## Overview
This project utilizes the FIO tool to test SSD latency and queue length. It will discuss the performance implications of data access size, read/write intensity, and I/O queue length. I will also attempt to maximize my SSD's performance and compare it to a standard server SSD.

## FIO Tool Background
FIO (Flexible I/O Tester) is a popular benchmarking tool used to measure and simulate I/O performance on storage devices and systems. It has the following key features: 
- **I/O Pattern Configuration**: Supports sequential, random, read, write, and mixed workloads.
- **Customizable**: Highly configurable with options for block size, I/O depth, threads, and more.
- **Cross-Platform Compatibility**: Available on Linux, Windows, WSL, and other systems.
- **Output**: Provides detailed performance metrics like throughput, latency, and IOPS.

For more details, visit the [FIO GitHub repository](https://github.com/axboe/fio).

## Key FIO Command Line Options for this Project
- `--bs=<int>`: sets the block size for each I/O operation during testing
- `--rw=<operation>`: sets the read/write operation. Let operation = `randread` or `randwrite` for random testing
- `--iodepth=<int>`: sets the maximum number of requests that can be queued
- `--size=<int>`: sets the size limit. You can use `GiB`, `MiB`, or `KiB` instead of 1024^x

## Data Access Size

## Read/Write Intensity

## I/O Queue Length

## Comparison to Intel Data Center NVME SSD D7-P5600

## Getting Started

1. Clone this repository:
   ```
   git clone https://github.com/AshtonRopp/AdvancedComputerSystems.git
   cd SSD-Profiling
   ```
2. Install dependencies:
   - JSON (Perl)
   - Text::CSV (Perl)
   - Pandas (Python)
   - Matplotlib (Python)
3. Edit test.pl to suit the runtime environment:
   - Set `$write_path` and `$read_path` to the desired locations
   - Edit the line `system("python3 graph.py $graphCMD");` to use a Python version which has Matplotlib
4. Allocate read/write storage for the experiment:
   ```
   perl test.p1 setup
   ```
5. Run the executable:
   ```
   perl test.pl run [option] [number of runs (optional)]
   ```
   Options:
   - `data_access`: Test various data access sizes
   - `rw_cent`: Test various read read/write ratios
   - `queue_depth`: Test various queue depths

   Example:
   ```
   ./perl test.pl run data_access
   ```
6. View the graphed data in plot.png
7. Remove SSD partitions after all testing is finished:
   ```
   perl test.pl clean
   ```
