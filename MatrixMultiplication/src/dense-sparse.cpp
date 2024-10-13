// dense-sparse.cpp: Functions to perform dense-sparse matrix multiplication

#include <dense-sparse.h> // Header file
#include <SparseMatrix.h> // SparseMatrix
#include <iomanip>        // std::invalid_argument, size_t
#include <thread>         // std::thread and related classes
#include <immintrin.h>    // AVX2 SIMD instructions

// Function to multiply dense * sparse which have same base dimension
std::vector<std::vector<int>> multiplyDenseSparseMatrices_none(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse) {

    size_t num_rows = dense.size();
    size_t num_cols = dense[0].size();
    std::vector<std::vector<int>> result(num_rows, std::vector<int>(num_cols, 0));

    for (size_t r = 0; r < num_rows; r++) {
        // Check each entry of the current row in the sparse matrix
        for (size_t i = 0; i < sparse.rows[r].size(); i++) {
            size_t col = sparse.rows[r][i]; 
            int val = sparse.values[r][i];

            // With target column, multiply and accumulate rows from dense matrix
            for (size_t dense_r = 0; dense_r < num_rows; dense_r++) {
               result[dense_r][col] += dense[dense_r][col] * val;
            }
        }
    }
    return result;
}

// Function to perform dense-sparse matrix multiplication using loop tiling
std::vector<std::vector<int>> multiplyDenseSparseMatrices_cache(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse, int blockSize) {

    size_t num_rows = dense.size();
    size_t num_cols = dense[0].size();
    std::vector<std::vector<int>> result(num_rows, std::vector<int>(num_cols, 0));

    // Loop over dense matrix rows in blocks
    for (size_t r_block = 0; r_block < num_rows; r_block += blockSize) {
        for (size_t c_block = 0; c_block < num_cols; c_block += blockSize) {
            // Process the blocks
            for (size_t r = r_block; r < std::min(r_block + blockSize, num_rows); r++) {
                for (size_t i = 0; i < sparse.rows[r].size(); i++) {
                    size_t col = sparse.rows[r][i];
                    int val = sparse.values[r][i];

                    for (size_t dense_r = r_block; dense_r < std::min(r_block + blockSize, num_rows); dense_r++) {
                        result[dense_r][col] += dense[dense_r][col] * val;
                    }
                }
            }
        }
    }
    return result;
}

std::vector<std::vector<int>> multiplyDenseSparseMatrices_multithread(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse, int numThreads) {

    size_t num_rows = dense.size();
    size_t num_cols = dense[0].size();
    std::vector<std::vector<int>> result(num_rows, std::vector<int>(num_cols, 0));

    // Lambda function for multithreaded row processing
    auto processRows = [&](size_t start, size_t end) {
        for (size_t r = start; r < end; r++) {
            for (size_t i = 0; i < sparse.rows[r].size(); i++) {
                size_t col = sparse.rows[r][i];
                int val = sparse.values[r][i];

                for (size_t dense_r = 0; dense_r < num_rows; dense_r++) {
                    result[dense_r][col] += dense[dense_r][col] * val;
                }
            }
        }
    };

    // Determine the number of rows per thread
    size_t rowsPerThread = num_rows / numThreads;
    std::vector<std::thread> threads;

    // Launch threads to process different row ranges
    for (int t = 0; t < numThreads; t++) {
        size_t startRow = t * rowsPerThread;
        size_t endRow = (t == numThreads - 1) ? num_rows : startRow + rowsPerThread;
        threads.push_back(std::thread(processRows, startRow, endRow));
    }

    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}

std::vector<std::vector<int>> multiplyDenseSparseMatrices_SIMD(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse) {

    size_t num_rows = dense.size();
    size_t num_cols = dense[0].size();
    std::vector<std::vector<int>> result(num_rows, std::vector<int>(num_cols, 0));

    for (size_t r = 0; r < num_rows; r++) {
        // Check each entry of the current row in the sparse matrix
        for (size_t i = 0; i < sparse.rows[r].size(); i++) {
            size_t col = sparse.rows[r][i]; 
            int val = sparse.values[r][i];

            // Load dense matrix values for the target column using AVX2
            for (size_t dense_r = 0; dense_r < num_rows; dense_r += 8) { // Process 8 rows at a time
                // Ensure we don't exceed the bounds
                __m256i dense_vals = _mm256_setzero_si256(); // Initialize a zero vector for accumulation

                // Load up to 8 elements from the dense matrix into SIMD register
                for (size_t j = 0; j < 8 && (dense_r + j) < num_rows; j++) {
                    dense_vals = _mm256_insert_epi32(dense_vals, dense[dense_r + j][col], j);
                }

                // Multiply the dense values by the sparse value
                __m256i sparse_vals = _mm256_set1_epi32(val); // Broadcast the sparse value
                __m256i prod = _mm256_mullo_epi32(dense_vals, sparse_vals); // Multiply dense values with sparse value

                // Accumulate the products into the result matrix
                for (size_t j = 0; j < 8 && (dense_r + j) < num_rows; j++) {
                    result[dense_r + j][col] += _mm256_extract_epi32(prod, j);
                }
            }
        }
    }
    return result;  // Return the resulting matrix
}

std::vector<std::vector<int>> multiplyDenseSparseMatrices_all(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse, int blockSize, int numThreads) {

    size_t num_rows = dense.size();
    size_t num_cols = dense[0].size();
    std::vector<std::vector<int>> result(num_rows, std::vector<int>(num_cols, 0));

    // Lambda function to handle block processing for a row range
    auto processBlocks = [&](size_t startRow, size_t endRow) {
        #pragma omp simd collapse(2)
        for (size_t r_block = startRow; r_block < endRow; r_block += blockSize) {
            for (size_t c_block = 0; c_block < num_cols; c_block += blockSize) {
                // Process the blocks
                for (size_t r = r_block; r < std::min(r_block + blockSize, endRow); r++) {
                    for (size_t i = 0; i < sparse.rows[r].size(); i++) {
                        size_t col = sparse.rows[r][i];
                        int val = sparse.values[r][i];

                        for (size_t dense_r = r_block; dense_r < std::min(r_block + blockSize, endRow); dense_r++) {
                            result[dense_r][col] += dense[dense_r][col] * val;
                        }
                    }
                }
            }
        }
    };

    // Determine the number of rows per thread
    size_t rowsPerThread = num_rows / numThreads;
    std::vector<std::thread> threads;

    // Launch threads to process different row ranges
    for (int t = 0; t < numThreads; t++) {
        size_t startRow = t * rowsPerThread;
        size_t endRow = (t == numThreads - 1) ? num_rows : startRow + rowsPerThread;
        threads.push_back(std::thread(processBlocks, startRow, endRow));
    }

    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}
