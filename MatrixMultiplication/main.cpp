// Local headers
#include<dense-dense.h>

// External headers
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <thread>
#include <algorithm> // std::shuffle
#include <random>    // std::random_device and std::mt19937
#include <string.h>  // strcmp 
#include <fstream>   // std::ofstream


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

    // Timing vectors for gnuplot
    std::vector<double> times_none, times_cache, times_multithread, times_SIMD, times_all;

    // Define test cases
    std::vector<size_t> sizes = {500, 1000};
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
            std::vector<std::vector<int>> C;
            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();

            std::cout << "Multiplying dense-dense matrices of size " << size << "x" << size << std::endl;

            // No optimization
            if (size < 10000) {
                std::cout << "Running operation with no optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplyDenseMatrices_none(A, B);
                end = std::chrono::high_resolution_clock::now();
                double time_none = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_none.push_back(time_none);
                std::cout << time_none << std::endl;
            }
            else {
                std::cout << "Skipping operation with no optimization" << std::endl;
                times_none.push_back(60*60);
                std::cout << 60*60 << " seconds" << std::endl;
            }

            // Cache optimization
            std::cout << "Running operation with cache optimization" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            C = multiplyDenseMatrices_cache(A, B, 64/sizeof(int));
            end = std::chrono::high_resolution_clock::now();
            double time_cache = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
            times_cache.push_back(time_cache);
            std::cout << time_cache << " seconds" << std::endl;

            // Multithreading optimization
            std::cout << "Running operation with multithreading optimization" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            C = multiplyDenseMatrices_multithread(A, B, 12);
            end = std::chrono::high_resolution_clock::now();
            double time_multithread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
            times_multithread.push_back(time_multithread);
            std::cout << time_multithread << " seconds" << std::endl;

            // SIMD optimization
            std::cout << "Running operation with SIMD optimization" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            C = multiplyDenseMatrices_SIMD(A, B);
            end = std::chrono::high_resolution_clock::now();
            double time_SIMD = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
            times_SIMD.push_back(time_SIMD);
            std::cout << time_SIMD << " seconds" << std::endl;

            // All optimizations
            std::cout << "Running operation with all optimizations" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            C = multiplyDenseMatrices_all(A, B, 64/sizeof(int), 12);
            end = std::chrono::high_resolution_clock::now();
            double time_all = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
            times_all.push_back(time_all);
            std::cout << time_all << " seconds" << std::endl;
        }
    }

    // Plot data
    std::string filename = "plot";
    std::ofstream data_file(filename + ".dat");
    data_file << "Size No-Optimization(s) Cache-Optimization(s) Multithread-Optimization(s) SIMD-Optimization(s) All-Optimization(s)\n";
    for (size_t i = 0; i < sizes.size(); ++i) {
        data_file << sizes[i] << " " << times_none[i] << " " << times_cache[i] << " " << times_multithread[i] << " " << times_SIMD[i] << " " << times_all[i] << "\n";
    }
    data_file.close();

    // Create gnuplot script
    std::ofstream gnuplot_file(filename + ".gp");
    gnuplot_file << "set title 'Matrix Multiplication Optimizations'\n";
    gnuplot_file << "set xlabel 'Row/Column Size'\n";
    gnuplot_file << "set ylabel 'Time(s)'\n";
    gnuplot_file << "set ytics\n";
    gnuplot_file << "set grid\n";
    gnuplot_file << "set key inside left\n";
    gnuplot_file << "set terminal pngcairo size 800,600\n";
    gnuplot_file << "set output '" << filename << ".png'\n";
    gnuplot_file << "plot '" << filename << ".dat' using 1:2 with lines title 'No Optimizations' lw 2 axis x1y1, \\\n";
    gnuplot_file << "     '" << filename << ".dat' using 1:3 with lines title 'Cache Optimization' lw 2 axis x1y1, \\\n";
    gnuplot_file << "     '" << filename << ".dat' using 1:4 with lines title 'Multithread Optimization' lw 2 axis x1y1, \\\n";
    gnuplot_file << "     '" << filename << ".dat' using 1:5 with lines title 'SIMD Optimization' lw 2 axis x1y1, \\\n";
    gnuplot_file << "     '" << filename << ".dat' using 1:6 with lines title 'All Optimizations' lw 2 axis x1y1\n";

    gnuplot_file.close();

    // Call gnuplot to generate the plot
    std::string command = "gnuplot " + filename + ".gp";
    system(command.c_str());

    return 0;
}
