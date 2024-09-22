#include <iostream>
#include <chrono>
#include <vector>

const int KB = 1024;
const int MB = 1024 * KB;
// const int cache_line_size = 64; // Cache line size for all caches

// Function to test read latency
void test_read_latency_no_queue(size_t array_size, int num_tests) {
    std::chrono::duration<double, std::nano> elapsed;
    int arr_num = array_size / sizeof(int);
    for (int t = 0; t < num_tests; t++) {
        std::vector<int> arr(arr_num); // Allocate memory
        int temp = 0; // Temporary variable to ensure the read happens

        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < arr.size(); i += 64) {
            temp += arr[i];  // Accessing every 64th element to avoid cache prefetching
        }
        auto end = std::chrono::high_resolution_clock::now();

        elapsed += end - start;
    }
       
    std::cout << "Average read latency for array of size " << array_size / KB << " KB: " 
        << elapsed.count() / (num_tests * arr_num / 64) << " ns" << std::endl;
}

// Function to test write latency
void test_write_latency_no_queue(size_t array_size, int num_tests) {
    std::chrono::duration<double, std::nano> elapsed;
    int arr_num = array_size / sizeof(int);
    for (int t = 0; t < num_tests; t++) {
        std::vector<int> arr(arr_num); // Allocate memory

        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < arr.size(); i += 64) {
            arr[i] = i;  // Write to every 64th element to avoid cache prefetching
        }
        auto end = std::chrono::high_resolution_clock::now();

        elapsed += end - start;
    }

    std::cout << "Average write latency for array of size " << array_size / KB << " KB: " 
        << elapsed.count() / (num_tests * arr_num / 64) << " ns" << std::endl;
}

int main() {
    size_t L1_cache_size = 6*32*KB;  // Measured L1 cache size
    size_t L2_cache_size = 6*256*KB; // Measured L2 cache size
    size_t L3_cache_size = 12*MB;    // Measured L3 cache size
    size_t main_memory_size = 20*MB; // Larger than cache (forcing main memory access)
    int num_tests = 20;
    std::string flushLine = "---------------------------------------------------------";

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
