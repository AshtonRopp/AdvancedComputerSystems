#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <algorithm> // for std::shuffle
#include <random>    // for std::random_device and std::mt19937

// Structure to represent a sparse matrix in LIL format
struct SparseMatrix {
    std::vector<std::vector<int>> rows;   // Row-wise storage
    std::vector<std::vector<int>> cols;   // Column-wise storage
    std::vector<std::vector<double>> values; // Values corresponding to the indices
};

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
                matrix.values[i].push_back(static_cast<double>(std::rand() % 10 + 1)); // Random value between 1 and 10
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
    std::vector<std::vector<int>> result(dense.size(), std::vector<int>(dense[0].size(), 0));

    for (size_t r = 0; r < num_rows; r++) {
        for (size_t c = 0; c < num_cols; c++) {

        }
    }

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

int main() {
    std::srand(static_cast<unsigned int>(std::time(0))); // Seed for randomness

    // Define test cases
    std::vector<size_t> sizes = {100, 1000, 10000};
    std::vector<double> sparsityVals = {0.001, 0.01};

    for (size_t i = 0; i < sizes.size(); ++i) {
        for (size_t j = 0; j < sparsityVals.size(); ++j) {
            size_t size = sizes[i];
            double sparsity = sparsityVals[j];
            
            // Create two sparse matrices
            SparseMatrix A = createSparseMatrix(size, size, sparsity);
            SparseMatrix B = createSparseMatrix(size, size, sparsity);
            
            std::cout << "Multiplying matrices of size " << size << "x" << size << " with sparsity " << sparsity * 100 << "%" << std::endl;
            // Multiply matrices
            SparseMatrix C = multiplySparseMatrices(A, B, size, size);
        }
    }

    return 0;
}
