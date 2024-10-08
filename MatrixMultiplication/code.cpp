#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <algorithm> // for std::shuffle
#include <random>    // for std::random_device and std::mt19937

using namespace std;

// Structure to represent a sparse matrix in LIL format
struct SparseMatrix {
    vector<vector<int>> rows;   // Row-wise storage
    vector<vector<int>> cols;   // Column-wise storage
    vector<vector<double>> values; // Values corresponding to the indices
};

// Function to create a random sparse matrix
SparseMatrix createSparseMatrix(int rows, int cols, double sparsity) {
    SparseMatrix matrix;
    matrix.rows.resize(rows);
    matrix.cols.resize(rows);
    matrix.values.resize(rows);


    int totalSize = rows*cols;
    int numOnes = totalSize*sparsity;
    vector<int> nonZeroIndeces(totalSize);

    for (int i = 0; i < numOnes; i++) { 
        nonZeroIndeces[i] = 1;
    }

    // Shuffle the vector
    random_device rd;
    mt19937 g(rd());
    shuffle(nonZeroIndeces.begin(), nonZeroIndeces.end(), g);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (nonZeroIndeces[j+i*j] == 1) {
                matrix.rows[i].push_back(j);
                matrix.values[i].push_back(static_cast<double>(rand() % 10 + 1)); // Random value between 1 and 10
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

// Function to print a sparse matrix
void printSparseMatrix(const SparseMatrix &matrix) {
    for (size_t i = 0; i < matrix.rows.size(); ++i) {
        cout << "Row " << i << ": ";
        for (size_t j = 0; j < matrix.rows[i].size(); ++j) {
            cout << "(" << matrix.rows[i][j] << ", " << fixed << setprecision(2) << matrix.values[i][j] << ") ";
        }
        cout << endl;
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed for randomness

    // Define sizes and sparsity percentages
    vector<pair<int, double>> testCases = {
        {4, 0.75},  // 4x4 matrix with 75% sparsity
    };

    // Define test cases
    vector<int> sizes = {100, 1000};
    vector<double> sparsityVals = {0.01, 0.001};

    for (size_t i = 0; i < sizes.size(); ++i) {
        for (size_t j = 0; j < sparsityVals.size(); ++j) {
            size_t size = sizes[i];
            double sparsity = sparsityVals[i];
            
            cout << "Creating two " << size << "x" << size << " sparse matrices with sparsity " << sparsity * 100 << "%:" << endl;

            // Create two sparse matrices
            SparseMatrix A = createSparseMatrix(size, size, sparsity);
            SparseMatrix B = createSparseMatrix(size, size, sparsity);

            // Print input matrices
            cout << "Matrix A:" << endl;
            printSparseMatrix(A);
            cout << "Matrix B:" << endl;
            printSparseMatrix(B);

            // Multiply matrices
            SparseMatrix C = multiplySparseMatrices(A, B, size, size);
            cout << "Resulting Matrix C (A * B):" << endl;
            printSparseMatrix(C);
            cout << endl;
        }
    }

    return 0;
}
