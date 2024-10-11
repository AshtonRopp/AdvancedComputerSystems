// Local headers
#include<dense-dense.h>

// External headers
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <thread>
#include <algorithm> // for std::shuffle
#include <random>    // for std::random_device and std::mt19937
#include <string.h>   //strcmp 

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

int main(int argc, char* argv[]) {
    // Seed for randomness
    std::srand(static_cast<unsigned int>(std::time(0)));

    // Define test cases
    std::vector<size_t> sizes = {100, 1000, 2000};
    std::vector<double> sparsityVals = {0.001, 0.01};

    // sparse-sparse
    if (strcmp(argv[1], "spsp") == 0) {
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
    else if (strcmp(argv[1], "dsp") == 0) {
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
