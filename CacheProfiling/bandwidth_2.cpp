#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <numeric>

const size_t BUFFER_SIZE = 1024 * 1024 * 1024; // 1 GB buffer
const size_t BUFFER_LEN  = BUFFER_SIZE/sizeof(int);
const int NUM_ITERATIONS = 10;

// Function to measure bandwidth
double measure_bandwidth(size_t access_size, double read_ratio) {
   std::vector<int> buffer(BUFFER_LEN);
    
   // Initialize buffer with random data
   std::random_device rd;
   std::mt19937 gen(rd());

   // Define the range of random numbers (e.g., 50 to 150)
   std::uniform_int_distribution<> dis(50, 150);

   // Fill the vector with random numbers
   std::generate(buffer.begin(), buffer.end(), [&]() { return dis(gen); });

   // Use the code for read/write
   // Divy up from BUFFER_SIZE 6 ways --> start threading


   // // Shuffle matrix to promote random, non-uniform access
   // std::vector<size_t> access_indices(BUFFER_SIZE);
   // std::iota(access_indices.begin(), access_indices.end(), 0);
   // std::shuffle(access_indices.begin(), access_indices.end(), gen);

   size_t num_accesses = access_size*8/sizeof(int);
   size_t max_accesses_min_BUFF_LEN = BUFFER_LEN-num_accesses;
   size_t readInd = read_ratio*BUFFER_LEN;

   // Begin timing
   auto start = std::chrono::high_resolution_clock::now();

   for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {      
      for (size_t sec = 0; sec < max_accesses_min_BUFF_LEN; sec += num_accesses) {
         // Access input bytes of data
         for (size_t i = 0; i < num_accesses; i++) {
            size_t index = i + sec;
            // Perform read
            if(index < readInd) {
               int temp =  buffer[index];
            }
            // Perform arbitrary write
            else { 
               buffer[index] = 500;
            }
         }
      }
   }
   // for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {      
   //    for (size_t sec = 0; sec < max_accesses_min_BUFF_LEN; sec += num_accesses) {
   //       // Access input bytes of data
   //       for (size_t i = 0; i < num_accesses; i++) {
   //          size_t index = i + sec;
   //          // Perform read
   //          if(index < readInd) {
   //             int temp =  buffer[index];
   //          }
   //          // Perform arbitrary write
   //          else { 
   //             buffer[index] = 500;
   //          }
   //       }
   //    }
   // }

   auto end = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> diff = end - start;

   // Calculate bandwidth in GB/s
   double total_data = static_cast<double>(BUFFER_LEN) * NUM_ITERATIONS * sizeof(int) / 8 / (1024 * 1024 * 1024);
   return total_data / diff.count();
}

int main() {
   // Access size in bytes
   std::vector<size_t> access_sizes = {64, 256, 1024};
   std::vector<double> read_ratios = {1.0, 0.0, 0.7, 0.5};

   for (size_t access_size : access_sizes) {
      std::cout << "Access size: " << access_size << " bytes" << std::endl;
      for (double read_ratio : read_ratios) {
         double bandwidth = measure_bandwidth(access_size, read_ratio);
         std::cout << "Read ratio: " << read_ratio * 100 << "%, Bandwidth: " << bandwidth << " GB/s" << std::endl;
      }
      std::cout << std::endl;
   }

   return 0;
}