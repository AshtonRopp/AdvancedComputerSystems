#include <iostream>
#include <chrono>
#include <vector>

const int KB = 1024;
const int MB = 1024 * KB;

// Function to test read latency
void test_read_latency(size_t array_size) {
    std::vector<int> arr(array_size / sizeof(int)); // Allocate memory
    int temp = 0; // Temporary variable to ensure the read happens

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < arr.size(); i += 16) {
        temp += arr[i];  // Accessing every 16th element to avoid cache prefetching
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::nano> elapsed = end - start;
    std::cout << "Read latency for array of size " << array_size / KB << " KB: " 
              << elapsed.count() / (arr.size() / 16) << " ns" << std::endl;
}

// Function to test write latency
void test_write_latency(size_t array_size) {
    std::vector<int> arr(array_size / sizeof(int)); // Allocate memory

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < arr.size(); i += 16) {
        arr[i] = i;  // Write to every 16th element to avoid cache prefetching
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::nano> elapsed = end - start;
    std::cout << "Write latency for array of size " << array_size / KB << " KB: " 
              << elapsed.count() / (arr.size() / 16) << " ns" << std::endl;
}

int main() {
    size_t L1_cache_size = 6*32 * KB;  // Typical L1 cache size
    size_t L2_cache_size = 6*256 * KB; // Typical L2 cache size
    size_t main_memory_size = 12 * MB; // Larger than cache (forcing main memory access)

    // Test read and write latency for different memory sizes
    std::cout << "Testing Cache and Main Memory Latency" << std::endl;

    // Test with L1 cache size
    test_read_latency(L1_cache_size);
    test_write_latency(L1_cache_size);

    // Test with L2 cache size
    test_read_latency(L2_cache_size);
    test_write_latency(L2_cache_size);

    // Test with main memory size
    test_read_latency(main_memory_size);
    test_write_latency(main_memory_size);

    return 0;
}
