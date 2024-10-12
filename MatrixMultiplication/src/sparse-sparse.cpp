// sparse-sparse.cpp: Functions to perform sparse-sparse matrix multiplication

#include <sparse-sparse.h> // Header file
#include <SparseMatrix.h>  // SparseMatrix
#include <thread>          // std::thread and related classes
#include <algorithm>       // std::min
#include <unordered_map>   // unordered_map
#include <iostream>
#include <immintrin.h>



// Function to perform dense-sparse matrix multiplication with no optimization
SparseMatrix multiplySparseMatrices_none(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols) {

    SparseMatrix result;
    result.rows.resize(resultRows);
    result.values.resize(resultRows);

    // Transpose B for easier column access (because LIL format is row-centric)
    SparseMatrix B_transpose;
    B_transpose.rows.resize(resultCols);
    B_transpose.values.resize(resultCols);

    // Fill B_transpose by iterating over each row of B
    for (int i = 0; i < B.rows.size(); ++i) {
        for (int j = 0; j < B.rows[i].size(); ++j) {
            int col = B.rows[i][j];
            int value = B.values[i][j];
            B_transpose.rows[col].push_back(i);
            B_transpose.values[col].push_back(value);
        }
    }

    // Perform multiplication
    for (int i = 0; i < A.rows.size(); ++i) {
        std::unordered_map<int, int> row_result;  // Temporary result for the i-th row

        // Traverse through non-zero elements of A[i]
        for (int j = 0; j < A.rows[i].size(); ++j) {
            int a_col = A.rows[i][j];
            int a_value = A.values[i][j];

            // Multiply A's element with corresponding elements in B_transpose (i.e., B's column)
            for (int k = 0; k < B_transpose.rows[a_col].size(); ++k) {
                int b_col = B_transpose.rows[a_col][k];
                int b_value = B_transpose.values[a_col][k];

                // Add the product to the result
                row_result[b_col] += a_value * b_value;
            }
        }

        // Only store non-zero results in the final result matrix
        for (const auto& entry : row_result) {
            if (entry.second != 0) {
                result.rows[i].push_back(entry.first);
                result.values[i].push_back(entry.second);
            }
        }
    }

    return result;
}

SparseMatrix multiplySparseMatrices_cache(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols, int blockSize) {

    SparseMatrix result;
    result.rows.resize(resultRows);
    result.values.resize(resultRows);

    // Transpose B for easier column access
    SparseMatrix B_transpose;
    B_transpose.rows.resize(resultCols);
    B_transpose.values.resize(resultCols);

    // Pre-allocate memory for B_transpose
    for (const auto& row : B.rows) {
        for (int col : row) {
            B_transpose.rows[col].reserve(B.rows.size());
            B_transpose.values[col].reserve(B.rows.size());
        }
    }

    // Fill B_transpose
    for (int i = 0; i < B.rows.size(); ++i) {
        const auto& row = B.rows[i];
        const auto& values = B.values[i];
        for (size_t j = 0; j < row.size(); ++j) {
            int col = row[j];
            int value = values[j];
            B_transpose.rows[col].push_back(i);
            B_transpose.values[col].push_back(value);
        }
    }

    // Perform multiplication
    std::vector<int> row_result(resultCols, 0);
    for (int i = 0; i < A.rows.size(); ++i) {
        std::fill(row_result.begin(), row_result.end(), 0);

        const auto& A_row = A.rows[i];
        const auto& A_values = A.values[i];

        // Traverse through non-zero elements of A[i]
        for (size_t j = 0; j < A_row.size(); ++j) {
            int a_col = A_row[j];
            int a_value = A_values[j];

            const auto& B_row = B_transpose.rows[a_col];
            const auto& B_values = B_transpose.values[a_col];

            // Multiply A's element with corresponding elements in B_transpose
            for (size_t k = 0; k < B_row.size(); ++k) {
                row_result[B_row[k]] += a_value * B_values[k];
            }
        }

        // Store non-zero results in the final result matrix
        for (int col = 0; col < resultCols; ++col) {
            if (row_result[col] != 0) {
                result.rows[i].push_back(col);
                result.values[i].push_back(row_result[col]);
            }
        }
    }

    return result;
}


SparseMatrix multiplySparseMatrices_multithread(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols, int numThreads) {

    SparseMatrix result;
    result.rows.resize(resultRows);
    result.values.resize(resultRows);

    // Transpose B for easier column access (because LIL format is row-centric)
    SparseMatrix B_transpose;
    B_transpose.rows.resize(resultCols);
    B_transpose.values.resize(resultCols);

    // Fill B_transpose by iterating over each row of B
    for (int i = 0; i < B.rows.size(); ++i) {
        for (int j = 0; j < B.rows[i].size(); ++j) {
            int col = B.rows[i][j];
            double value = B.values[i][j];
            if (col < resultCols) { // Ensure col is within bounds
                B_transpose.rows[col].push_back(i);
                B_transpose.values[col].push_back(value);
            }
        }
    }

    // Worker function to handle multiplication for a range of rows
    auto worker = [&](int startRow, int endRow) {
        for (int i = startRow; i < endRow; ++i) {
            std::unordered_map<int, double> row_result;  // Temporary result for the i-th row

            // Traverse through non-zero elements of A[i]
            for (int j = 0; j < A.rows[i].size(); ++j) {
                int a_col = A.rows[i][j];
                double a_value = A.values[i][j];

                // Multiply A's element with corresponding elements in B_transpose
                for (size_t k = 0; k < B_transpose.rows[a_col].size(); ++k) {
                    int b_row = B_transpose.rows[a_col][k];
                    double b_value = B_transpose.values[a_col][k];

                    // Add the product to the result
                    row_result[b_row] += a_value * b_value;
                }
            }

            // Only store non-zero results in the final result matrix
            for (const auto& entry : row_result) {
                if (entry.second != 0) {
                    result.rows[i].push_back(entry.first);
                    result.values[i].push_back(entry.second);
                }
            }
        }
    };

    // Create threads
    std::vector<std::thread> threads;
    int totalRows = A.rows.size();
    int rows_per_thread = (totalRows + numThreads - 1) / numThreads; // Calculate how many rows each thread will handle

    for (int t = 0; t < numThreads; ++t) {
        int startRow = t * rows_per_thread;
        int endRow = std::min(startRow + rows_per_thread, totalRows);
        if (startRow < totalRows) {
            threads.emplace_back(worker, startRow, endRow);
        }
    }

    // Join threads
    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}


SparseMatrix multiplySparseMatrices_SIMD(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols) {

    SparseMatrix result;
    result.rows.resize(resultRows);
    result.values.resize(resultRows);

    // Transpose B for easier column access (because LIL format is row-centric)
    SparseMatrix B_transpose;
    B_transpose.rows.resize(resultCols);
    B_transpose.values.resize(resultCols);

    // Fill B_transpose by iterating over each row of B
    for (int i = 0; i < B.rows.size(); ++i) {
        for (int j = 0; j < B.rows[i].size(); ++j) {
            int col = B.rows[i][j];
            int value = B.values[i][j];
            B_transpose.rows[col].push_back(i);
            B_transpose.values[col].push_back(value);
        }
    }

    // Perform multiplication
    for (int i = 0; i < A.rows.size(); ++i) {
        std::vector<int> row_result(resultCols, 0);

        // Traverse through non-zero elements of A[i]
        for (int j = 0; j < A.rows[i].size(); ++j) {
            int a_col = A.rows[i][j];
            int a_value = A.values[i][j];

            // Using SIMD to process B_transpose[a_col]
            const auto& b_indices = B_transpose.rows[a_col];
            const auto& b_values = B_transpose.values[a_col];

            // Create a vector where all 8 32-bit integers are set to a_value
            __m256i a_value_vec = _mm256_set1_epi32(a_value);

            for (size_t k = 0; k < b_indices.size(); k += 8) {
                size_t rem = std::min(size_t(8), b_indices.size() - k);

                if (rem == 8) {
                    // Load 8 32-bit integers from b_values into a 256-bit vector
                    __m256i b_values_vec = _mm256_loadu_si256((__m256i*)&b_values[k]);

                    // Load 8 32-bit integers from b_indices into a 256-bit vector
                    __m256i b_indices_vec = _mm256_loadu_si256((__m256i*)&b_indices[k]);

                    // Multiply a_value_vec and b_values_vec element-wise
                    // Each 32-bit integer in a_value_vec is multiplied with the corresponding
                    // 32-bit integer in b_values_vec
                    __m256i prod = _mm256_mullo_epi32(a_value_vec, b_values_vec);

                    // Gather 8 32-bit integers from row_result using indices in b_indices_vec
                    // This is equivalent to:
                    // for (int l = 0; l < 8; ++l)
                    //     temp[l] = row_result[b_indices[k + l]];
                    __m256i row_result_vec = _mm256_i32gather_epi32(row_result.data(), b_indices_vec, 4);

                    // Add prod to the gathered values from row_result
                    row_result_vec = _mm256_add_epi32(row_result_vec, prod);

                    // Store results back to a temporary array
                    // We can't directly store to row_result because the indices might not be contiguous
                    int temp[8];
                    _mm256_storeu_si256((__m256i*)temp, row_result_vec);

                    // Copy values from temp back to row_result at the correct indices
                    for (int l = 0; l < 8; ++l) {
                        row_result[b_indices[k + l]] = temp[l];
                    }
                } else {
                    // Handle remaining elements (less than 8) using scalar operations
                    for (size_t l = 0; l < rem; ++l) {
                        int b_col = b_indices[k + l];
                        int b_value = b_values[k + l];
                        row_result[b_col] += a_value * b_value;
                    }
                }
            }
        }

        // Only store non-zero results in the final result matrix
        for (int col = 0; col < resultCols; ++col) {
            if (row_result[col] != 0) {
                result.rows[i].push_back(col);
                result.values[i].push_back(row_result[col]);
            }
        }
    }

    return result;
}

// Main function to multiply sparse matrices with multithreading
SparseMatrix multiplySparseMatrices_all(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols,
    int blockSize, int numThreads) {

    SparseMatrix result;
    result.rows.resize(resultRows);
    result.values.resize(resultRows);

    // Transpose B for easier column access
    SparseMatrix B_transpose;
    B_transpose.rows.resize(resultCols);
    B_transpose.values.resize(resultCols);

    // Pre-allocate memory for B_transpose
    for (const auto& row : B.rows) {
        for (int col : row) {
            B_transpose.rows[col].reserve(B.rows.size());
            B_transpose.values[col].reserve(B.rows.size());
        }
    }

    // Fill B_transpose
    for (int i = 0; i < B.rows.size(); ++i) {
        const auto& row = B.rows[i];
        const auto& values = B.values[i];
        for (size_t j = 0; j < row.size(); ++j) {
            int col = row[j];
            int value = values[j];
            B_transpose.rows[col].push_back(i);
            B_transpose.values[col].push_back(value);
        }
    }

    // Function to process a chunk of rows
    auto processChunk = [&](int startRow, int endRow) {
        std::vector<int> row_result(resultCols, 0);
        for (int i = startRow; i < endRow; ++i) {
            std::fill(row_result.begin(), row_result.end(), 0);

            const auto& A_row = A.rows[i];
            const auto& A_values = A.values[i];

            // Traverse through non-zero elements of A[i]
            for (size_t j = 0; j < A_row.size(); ++j) {
                int a_col = A_row[j];
                int a_value = A_values[j];

                const auto& B_row = B_transpose.rows[a_col];
                const auto& B_values = B_transpose.values[a_col];

                // Multiply A's element with corresponding elements in B_transpose
                for (size_t k = 0; k < B_row.size(); ++k) {
                    row_result[B_row[k]] += a_value * B_values[k];
                }
            }

            // Store non-zero results in the final result matrix
            for (int col = 0; col < resultCols; ++col) {
                if (row_result[col] != 0) {
                    result.rows[i].push_back(col);
                    result.values[i].push_back(row_result[col]);
                }
            }
        }
    };

    // Create and launch threads
    std::vector<std::thread> threads;
    int rowsPerThread = A.rows.size() / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int startRow = i * rowsPerThread;
        int endRow = (i == numThreads - 1) ? A.rows.size() : (i + 1) * rowsPerThread;
        threads.emplace_back(processChunk, startRow, endRow);
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}
