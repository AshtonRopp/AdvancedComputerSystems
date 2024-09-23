#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <random>

const int KB = 1024;
const int MB = 1024 * KB;

// Function to simulate a memory read operation
void memory_read(int* arr, size_t arr_size, int num_accesses, std::atomic<int>& completed_accesses) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, arr_size - 1);

    for (int i = 0; i < num_accesses; ++i) {
        volatile int temp = arr[dis(gen)];  // Simulate random memory read
        completed_accesses++;
    }
}

// Function to simulate a memory write operation
void memory_write(int* arr, size_t arr_size, int num_accesses, std::atomic<int>& completed_accesses) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, arr_size - 1);

    for (int i = 0; i < num_accesses; ++i) {
        arr[dis(gen)] = i;  // Simulate random memory write
        completed_accesses++;
    }
}

// Measure latency and throughput for memory operations
void measure_latency_throughput(int* arr, size_t arr_size, int num_threads, int num_accesses_per_thread, bool is_write,
                                std::vector<double>& latencies, std::vector<double>& throughputs) {
    std::vector<std::thread> threads;
    std::atomic<int> completed_accesses(0);

    std::vector<int> indices(arr_size);
    for (int i = 0; i < arr_size; ++i) {
        indices[i] = i;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    auto start = std::chrono::high_resolution_clock::now();
    
    // Launch threads to simulate concurrent memory access
    for (int t = 0; t < num_threads; ++t) {
        if (is_write) {
            threads.emplace_back(memory_write, arr, arr_size, num_accesses_per_thread, std::ref(completed_accesses));
        } else {
            threads.emplace_back(memory_read, arr, arr_size, num_accesses_per_thread, std::ref(completed_accesses));
        }
    }

    // Wait for all threads to finish
    for (auto& th : threads) {
        th.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    double total_time = elapsed.count();
    double throughput = completed_accesses / total_time;  // Accesses per second
    double latency = total_time / completed_accesses;     // Time per access

    // Store results for graphing
    latencies.push_back(latency * 1e6);  // Convert latency to microseconds
    throughputs.push_back(throughput);   // Throughput in accesses per second

    // Print results
    std::cout << "Number of Threads: " << num_threads << "\n";
    std::cout << "Total Accesses: " << completed_accesses << "\n";
    std::cout << "Total Time: " << total_time << " seconds\n";
    std::cout << "Throughput: " << throughput << " accesses/second\n";
    std::cout << "Average Latency: " << latency * 1e6 << " microseconds/access\n";
    std::cout << "-------------------------------------------\n";
}

// Function to plot data using gnuplot
void plot_data(const std::vector<double>& thread_counts, const std::vector<double>& latencies, const std::vector<double>& throughputs, const std::string& filename) {
    // Write data to file
    std::ofstream data_file(filename + ".dat");
    data_file << "#Threads Latency(µs) Throughput(accesses/sec)\n";
    for (size_t i = 0; i < thread_counts.size(); ++i) {
        data_file << thread_counts[i] << " " << latencies[i] << " " << throughputs[i] << "\n";
    }
    data_file.close();

    // Create gnuplot script
    std::ofstream gnuplot_file(filename + ".gp");
    gnuplot_file << "set title 'Latency and Throughput vs. Number of Threads'\n";
    gnuplot_file << "set xlabel 'Number of Threads'\n";
    gnuplot_file << "set ylabel 'Latency (µs)'\n";
    gnuplot_file << "set y2label 'Throughput (accesses/sec)'\n";
    gnuplot_file << "set ytics nomirror\n";
    gnuplot_file << "set y2tics\n";
    gnuplot_file << "set grid\n";
    gnuplot_file << "set terminal pngcairo size 800,600\n";
    gnuplot_file << "set output '" << filename << ".png'\n";
    gnuplot_file << "plot '" << filename << ".dat' using 1:2 with lines title 'Latency' lw 2 axis x1y1, \\\n";
    gnuplot_file << "     '" << filename << ".dat' using 1:3 with lines title 'Throughput' lw 2 axis x1y2\n";
    gnuplot_file.close();

    // Call gnuplot to generate the plot
    std::string command = "gnuplot " + filename + ".gp";
    system(command.c_str());
}

int main() {
    // Allocate memory (simulate main memory)
    size_t memory_size = 80 * MB;  // 40 MB of memory
    size_t arr_size = memory_size / sizeof(int);
    int* arr = new int[arr_size];

    // Number of accesses each thread will perform
    int num_accesses_per_thread = 1e6;

    // Variables to store latency and throughput data for graphing
    std::vector<double> thread_counts;
    std::vector<double> latencies_read, throughputs_read;
    std::vector<double> latencies_write, throughputs_write;

    std::cout << "Read Latency and Throughput Experiment:\n";
    std::cout << "======================================\n";
    
    // Test with different numbers of threads for reads
    for (int num_threads = 1; num_threads <= 12; num_threads += 1) {
        thread_counts.push_back(num_threads);
        measure_latency_throughput(arr, arr_size, num_threads, num_accesses_per_thread, false, latencies_read, throughputs_read);
    }

    std::cout << "\nWrite Latency and Throughput Experiment:\n";
    std::cout << "=======================================\n";

    // Test with different numbers of threads for writes
    for (int num_threads = 1; num_threads <= 12; num_threads += 1) {
        measure_latency_throughput(arr, arr_size, num_threads, num_accesses_per_thread, true, latencies_write, throughputs_write);
    }

    // Plot read results
    plot_data(thread_counts, latencies_read, throughputs_read, "read_latency_throughput");

    // Plot write results
    plot_data(thread_counts, latencies_write, throughputs_write, "write_latency_throughput");

    delete[] arr;  // Free allocated memory
    return 0;
}
