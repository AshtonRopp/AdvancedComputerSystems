// Local headers
#include <SparseMatrix.h>
#include <dense-dense.h>
#include <dense-sparse.h>
#include <sparse-sparse.h>

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

// Function to create a random sparse matrix
SparseMatrix createSparseMatrix(int rows, int cols, double sparsity) {

    SparseMatrix matrix;
    matrix.rows.resize(rows);
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

// Used by all calls to generate corresponding plot
void create_gnuplot(const std::string& filename, const std::string& title, const std::vector<size_t>& sizes,
    const std::vector<double>& times_none, const std::vector<double>& times_cache,
    const std::vector<double>& times_multithread, const std::vector<double>& times_SIMD,
    const std::vector<double>& times_all) {

    // Plot data
    std::ofstream data_file(filename + ".dat");
    data_file << "Size No-Optimization(s) Cache-Optimization(s) Multithread-Optimization(s) SIMD-Optimization(s) All-Optimization(s)\n";
    for (size_t i = 0; i < sizes.size(); ++i) {
        data_file << sizes[i] << " " << times_none[i] << " " << times_cache[i] << " " << times_multithread[i] << " " << times_SIMD[i] << " " << times_all[i] << "\n";
    }
    data_file.close();

    // Create gnuplot script
    std::ofstream gnuplot_file(filename + ".gp");
    gnuplot_file << "set title '" << title << "'\n";
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
    system(("rm " + filename + ".gp").c_str());
    system(("rm " + filename + ".dat").c_str());
}

int main(int argc, char* argv[]) {

    // Seed for randomness
    std::srand(static_cast<unsigned int>(std::time(0)));

    // Timing variables
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();

    // Define test cases
    std::vector<size_t> sizes = {1200, 1500, 1800};
    std::vector<double> sparsityVals = {0.001, 0.01, 0.1};

    // Allow configurable number of threads
    int num_threads = 12;
    if(argc > 2) {
        num_threads = std::stoi(argv[2]); // Convert from str to int
        std::cout << "Running with " << num_threads << " threads." << std::endl;
    }

    // sparse-sparse
    if (strcmp(argv[1], "spsp") == 0) {
        sizes = {3000, 4000, 5000};
        SparseMatrix C; // Init result
        for (size_t j = 0; j < sparsityVals.size(); ++j) {
            // Timing vectors for gnuplot
            std::vector<double> times_none, times_cache, times_multithread, times_SIMD, times_all;
            for (size_t i = 0; i < sizes.size(); ++i) {
                size_t size = sizes[i];
                double sparsity = sparsityVals[j];
                std::cout << "Multiplying sparse-sparse matrices of size " << size << "x" << size
                    << " and sparsity " << sparsity*100 << "%:" << std::endl;

                SparseMatrix A = createSparseMatrix(size, size, sparsity);
                SparseMatrix B = createSparseMatrix(size, size, sparsity);

                // No optimization
                std::cout << "Running operation with no optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplySparseMatrices_none(A, B, size, size);
                end = std::chrono::high_resolution_clock::now();
                double time_none = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_none.push_back(time_none);
                std::cout << time_none << " seconds" << std::endl;

                // Cache optimization
                std::cout << "Running operation with cache optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplySparseMatrices_cache(A, B, size, size);
                end = std::chrono::high_resolution_clock::now();
                double time_cache = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_cache.push_back(time_cache);
                std::cout << time_cache << " seconds" << std::endl;

                // Multithreading optimization
                std::cout << "Running operation with multithreading optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplySparseMatrices_multithread(A, B, size, size, num_threads);
                end = std::chrono::high_resolution_clock::now();
                double time_multithread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_multithread.push_back(time_multithread);
                std::cout << time_multithread << " seconds" << std::endl;

                // SIMD optimization
                std::cout << "Running operation with SIMD optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplySparseMatrices_SIMD(A, B, size, size);
                end = std::chrono::high_resolution_clock::now();
                double time_SIMD = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_SIMD.push_back(time_SIMD);
                std::cout << time_SIMD << " seconds" << std::endl;

                // All optimizations
                std::cout << "Running operation with all optimizations" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplySparseMatrices_all(A, B, size, size, num_threads);
                end = std::chrono::high_resolution_clock::now();
                double time_all = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_all.push_back(time_all);
                std::cout << time_all << " seconds" << std::endl << std::endl;

                // Create gnuplot for every sparsity
                std::string percent = std::to_string(sparsity*100).substr(0, 3);
                create_gnuplot("sparse-sparse"+percent, "Sparse-Sparse Multiplication: " + percent + "%", sizes, times_none, times_cache, times_multithread, times_SIMD, times_all);
            }
        }
    }

    // dense-sparse
    else if (strcmp(argv[1], "dsp") == 0) {
        std::vector<std::vector<int>> C; // Init result
        for (size_t j = 0; j < sparsityVals.size(); ++j) {
            // Timing vectors for gnuplot
            std::vector<double> times_none, times_cache, times_multithread, times_SIMD, times_all;
            for (size_t i = 0; i < sizes.size(); ++i) {

                size_t size = sizes[i];
                double sparsity = sparsityVals[j];
                std::cout << "Multiplying dense-sparse matrices of size " << size << "x" << size
                    << " and sparsity " << sparsity*100 << "%:" << std::endl;

                std::vector<std::vector<int>> A = createDenseMatrix(size, size);
                SparseMatrix B = createSparseMatrix(size, size, sparsity);

                // No optimization
                std::cout << "Running operation with no optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplyDenseSparseMatrices_none(A, B);
                end = std::chrono::high_resolution_clock::now();
                double time_none = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_none.push_back(time_none);
                std::cout << time_none << " seconds" << std::endl;

                // Cache optimization
                std::cout << "Running operation with cache optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplyDenseSparseMatrices_cache(A, B, 64/sizeof(int));
                end = std::chrono::high_resolution_clock::now();
                double time_cache = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_cache.push_back(time_cache);
                std::cout << time_cache << " seconds" << std::endl;

                // Multithreading optimization
                std::cout << "Running operation with multithreading optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplyDenseSparseMatrices_multithread(A, B, num_threads);
                end = std::chrono::high_resolution_clock::now();
                double time_multithread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_multithread.push_back(time_multithread);
                std::cout << time_multithread << " seconds" << std::endl;

                // SIMD optimization
                std::cout << "Running operation with SIMD optimization" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplyDenseSparseMatrices_SIMD(A, B);
                end = std::chrono::high_resolution_clock::now();
                double time_SIMD = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_SIMD.push_back(time_SIMD);
                std::cout << time_SIMD << " seconds" << std::endl;

                // All optimizations
                std::cout << "Running operation with all optimizations" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                C = multiplyDenseSparseMatrices_all(A, B, 64/sizeof(int), num_threads);
                end = std::chrono::high_resolution_clock::now();
                double time_all = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
                times_all.push_back(time_all);
                std::cout << time_all << " seconds" << std::endl << std::endl;

                // Create gnuplot for every sparsity
                std::string percent = std::to_string(sparsity*100).substr(0, 3);
                create_gnuplot("dense-sparse"+percent, "Dense-Sparse Multiplication: " + percent + "%", sizes, times_none, times_cache, times_multithread, times_SIMD, times_all);
            }
        }
    }

    // dense-dense
    else {
        // Timing vectors for gnuplot
        std::vector<double> times_none, times_cache, times_multithread, times_SIMD, times_all;
        for (size_t i = 0; i < sizes.size(); ++i) {
            size_t size = sizes[i];
            std::vector<std::vector<int>> A = createDenseMatrix(size, size);
            std::vector<std::vector<int>> B = createDenseMatrix(size, size);
            std::vector<std::vector<int>> C;

            std::cout << "Multiplying dense-dense matrices of size " << size << "x" << size << ":" << std::endl;

            // No optimization
            std::cout << "Running operation with no optimization" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            C = multiplyDenseMatrices_none(A, B);
            end = std::chrono::high_resolution_clock::now();
            double time_none = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
            times_none.push_back(time_none);
            std::cout << time_none << std::endl;

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
            C = multiplyDenseMatrices_multithread(A, B, num_threads);
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
            C = multiplyDenseMatrices_all(A, B, 64/sizeof(int), num_threads);
            end = std::chrono::high_resolution_clock::now();
            double time_all = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
            times_all.push_back(time_all);
            std::cout << time_all << " seconds" << std::endl;
        }

        // Create single plot (no individual plots for sparsity)
        create_gnuplot("dense-dense", "Dense-Dense Matrix Multiplication", sizes, times_none, times_cache, times_multithread, times_SIMD, times_all);
    }

    return 0;
}
