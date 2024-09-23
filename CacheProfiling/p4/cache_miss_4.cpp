#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>

const int KB = 1024;
const int MB = 1024 * KB;
const int cache_line_size = 64; // Cache line size for all caches
const int cache_stride = cache_line_size / sizeof(int);
const long int CPU_Hz = 3500e6;

const size_t L1_cache_size = 4*32*KB;  // Measured L1 cache size = 6*32*KB
const size_t L2_cache_size = 4*256*KB; // Measured L2 cache size = 6*256*KB
const size_t L3_cache_size = 4*MB;     // Measured L3 cache size = 12*MB
const size_t main_memory_size = 16*MB; // Larger than 12*MB

// Function to test read latency
void test_read_latency(size_t array_size, int num_tests) {
    std::chrono::duration<double, std::nano> elapsed;

    // Allocate to appropriate cache using cache size limits
    size_t array_num = array_size / sizeof(int);

    for (int t = 0; t < num_tests; t++) {
        std::vector<int> arr(array_num);

        // Fill the array with random values
        for (int i = 0; i < array_num; i++) {
            arr[i] = 1;
        }

        if (array_size == L2_cache_size) {
            std::vector<int> arr1(L1_cache_size/sizeof(int));
            std::vector<int> arr2(L1_cache_size/sizeof(int));

            for (int i = 0; i < L1_cache_size/sizeof(int); i++) {
                arr1[i] = 1;
                arr2[i] = 1;
            }
        }

        else if (array_size == L3_cache_size) {
            std::vector<int> arr1(L1_cache_size/sizeof(int));
            std::vector<int> arr2(L1_cache_size/sizeof(int));
            std::vector<int> arr3(L2_cache_size/sizeof(int));
            for (int i = 0; i < L1_cache_size/sizeof(int); i++) {
                arr1[i] = 1;
                arr2[i] = 1;
            }
            for (int i = 0; i < L2_cache_size/sizeof(int); i++) {
                arr3[i] = 1;
            }
        }

        else if (array_size == main_memory_size) {
            std::vector<int> arr1(L1_cache_size/sizeof(int));
            std::vector<int> arr2(L1_cache_size/sizeof(int));
            std::vector<int> arr3(L2_cache_size/sizeof(int));
            std::vector<int> arr4(L3_cache_size/sizeof(int)*3);
            for (int i = 0; i < L1_cache_size/sizeof(int); i++) {
                arr1[i] = 1;
                arr2[i] = 1;
            }
            for (int i = 0; i < L2_cache_size/sizeof(int); i++) {
                arr3[i] = 1;
            }
            for (int i = 0; i < L3_cache_size/sizeof(int)*3; i++) {
                arr4[i] = 1;
            }
        }

        int temp = 0; // Temporary variable to ensure the read happens

        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < array_num; i += cache_stride) {
            temp += arr[i];  // Using stride to avoid prefetching
        }
        auto end = std::chrono::high_resolution_clock::now();

        elapsed += end - start;
    }
    
    double latency = elapsed.count() / (num_tests * array_num / cache_stride);
    double clock_cost = latency * 1e-9 * CPU_Hz;
    std::cout << "Average Read Latency: " <<  latency << " ns" << std::endl
              << "Average CPU Cycle Cost: " << clock_cost << " cycles" << std::endl;
}

int main() {
    int num_tests = 500;
    std::string flushLine = "---------------------------------------------------------";
    // Test with L1 cache miss
    std::cout << "L1 Cache Miss:" << std::endl << flushLine << std::endl;
    test_read_latency(L2_cache_size, num_tests);

    // Test with L2 cache miss
    std::cout << "\nL2 Cache Miss:" << std::endl << flushLine << std::endl;
    test_read_latency(L3_cache_size, num_tests);

    // Test with L3 cache miss
    std::cout << "\nL3 Cache Miss:" << std::endl << flushLine << std::endl;
    test_read_latency(main_memory_size, num_tests);

    return 0;
}
