#include <iostream>
#include <vector>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()
#include <utility>  // For pair()

// Function to generate a matrix of desired sparsity
std::vector<std::vector<int>> generateMatrix(int rows, int cols, double fullPercent) {
    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols, 0));  // Initialize the matrix with zeros

    // Calculate the number of non-zero elements based on the "full" percentage
    int totalElements = rows * cols;
    int nonZeroElements = static_cast<int>((fullPercent / 100.0) * totalElements);

    // Randomly assign non-zero values to the matrix
    std::srand(static_cast<unsigned int>(std::time(0)));  // Seed for random number generator

    while (nonZeroElements > 0) {
        int row = std::rand() % rows;
        int col = std::rand() % cols;

        // Assign a random non-zero value if the current element is zero
        if (matrix[row][col] == 0) {
            matrix[row][col] = std::rand() % 10 + 1;  // Random values between 1 and 10
            nonZeroElements--;
        }
    }

    return matrix;
}

// Convert matrix to only include {col, value} for non-zero elements in each row
std::vector<std::vector<std::pair<int, int>>> createSparseVersion(std::vector<std::vector<int>>& mat) {
    // Acquire bounds
    int num_rows = mat.size();
    int num_cols = mat[0].size();

    // Create vectors for each row in new matrix format
    std::vector<std::vector<std::pair<int, int>>> sparseMat(num_rows);

    // Iterate through old matrix
    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            // Check for non-zero values
            if (mat[r][c] != 0) {
                sparseMat[r].emplace_back(c, mat[r][c]); // Add newly generated element = {col, value}
            }
        }
    }

    return sparseMat;
}

std::vector<std::vector<int>> multiplySparse(
    std::vector<std::vector<int>>& a, 
    std::vector<std::vector<int>>& b) {
    // Result of a(ra, ca) * b(rb, rb) = c(ra, cb)
    int num_rows = a.size();
    int num_cols = b[0].size();
    std::vector<std::vector<int>> ret(num_rows, std::vector<int>(num_cols));

    // Create compacted sparse versions of matrices
    std::vector<std::vector<std::pair<int, int>>> aSparse = createSparseVersion(a);
    std::vector<std::vector<std::pair<int, int>>> bSparse = createSparseVersion(b);

    // Perform optimized sparse matrix multiplication
    for (int r = 0; r < num_rows; ++r) {
        // Grab the column (k) and value (v1) of the entry
        for (const auto& entryA : aSparse[r]) {
            int k = entryA.first;
            int v1 = entryA.second;

            // Perform dot product using k and value from b mat (v2)
            for (const auto& entryB : bSparse[k]) {
                int c = entryB.first;
                int v2 = entryB.second;

                ret[r][c] += v1 * v2;
            }
        }
    }
    
    return ret;
}

int main() {
    // List of matrix sizes (rows, cols) and corresponding sparsity percentages (percent full)
    std::vector<std::pair<int, int>> matrixSizes = {{5, 5}, {10, 10}, {1500, 1500}};
    std::vector<double> fullPercentages = {20.0, 50.0, 80.0};  // Full percentages (not empty)

    for (const auto& size : matrixSizes) {
        for (double fullPercent : fullPercentages) {
            int rows = size.first;
            int cols = size.second;

            // Generate the sparse matrix for the given size and sparsity percentage
            std::cout << "Generating " << rows << "x" << cols << " matrix with " << fullPercent << "% full." << std::endl;
            std::vector<std::vector<int>> matA = generateMatrix(rows, cols, fullPercent);
            std::vector<std::vector<int>> matB = generateMatrix(rows, cols, fullPercent);
            
            // Convert the matrices and multiply
            multiplySparse(matA, matB);
        }
    }

    return 0;
}
