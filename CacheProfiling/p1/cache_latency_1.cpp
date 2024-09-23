#include <iostream>
#include <chrono>
#include <vector>

const int KB = 1024;
const int MB = 1024 * KB;
const int cache_line_size = 64; // Cache line size for all caches
const int cache_stride = 8*cache_line_size / sizeof(int);

const size_t L1_cache_size = 4*32*KB;  // Measured L1 cache size = 6*32*KB
const size_t L2_cache_size = 4*256*KB; // Measured L2 cache size = 6*256*KB
const size_t L3_cache_size = 4*MB;     // Measured L3 cache size = 12*MB
const size_t main_memory_size = 12*MB;

// Function to test read latency
void test_read_latency_no_queue(size_t array_size, int num_tests) {
    std::chrono::duration<double, std::nano> elapsed;
    int arr_num = array_size / sizeof(int);
    for (int t = 0; t < num_tests; t++) {
        std::vector<int> arr(arr_num); // Allocate memory
        int temp = 0; // Temporary variable to ensure the read happens

        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < arr.size(); i += cache_stride) {
            temp += arr[i];  // Use stride to avoid prefetching
        }
        auto end = std::chrono::high_resolution_clock::now();

        elapsed += end - start;
    }
       
    std::cout << "Average read latency for array of size " << array_size / KB << " KB: " 
        << elapsed.count() / (num_tests * arr_num / cache_stride) << " ns" << std::endl;
}

// Function to test write latency
void test_write_latency_no_queue(size_t array_size, int num_tests) {
    std::chrono::duration<double, std::nano> elapsed;
    int arr_num = array_size / sizeof(int);
    for (int t = 0; t < num_tests; t++) {
        std::vector<int> arr(arr_num); // Allocate memory

        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < arr.size(); i += cache_stride) {
            arr[i] = i;  // Use stride to avoid prefetching
        }
        auto end = std::chrono::high_resolution_clock::now();

        elapsed += end - start;
    }

    std::cout << "Average write latency for array of size " << array_size / KB << " KB: " 
        << elapsed.count() / (num_tests * arr_num / cache_stride) << " ns" << std::endl;
}

int main() {
    int num_tests = 2000;
    std::string flushLine = "-------------------------------------------------------------";

    // Test with L1 cache size
    std::cout << "L1 Cache:" << std::endl <<  flushLine << std::endl;
    test_read_latency_no_queue(L1_cache_size, num_tests);
    test_write_latency_no_queue(L1_cache_size, num_tests);

    // Test with L2 cache size
    std::cout << "\nL2 Cache:" << std::endl << flushLine << std::endl;
    test_read_latency_no_queue(L2_cache_size, num_tests);
    test_write_latency_no_queue(L2_cache_size, num_tests);

    // Test with L3 cache size
    std::cout << "\nL3 Cache:" << std::endl << flushLine << std::endl;
    test_read_latency_no_queue(L3_cache_size, num_tests);
    test_write_latency_no_queue(L3_cache_size, num_tests);

    // Test with main memory size
    std::cout << "\nMain Memory:" << std::endl << flushLine << std::endl;;
    test_read_latency_no_queue(main_memory_size, num_tests);
    test_write_latency_no_queue(main_memory_size, num_tests);

    return 0;
}
