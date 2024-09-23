#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <thread>
#include <fstream>
#include <cstdlib>

const size_t BUFFER_SIZE = 1024 * 1024 * 1024; // 1 GB buffer
const size_t BUFFER_LEN  = BUFFER_SIZE/sizeof(int);
const int NUM_ITERATIONS = 30;
const int NUM_THREADS = 12;

// Function to measure bandwidth for a single thread
void measure_bandwidth_thread(size_t access_size, double read_ratio, size_t start, size_t end, std::chrono::duration<double>& thread_time) {
    std::vector<int> buffer(end - start);
    
    // Initialize buffer with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(50, 150);
    std::generate(buffer.begin(), buffer.end(), [&]() { return dis(gen); });

    size_t num_accesses = access_size/sizeof(int);
    size_t max_accesses = end - start - num_accesses;
    size_t readInd = read_ratio * buffer.size();

    // Begin timing
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {      
        for (size_t sec = 0; sec < max_accesses; sec += num_accesses) {
            // Access input bytes of data
            for (size_t i = 0; i < num_accesses; i++) {
                size_t index = i + sec;
                // Perform read
                if(index < readInd) {
                    volatile int temp = buffer[index];
                }
                // Perform arbitrary write
                else { 
                    buffer[index] = 1;
                }
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    thread_time = end_time - start_time;
}

// Function to run bandwidth test using multiple threads
double run_bandwidth_test(size_t access_size, double read_ratio) {
    std::vector<std::thread> threads;
    std::vector<std::chrono::duration<double>> thread_times(NUM_THREADS);

    size_t chunk_size = BUFFER_LEN / NUM_THREADS;

    // Create and start threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == NUM_THREADS - 1) ? BUFFER_LEN : (i + 1) * chunk_size;
        threads.emplace_back(measure_bandwidth_thread, access_size, read_ratio, start, end, std::ref(thread_times[i]));
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Calculate average time
    auto total_time = std::accumulate(thread_times.begin(), thread_times.end(), std::chrono::duration<double>(0));
    auto average_time = total_time / NUM_THREADS;

    // Calculate bandwidth in GB/s
    double total_data = static_cast<double>(BUFFER_LEN) * NUM_ITERATIONS * sizeof(int) / (1024 * 1024 * 1024);
    return total_data / average_time.count();
}

int main() {
    // Access sizes in bytes
    std::vector<size_t> access_sizes = {64, 256, 512, 1024, 2048};
    // Read ratios
    std::vector<double> read_ratios = {0.0, 0.5, 0.75, 1.0};

    // Output data file for gnuplot
    std::ofstream data_file("bandwidth_data.dat");

    if (!data_file.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return 1;
    }

    // Write data in a format that gnuplot can easily use
    for (size_t access_size : access_sizes) {
        data_file << "# Access size: " << access_size << " bytes" << std::endl;
        for (double read_ratio : read_ratios) {
            double bandwidth = run_bandwidth_test(access_size, read_ratio);
            // Write read_ratio and bandwidth for this access size
            data_file << read_ratio << " " << bandwidth << std::endl;
        }
        data_file << "\n\n";  // Ensure double blank lines between datasets
    }
    
    data_file.close();

    // Generate the gnuplot script
    std::ofstream gnuplot_script("plot_bandwidth.gnuplot");
    if (!gnuplot_script.is_open()) {
        std::cerr << "Failed to open gnuplot script for writing." << std::endl;
        return 1;
    }

    // Write the gnuplot commands
    gnuplot_script << "set title 'Bandwidth vs. Read Ratio'\n";
    gnuplot_script << "set xlabel 'Read Ratio'\n";
    gnuplot_script << "set ylabel 'Bandwidth (GB/s)'\n";
    gnuplot_script << "set key outside right top\n";  // Display the legend outside the graph
    gnuplot_script << "set grid\n";
    gnuplot_script << "set style line 1 lt 1 lc rgb '#FF0000' lw 2\n";  // Red line
    gnuplot_script << "set style line 2 lt 1 lc rgb '#00FF00' lw 2\n";  // Green line
    gnuplot_script << "set style line 3 lt 1 lc rgb '#0000FF' lw 2\n";  // Blue line
    gnuplot_script << "set style line 4 lt 1 lc rgb '#FF00FF' lw 2\n";  // Magenta line
    gnuplot_script << "set style line 5 lt 1 lc rgb '#00FFFF' lw 2\n";  // Cyan line
    gnuplot_script << "plot \\\n";

    // Plot each access size with a different line and color (style lines)
    for (size_t i = 0; i < access_sizes.size(); ++i) {
        gnuplot_script << "'bandwidth_data.dat' index " << i 
                       << " using 1:2 with lines linestyle " << (i + 1) 
                       << " title 'Access size: " << access_sizes[i] << " bytes'";
        if (i < access_sizes.size() - 1) {
            gnuplot_script << ", \\\n";
        }
    }
    gnuplot_script << "\n";

    gnuplot_script.close();

    // Call gnuplot to generate the plot
    system("gnuplot -p plot_bandwidth.gnuplot");

    return 0;
}
