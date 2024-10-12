// sparse-sparse.cpp: Functions to perform sparse-sparse matrix multiplication

#include <sparse-sparse.h> // Header file
#include <SparseMatrix.h>  // SparseMatrix
#include <thread>          // std::thread and related classes
#include <mutex>           // Used for mutex
#include <algorithm>       // For std::min

std::mutex resultMutex; // Mutex to protect shared result matrix

// Function to perform dense-sparse matrix multiplication with no optimization
SparseMatrix multiplySparseMatrices_none(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols) {

    // Initialize result matrix
    SparseMatrix result;
    result.rows.resize(resultRows);
    result.cols.resize(resultCols);
    result.values.resize(resultCols);

    // Begin multiplication
    for (size_t i = 0; i < A.rows.size(); ++i) {
        for (size_t j = 0; j < A.rows[i].size(); ++j) {
            int aCol = A.rows[i][j];
            double aValue = A.values[i][j];
            for (size_t k = 0; k < B.rows[aCol].size(); ++k) {
                int bCol = B.rows[aCol][k];
                double bValue = B.values[aCol][k];

                // Add the product to the result
                result.rows[i].push_back(bCol);
                result.values[i].push_back(aValue * bValue);
            }
        }
    }
    return result;
}

// Function to perform dense-sparse matrix multiplication with cache optimization
SparseMatrix multiplySparseMatrices_cache(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols, int blockSize){

    // Initialize result matrix
    SparseMatrix result;
    result.rows.resize(resultRows);
    result.cols.resize(resultCols);
    result.values.resize(resultCols);

    // Begin multiplication using loop tiling with the given block size
    for (int ii = 0; ii < A.rows.size(); ii += blockSize) {
        for (int jj = 0; jj < resultCols; jj += blockSize) {
            for (int kk = 0; kk < A.rows.size(); kk += blockSize) {
                // Multiply blocks
                for (int i = ii; i < std::min(ii + blockSize, static_cast<int>(A.rows.size())); ++i) {
                    for (int j = 0; j < A.rows[i].size(); ++j) {
                        int aCol = A.rows[i][j];
                        double aValue = A.values[i][j];
                        for (int k = kk; k < std::min(kk + blockSize, static_cast<int>(B.rows[aCol].size())); ++k) {
                            int bCol = B.rows[aCol][k];
                            double bValue = B.values[aCol][k];

                            // Add the product to the result
                            result.rows[i].push_back(bCol);
                            result.values[i].push_back(aValue * bValue);
                        }
                    }
                }
            }
        }
    }

    return result;
}

void multiplyPartial(const SparseMatrix& A, const SparseMatrix& B, SparseMatrix& result,
    int startRow, int endRow) {

    for (int i = startRow; i < endRow; ++i) {
        for (size_t j = 0; j < A.rows[i].size(); ++j) {
            int aCol = A.rows[i][j];
            double aValue = A.values[i][j];
            for (size_t k = 0; k < B.rows[aCol].size(); ++k) {
                int bCol = B.rows[aCol][k];
                double bValue = B.values[aCol][k];

                // Add the product to the result
                result.rows[i].push_back(bCol);
                result.values[i].push_back(aValue * bValue);
            }
        }
    }
}

SparseMatrix multiplySparseMatrices_multithread(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols, int numThreads) {

    // Initialize result matrix
    SparseMatrix result;
    result.rows.resize(resultRows);
    result.cols.resize(resultRows);
    result.values.resize(resultRows);

    // Determine the workload for each thread
    int rowsPerThread = resultRows / numThreads;
    std::vector<std::thread> threads;

    // Spawn threads to perform the matrix multiplication in parallel
    for (unsigned int t = 0; t < numThreads; ++t) {
        int startRow = t * rowsPerThread;
        int endRow = (t == numThreads - 1) ? resultRows : startRow + rowsPerThread;

        threads.push_back(std::thread(multiplyPartial, std::cref(A), std::cref(B), 
            std::ref(result), startRow, endRow));
    }

    // Join all threads
    for (auto& th : threads) {
        th.join();
    }

    return result;
}

SparseMatrix multiplySparseMatrices_SIMD(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols) {

    // Initialize result matrix
    SparseMatrix result;
    result.rows.resize(resultRows);
    result.cols.resize(resultCols);
    result.values.resize(resultCols);

    // Begin multiplication
    for (size_t i = 0; i < A.rows.size(); ++i) {
        for (size_t j = 0; j < A.rows[i].size(); ++j) {
            int aCol = A.rows[i][j];
            double aValue = A.values[i][j];
            #pragma omp simd
            for (size_t k = 0; k < B.rows[aCol].size(); ++k) {
                int bCol = B.rows[aCol][k];
                double bValue = B.values[aCol][k];

                // Add the product to the result
                result.rows[i].push_back(bCol);
                result.values[i].push_back(aValue * bValue);
            }
        }
    }
    return result;
}

// Function to multiply a block of the matrix in a thread
void multiplyBlock(const SparseMatrix& A, const SparseMatrix& B, SparseMatrix& result, 
    int ii, int jj, int kk, int blockSize, int resultCols) {

    for (int i = ii; i < std::min(ii + blockSize, static_cast<int>(A.rows.size())); ++i) {
        for (int j = 0; j < A.rows[i].size(); ++j) {
            int aCol = A.rows[i][j];
            double aValue = A.values[i][j];

            // Apply SIMD for this innermost loop, since it's the most compute-intensive
            #pragma omp simd
            for (int k = kk; k < std::min(kk + blockSize, static_cast<int>(B.rows[aCol].size())); ++k) {
                int bCol = B.rows[aCol][k];
                double bValue = B.values[aCol][k];

                // Add the product to the result (with thread safety)
                std::lock_guard<std::mutex> guard(resultMutex);
                result.rows[i].push_back(bCol);
                result.values[i].push_back(aValue * bValue);
            }
        }
    }
}

// Main function to multiply sparse matrices with multithreading
SparseMatrix multiplySparseMatrices_all(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols,
    int blockSize, int numThreads) {

    // Initialize result matrix
    SparseMatrix result;
    result.rows.resize(resultRows);
    result.cols.resize(resultCols);
    result.values.resize(resultCols);

    // Vector to hold threads
    std::vector<std::thread> threads;

    // Begin multiplication using loop tiling with multithreading
    for (int ii = 0; ii < A.rows.size(); ii += blockSize) {
        for (int jj = 0; jj < resultCols; jj += blockSize) {
            for (int kk = 0; kk < A.rows.size(); kk += blockSize) {
                if (threads.size() >= numThreads) {
                    // Wait for all threads to finish before continuing
                    for (auto& th : threads) {
                        th.join();
                    }
                    threads.clear();
                }

                // Launch a new thread to process the current block
                threads.emplace_back(multiplyBlock, std::cref(A), std::cref(B), std::ref(result),
                                     ii, jj, kk, blockSize, resultCols);
            }
        }
    }

    // Wait for remaining threads to finish
    for (auto& th : threads) {
        th.join();
    }

    return result;
}
