## Matrix Multiplication Optimization Project

## Table of Contents
- [Matrix Multiplication Optimization Project](#matrix-multiplication-optimization-project)
- [Table of Contents](#table-of-contents)
- [Project Highlights](#project-highlights)
- [Overview](#overview)
- [Implementation Details](#implementation-details)
  - [LIL (List of Lists) Sparse Matrix Storage](#lil-list-of-lists-sparse-matrix-storage)
  - [Optimization Techniques](#optimization-techniques)
- [Algorithms and Implementations](#algorithms-and-implementations)
  - [Sparse-Sparse Matrix Multiplication](#sparse-sparse-matrix-multiplication)
  - [Dense-Sparse Matrix Multiplication](#dense-sparse-matrix-multiplication)
  - [Dense-Dense Matrix Multiplication](#dense-dense-matrix-multiplication)
- [Results and Analysis](#results-and-analysis)
  - [Sparse-Sparse Matrix Multiplication](#sparse-sparse-matrix-multiplication-1)
  - [Dense-Sparse Matrix Multiplication](#dense-sparse-matrix-multiplication-1)
  - [Dense-Dense Matrix Multiplication](#dense-dense-matrix-multiplication-1)
  - [Sample Performance Comparison Table](#sample-performance-comparison-table)
- [Getting Started](#getting-started)
- [Compiler Flags](#compiler-flags)
- [Contributors](#contributors)

## Project Highlights

- Implemented Intel AVX2 SIMD instructions for manual data vectorization, leveraging parallel processing capabilities of modern CPUs
- Utilized OpenMP SIMD directives for automatic data vectorization, simplifying the optimization process
- Applied advanced cache optimization techniques, including loop tiling, to minimize memory access latency
- Developed multithreaded versions of matrix multiplication algorithms using both std::thread and OpenMP
- Combined multiple optimization strategies to achieve up to 142x speedup in matrix multiplication operations
- Employed gnuplot for comprehensive data visualization and performance analysis

## Overview

This project focuses on optimizing matrix multiplication operations, with a particular emphasis on sparse matrix computations. Matrix multiplication is a fundamental operation in numerous fields, including:

- Machine Learning and AI: Training neural networks and performing dimensionality reduction
- Scientific Computing: Solving systems of linear equations and performing numerical simulations
- Computer Graphics: Transforming 3D objects and rendering scenes
- Graph Analytics: Analyzing social networks and performing PageRank-like algorithms

Sparse matrices, which contain mostly zero elements, are prevalent in many real-world applications. By efficiently storing and operating only on non-zero values, sparse matrix operations can significantly reduce memory usage and computational requirements, enabling the processing of much larger datasets.

This project implements and compares various optimization techniques for three types of matrix multiplication:
1. Sparse-Sparse
2. Dense-Sparse
3. Dense-Dense

Each type presents unique challenges and opportunities for optimization, which this project explores in depth.

## Implementation Details

### LIL (List of Lists) Sparse Matrix Storage

We utilize the List of Lists (LIL) format for efficient sparse matrix storage. This format is particularly well-suited for incremental matrix construction and row-oriented operations.

LIL represents a sparse matrix as an array of rows, where each row contains two lists:
1. A list of column indices for non-zero elements
2. A corresponding list of non-zero values

Example:
```
Dense Matrix:
[0] [5] [0]
[1] [0] [0]
[0] [0] [3]

LIL representation:
Row 1: column indices [1], values [5]
Row 2: column indices [0], values [1]
Row 3: column indices [2], values [3]
```

This representation allows for efficient row-wise access and modification, which is crucial for our optimization strategies.

### Optimization Techniques

1. **Cache Optimization (Loop Tiling)**
   - Minimizes cache misses by improving spatial and temporal locality
   - Divides the computation into smaller blocks that fit in the cache
   - Reduces memory access latency and improves overall performance
   - Particularly effective for dense matrix operations and larger sparse matrices

2. **Multithreading**
   - Utilizes `std::thread` to divide work across 12 CPU threads, fully utilizing modern multi-core processors
   - Implements OpenMP for cleaner, compiler-driven multithreading, reducing the complexity of manual thread management
   - Balances workload across threads to maximize parallelism and minimize idle time

3. **SIMD Optimization**
   - Employs AVX2 SIMD instructions for parallel data processing, performing multiple floating-point operations simultaneously
   - Combines with OpenMP SIMD directives for enhanced vectorization, allowing the compiler to generate optimal SIMD code
   - Particularly effective for dense matrix operations and the innermost loops of sparse matrix multiplication

## Algorithms and Implementations

### Sparse-Sparse Matrix Multiplication

The sparse-sparse multiplication algorithm follows these steps:
1. Transpose the second input matrix for efficient column access
2. For each non-zero element in matrix A:
   - Identify corresponding elements in the transposed B matrix
   - Perform multiplication and accumulate results
3. Store only non-zero results in the output matrix

Optimizations applied:
- Loop tiling to improve cache utilization
- Multithreading to parallelize row-wise computations
- Custom SIMD instructions for the innermost loop of element-wise multiplication

### Dense-Sparse Matrix Multiplication

The dense-sparse multiplication leverages the sparsity of one matrix:
1. Iterate through the sparse matrix efficiently
2. For each non-zero element, perform a partial dot product with the corresponding dense matrix column
3. Accumulate results in the output matrix

Optimizations applied:
- Careful ordering of operations to maximize cache hits
- Tiling strategy adapted for the mixed dense-sparse structure
- Multithreading to distribute work across CPU cores
- SIMD instructions for vectorizing partial dot products

### Dense-Dense Matrix Multiplication

Standard matrix multiplication algorithm with the following optimizations:
- Cache-oblivious loop tiling to minimize cache misses
- OpenMP for both multithreading and SIMD directives
- Hand-tuned AVX2 instructions for maximum SIMD utilization

## Results and Analysis

### Sparse-Sparse Matrix Multiplication
- Test setup: Matrix sizes {3000, 4000, 5000}, Sparsity levels {0.1%, 1%, 10%}
- Key findings:
  - SIMD proved most effective for this case, especially at higher densities
  - Performance gains increase with matrix size and density
  - Combined optimizations achieved up to 34x speedup

![Sparse-Sparse Results](images/combinedSpSp.jpg)

### Dense-Sparse Matrix Multiplication
- Test setup: Matrix sizes {1200, 1500, 1800}, Sparsity levels {0.1%, 1%, 10%}
- Key findings:
  - Cache optimization was crucial due to varying matrix structures
  - Performance gains were most pronounced for larger, denser matrices
  - Combined optimizations achieved up to 36x speedup

![Dense-Sparse Results](images/combinedDSp.jpg)

### Dense-Dense Matrix Multiplication
- Test setup: Matrix sizes {1200, 1500, 1800}
- Key findings:
  - SIMD optimization was highly effective for dense matrices
  - Performance scaled nearly linearly with the number of cores utilized
  - Combined optimizations achieved up to 142x speedup

![Dense-Dense Results](images/dense-dense.png)

### Sample Performance Comparison Table

| Matrix Type     | Size | Sparsity | Baseline (s) | Cache (s) | Multithreading (s) | SIMD (s) | All Optimizations (s) | Speedup |
|-----------------|------|----------|--------------|-------------|-------------------|----------|----------------------|---------|
| Sparse-Sparse | 5000 | 0.1% | 0.016 | 0.036 | 0.007 | 0.026 | 0.019 | 0.84x |
| Sparse-Sparse | 5000 | 1% | 2.016 | 0.511 | 0.495 | 0.265 | 0.059 | 34.2x |
| Sparse-Sparse | 5000 | 10% | 16.724 | 1.934 | 4.841 | 2.027 | 0.746 | 22.4x |
| Dense-Sparse | 1800 | 0.1% | 0.167 | 0.011 | 0.043 | 0.132 | 0.006 | 27.8x |
| Dense-Sparse | 1800 | 1% | 1.608 | 0.114 | 0.348 | 1.36 | 0.044 | 36.5x |
| Dense-Sparse | 1800 | 10% | 13.072 | 0.777 | 2.557 | 11.747 | 0.404 | 32.3x |
| Dense-Dense | 1200 | N/A | 2.421 | 1.346 | 1.957 | 0.479 | 0.103 | 23.5x |
| Dense-Dense | 1500 | N/A | 12.788 | 2.68 | 5.052 | 1.15 | 0.22 | 58.1x |
| Dense-Dense | 1800 | N/A | 49.815 | 5.637 | 13.285 | 1.894 | 0.35 | 142.0x |

## Getting Started

1. Clone this repository:
   ```
   git clone https://github.com/AshtonRopp/AdvancedComputerSystems.git
   cd MatrixMultiplication
   ```

2. Ensure you have the necessary dependencies:
   - A C++ compiler with C++11 support
   - OpenMP support
   - AVX2-compatible CPU

3. Build the project:
   ```
   make
   ```

4. Run the executable:
   ```
   ./mult.exe [option] [thread_count]
   ```
   Options:
   - `dsp`: Dense-sparse multiplication
   - `spsp`: Sparse-sparse multiplication
   - `dd`: Dense-dense multiplication

   Example with thread count:
   ```
   ./mult.exe dsp 10
   ```
   Runs dense-sparse multiplication with 10 threads

## Compiler Flags

The following flags are used for optimization:
- `-pthread`: Enables std::thread support
- `-fopenmp-simd`: Enables OpenMP SIMD directives
- `-fopenmp`: Enables OpenMP support
- `-mavx2`: Enables AVX2 SIMD instructions
- `-O3`: Enables aggressive compiler optimizations

## Contributors

Ashton Ropp