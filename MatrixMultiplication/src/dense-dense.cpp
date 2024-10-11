// dense-dense.cpp: Functions to perform dense-dense matrix multiplication

#include<dense-dense.h> // Header file
#include <iomanip>      // std::invalid_argument, size_t
#include <thread>       // std::thread and related classes

// Function to multiply dense matrices
std::vector<std::vector<int>> multiplyDenseMatrices_none(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
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
std::vector<std::vector<int>> multiplyDenseMatrices_cache(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int blockSize) {
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Perform the dot product (matrix multiplication) with loop tiling
    for (int i = 0; i < rowsA; i += blockSize) {
        for (int j = 0; j < colsB; j += blockSize) {
            for (int k = 0; k < colsA; k += blockSize) {
                
                // Multiply blocks
                for (int ii = i; ii < std::min(i + blockSize, (int)rowsA); ++ii) {
                    for (int jj = j; jj < std::min(j + blockSize, (int)colsB); ++jj) {
                        for (int kk = k; kk < std::min(k + blockSize, (int)colsA); ++kk) {
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
void multiplyRowRange(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, std::vector<std::vector<int>>& result, int startRow, int endRow) {
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
std::vector<std::vector<int>> multiplyDenseMatrices_multithread(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int numThreads) {
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
std::vector<std::vector<int>> multiplyDenseMatrices_SIMD(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Perform the dot product (matrix multiplication)
    #pragma omp simd collapse(2)
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            for (size_t k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}


// Function to multiply dense matrices using optimization methods
std::vector<std::vector<int>> multiplyDenseMatrices_all(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int blockSize, int numThreads) {
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Lambda function to process a range of rows
    auto processRows = [&](int startRow, int endRow) {
        for (int i = startRow; i < endRow; i += blockSize) {
            for (int j = 0; j < colsB; j += blockSize) {
                for (int k = 0; k < colsA; k += blockSize) {
                    // Multiply blocks
                    for (int ii = i; ii < std::min(i + blockSize, (int)rowsA); ++ii) {
                        for (int jj = j; jj < std::min(j + blockSize, (int)colsB); ++jj) {
                            for (int kk = k; kk < std::min(k + blockSize, (int)colsA); ++kk) {
                                result[ii][jj] += A[ii][kk] * B[kk][jj];
                            }
                        }
                    }
                }
            }
        }
    };

    // Create and launch threads
    std::vector<std::thread> threads;
    int rowsPerThread = rowsA / numThreads;
    for (int t = 0; t < numThreads; ++t) {
        int startRow = t * rowsPerThread;
        int endRow = (t == numThreads - 1) ? rowsA : (t + 1) * rowsPerThread;
        threads.emplace_back(processRows, startRow, endRow);
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}