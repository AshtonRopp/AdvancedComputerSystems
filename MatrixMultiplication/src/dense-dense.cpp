// dense-dense.cpp: Functions to perform dense-dense matrix multiplication

#include<dense-dense.h> // Header file
#include <iomanip>      // std::invalid_argument, size_t
#include <thread>       // std::thread and related classes
#include <immintrin.h>  // AVX2 intrinsics
#include <omp.h>        // OpenMP


// Function to multiply dense matrices
std::vector<std::vector<int>> multiplyDenseMatrices_none(
    const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {

    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Perform the dot product (matrix multiplication)
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            for (size_t k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

// Function to multiply dense matrices using loop tiling
std::vector<std::vector<int>> multiplyDenseMatrices_cache(
    const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int blockSize) {

    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Perform the dot product (matrix multiplication) with loop tiling
    for (int i = 0; i < rowsA; i += blockSize) {
        for (int j = 0; j < colsB; j += blockSize) {
            for (int k = 0; k < colsA; k += blockSize) {
                // Multiply blocks
                for (int ii = i; ii < std::min(i + blockSize, rowsA); ++ii) {
                    for (int jj = j; jj < std::min(j + blockSize, colsB); ++jj) {
                        for (int kk = k; kk < std::min(k + blockSize, colsA); ++kk) {
                            result[ii][jj] += A[ii][kk] * B[kk][jj];
                        }
                    }
                }
            }
        }
    }

    return result;
}

// Function to multiply a portion of the matrix
void multiplyRowRange(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B,
    std::vector<std::vector<int>>& result, int startRow, int endRow) {

    size_t colsA = A[0].size();
    size_t colsB = B[0].size();

    for (int i = startRow; i < endRow; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            for (size_t k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Function to multiply dense matrices using multithreading
std::vector<std::vector<int>> multiplyDenseMatrices_multithread(
    const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int numThreads) {

    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();

    // Ensure matrix multiplication is possible
    if (colsA != B.size()) {
        throw std::invalid_argument("Number of columns of A must equal number of rows of B.");
    }

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Determine how many rows each thread should compute
    int rowsPerThread = rowsA / numThreads;
    int remainingRows = rowsA % numThreads;

    std::vector<std::thread> threads;
    int startRow = 0;

    // Launch threads to process row ranges
    for (int t = 0; t < numThreads; ++t) {
        int endRow = startRow + rowsPerThread + (t < remainingRows ? 1 : 0);  // Distribute remaining rows
        threads.emplace_back(multiplyRowRange, std::ref(A), std::ref(B), std::ref(result), startRow, endRow);
        startRow = endRow;  // Move to the next range
    }

    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}

// Function to multiply dense matrices using SIMD instructions
std::vector<std::vector<int>> multiplyDenseMatrices_SIMD(
    const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {

    size_t rowsA = A.size();              // Get the number of rows in matrix A
    size_t colsA = A[0].size();          // Get the number of columns in matrix A (also the number of rows in matrix B)
    size_t colsB = B[0].size();          // Get the number of columns in matrix B

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Perform the dot product (matrix multiplication) using AVX2
    for (size_t i = 0; i < rowsA; ++i) {  // Iterate through each row of A
        for (size_t j = 0; j < colsB; ++j) {  // Iterate through each column of B
            __m256i sum = _mm256_setzero_si256(); // Initialize sum to zero (256-bit vector for accumulating results)

            // Compute the dot product of row i of A and column j of B
            for (size_t k = 0; k < colsA; k += 8) {  // Process 8 elements at a time
                // Load 8 integers from row i of A into a 256-bit SIMD register
                __m256i a = _mm256_loadu_si256((__m256i*)&A[i][k]);
                // Load 8 integers from column j of B into another 256-bit SIMD register
                __m256i b = _mm256_loadu_si256((__m256i*)&B[k][j]);

                // Multiply the elements in registers a and b. 
                // The result is also a 256-bit vector where each element is the product of corresponding elements of a and b.
                __m256i prod = _mm256_mullo_epi32(a, b);

                // Accumulate the products into the sum vector
                // Each element in prod is added to the corresponding element in sum
                sum = _mm256_add_epi32(sum, prod);
            }

            // Horizontal sum to extract the final result for result[i][j]
            // Store the sum vector into a temporary array
            int32_t temp[8];
            _mm256_storeu_si256((__m256i*)temp, sum);

            // Calculate the final result for result[i][j] by summing all elements in the temp array
            result[i][j] = temp[0] + temp[1] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6] + temp[7];
        }
    }

    return result;  // Return the resulting matrix
}

// Function to multiply dense matrices using optimization methods
std::vector<std::vector<int>> multiplyDenseMatrices_all(
    const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int blockSize, int numThreads) {

    size_t rowsA = A.size();              // Get the number of rows in matrix A
    size_t colsA = A[0].size();          // Get the number of columns in matrix A (also the number of rows in matrix B)
    size_t colsB = B[0].size();          // Get the number of columns in matrix B

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Set the number of threads for OpenMP
    omp_set_num_threads(numThreads);

    // Perform the dot product (matrix multiplication) using AVX2 with block multiplication
    #pragma omp parallel for collapse(2) // Parallelize outer two loops
    for (size_t i = 0; i < rowsA; i += blockSize) { // Iterate through blocks of rows of A
        for (size_t j = 0; j < colsB; j += blockSize) { // Iterate through blocks of columns of B
            // Each thread will work on a block
            for (size_t ii = i; ii < std::min(i + blockSize, rowsA); ++ii) { // Iterate through rows in the block
                for (size_t jj = j; jj < std::min(j + blockSize, colsB); ++jj) { // Iterate through columns in the block
                    __m256i sum = _mm256_setzero_si256(); // Initialize sum to zero (256-bit vector for accumulating results)

                    // Compute the dot product of row ii of A and column jj of B
                    for (size_t k = 0; k < colsA; k += 8) { // Process 8 elements at a time
                        // Load 8 integers from row ii of A into a 256-bit SIMD register
                        __m256i a = _mm256_loadu_si256((__m256i*)&A[ii][k]);
                        // Load 8 integers from column jj of B into another 256-bit SIMD register
                        __m256i b = _mm256_loadu_si256((__m256i*)&B[k][jj]);

                        // Multiply the elements in registers a and b. 
                        __m256i prod = _mm256_mullo_epi32(a, b);

                        // Accumulate the products into the sum vector
                        sum = _mm256_add_epi32(sum, prod);
                    }

                    // Horizontal sum to extract the final result for result[ii][jj]
                    int32_t temp[8];
                    _mm256_storeu_si256((__m256i*)temp, sum);

                    // Calculate the final result for result[ii][jj] by summing all elements in the temp array
                    result[ii][jj] += temp[0] + temp[1] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6] + temp[7];
                }
            }
        }
    }

    return result;  // Return the resulting matrix
}
