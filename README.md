# Advanced Computer Systems Course Projects

## Overview
This repository contains performance optimization projects focused on cache profiling, matrix multiplication, and SSD performance testing. Each project demonstrates advanced programming techniques, system analysis, and data-driven optimizations to improve performance.

## Projects

### 1. Cache Profiling
This project explores how cache hierarchy and memory systems impact performance, measuring latency, bandwidth, and cache misses across various levels.

**Highlights:**
- Investigated cache structure using tools like `hwloc` and CPU-Z.
- Measured read/write latencies for L1, L2, L3 caches, and main memory.
- Assessed memory bandwidth for different data sizes to find optimal access patterns.
- Analyzed multithreading's effect on cache performance, revealing throughput and latency tradeoffs.
- Quantified performance impacts of cache and TLB misses using `perf` and Python for visualization.

### 2. Matrix Multiplication
This project focuses on optimizing matrix multiplication algorithms using SIMD vectorization, multithreading, and cache techniques.

**Highlights:**
- Implemented Intel AVX2 SIMD and OpenMP for data vectorization.
- Applied cache optimizations like loop tiling to minimize memory access latency.
- Developed multithreaded algorithms achieving up to 142x speedup.
- Used gnuplot for data visualization and performance analysis.

### 3. SSD Profiling
This project tests SSD performance, optimizing data access, and evaluating queue depths and access patterns with the FIO tool.

**Highlights:**
- Optimized SSD performance based on access size and queue depth.
- Automated experiment orchestration with Perl scripts.
- Collected and visualized performance data using FIO and Python.

### 4. Dictionary Codec
This project implements a dictionary codec for compression and memory latency reduction.

**Highlights:**
- Reduced memory latency by 80% using dictionary encoding
- Deployed SIMD instructions to promote effective data processing
- Utilized multithreading for search/scan operations

## Getting Started
Each project includes detailed setup instructions in its respective folder. Follow those to run the experiments and explore the results.
