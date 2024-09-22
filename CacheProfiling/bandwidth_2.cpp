#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>

const size_t BUFFER_SIZE = 1024 * 1024 * 1024; // 1 GB buffer
const size_t BUFFER_LEN  = BUFFER_SIZE/sizeof(int);
const int NUM_ITERATIONS = 10;
const int NUM_THREADS = 6;

std::mutex cout_mutex;

// Function to measure bandwidth for a single thread
void measure_bandwidth_thread(size_t access_size, double read_ratio, size_t start, size_t end, std::chrono::duration<double>& thread_time) {
    std::vector<int> buffer(end - start);
    
    // Initialize buffer with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(50, 150);
    std::generate(buffer.begin(), buffer.end(), [&]() { return dis(gen); });

    size_t num_accesses = access_size*8/sizeof(int);
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
                    buffer[index] = 500;
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
    double total_data = static_cast<double>(BUFFER_LEN) * NUM_ITERATIONS * sizeof(int) / 8 / (1024 * 1024 * 1024);
    return total_data / average_time.count();
}

int main() {
    // Access size in bytes
    std::vector<size_t> access_sizes = {64, 256, 1024, 1024*8};
    std::vector<double> read_ratios = {1.0, 0.7, 0.5, 0.0};

    for (size_t access_size : access_sizes) {
        std::cout << "Access size: " << access_size << " bytes" << std::endl;
        for (double read_ratio : read_ratios) {
            double bandwidth = run_bandwidth_test(access_size, read_ratio);
            std::cout << "Read ratio: " << read_ratio * 100 << "%, Average Bandwidth: " << bandwidth << " GB/s" << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}