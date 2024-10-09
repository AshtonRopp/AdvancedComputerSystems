#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <thread>
#include <algorithm> // for std::shuffle
#include <random>    // for std::random_device and std::mt19937

// Structure to represent a sparse matrix in LIL format
struct SparseMatrix {
    std::vector<std::vector<int>> rows;   // Row-wise storage
    std::vector<std::vector<int>> cols;   // Column-wise storage
    std::vector<std::vector<int>> values; // Values corresponding to the indices
};

// Create randomized matrix of size (row, col)
std::vector<std::vector<int>> createDenseMatrix(int rows, int cols) {
    std::vector<std::vector<int>> result(rows, std::vector<int>(cols, 0));

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = std::rand() % 10 + 1;
        }
    }

    return result;
}

// Function to create a random sparse matrix
SparseMatrix createSparseMatrix(int rows, int cols, double sparsity) {
    SparseMatrix matrix;
    matrix.rows.resize(rows);
    matrix.cols.resize(rows);
    matrix.values.resize(rows);

    int totalSize = rows * cols;
    int numOnes = totalSize * sparsity;
    std::vector<int> nonZeroIndices(totalSize);

    for (int i = 0; i < numOnes; i++) { 
        nonZeroIndices[i] = 1;
    }

    // Shuffle the vector
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(nonZeroIndices.begin(), nonZeroIndices.end(), g);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (nonZeroIndices[j + i * j] == 1) {
                matrix.rows[i].push_back(j);
                matrix.values[i].push_back(std::rand() % 10 + 1); // Random value between 1 and 10
            }
        }
    }
    return matrix;
}

// Function to multiply two sparse matrices in LIL format
SparseMatrix multiplySparseMatrices(const SparseMatrix &A, const SparseMatrix &B, int resultRows, int resultCols) {
    SparseMatrix result;
    result.rows.resize(resultRows);
    result.cols.resize(resultRows);
    result.values.resize(resultRows);

    for (int i = 0; i < A.rows.size(); ++i) {
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

// Function to multiply dense * sparse which have same base dimension
std::vector<std::vector<int>> multiplyDenseSparseMatrices(const std::vector<std::vector<int>> &dense, const SparseMatrix &sparse) {
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

// Function to multiply dense matrices
std::vector<std::vector<int>> multiplyDenseMatrices_none(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();

    // Initialize result matrix with zeros
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    // Perform the dot product (matrix multiplication)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
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
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
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


int main(int argc, char* argv[]) {
    // Seed for randomness
    std::srand(static_cast<unsigned int>(std::time(0)));

    // Define test cases
    std::vector<size_t> sizes = {100, 1000, 2000};
    std::vector<double> sparsityVals = {0.001, 0.01};

    // sparse-sparse
    if (argv[1] == "spsp") {
        for (size_t i = 0; i < sizes.size(); ++i) {
            for (size_t j = 0; j < sparsityVals.size(); ++j) {
                size_t size = sizes[i];
                double sparsity = sparsityVals[j];

                // Create two sparse matrices
                SparseMatrix A = createSparseMatrix(size, size, sparsity);
                SparseMatrix B = createSparseMatrix(size, size, sparsity);
                
                std::cout << "Multiplying sparse-sparse of size " << size << "x" << size << " with sparsity " << sparsity * 100 << "%" << std::endl;
                // Multiply matrices
                SparseMatrix C = multiplySparseMatrices(A, B, size, size);
            }
        }
    }
    
    // dense-sparse
    else if (argv[1] == "dsp") {
        for (size_t i = 0; i < sizes.size(); ++i) {
            for (size_t j = 0; j < sparsityVals.size(); ++j) {
                size_t size = sizes[i];
                double sparsity = sparsityVals[j];
                for (size_t j = 0; j < sparsityVals.size(); ++j) {
                    std::vector<std::vector<int>> A = createDenseMatrix(size, size);
                    SparseMatrix B = createSparseMatrix(size, size, sparsity);

                    std::cout << "Multiplying dense-sparse matrices of size " << size << "x" << size << " with sparsity " << sparsity * 100 << "%" << std::endl;

                    auto C = multiplyDenseSparseMatrices(A, B);
                }
            }
        }
    }

    // dense-dense
    else {
        for (size_t i = 0; i < sizes.size(); ++i) {
            size_t size = sizes[i];
            std::vector<std::vector<int>> A = createDenseMatrix(size, size);
            std::vector<std::vector<int>> B = createDenseMatrix(size, size);

            std::cout << "Multiplying dense-dense matrices of size " << size << "x" << size << std::endl;
            std::cout << "No optimization" << std::endl;
            auto C = multiplyDenseMatrices_none(A, B);
            std::cout << "Cache" << std::endl;
            C = multiplyDenseMatrices_cache(A, B, 64/sizeof(int));

            std::cout << "Multithreading" << std::endl;
            C = multiplyDenseMatrices_multithread(A, B, 12);
        }
    }

    return 0;
}
