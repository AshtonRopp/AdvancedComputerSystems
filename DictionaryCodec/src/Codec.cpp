// Codec.cpp
#include "Codec.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <immintrin.h>
#include <thread>
#include <chrono>
#include <cstdlib>

DictionaryCodec::DictionaryCodec() {}

// Helper to load a column file into memory for processing
std::vector<std::string> DictionaryCodec::LoadColumnFile(const std::string& inputFile) const {
    std::cout << "Loading file." << std::endl;
    std::ifstream file(inputFile);
    std::vector<std::string> columnData;
    std::string line;
    while (std::getline(file, line)) {
        columnData.push_back(line);
    }

    file.close();
    return columnData;
}

// Helper to load an encoded file into memory for processing
void DictionaryCodec::LoadEncodedFile(const std::string& inputFile) {
    std::cout << "Loading file." << std::endl;
    std::ifstream file(inputFile);

    size_t ind = 0;
    size_t keyVal;
    std::string dataStr;

    file >> dataSize_; // Load first line, size of data

    dataColumn_ = std::make_unique<std::string[]>(dataSize_);

    while (file >> keyVal) {
        encodedColumn_.push_back(keyVal);
        file >> dataStr;
        dictionary_[dataStr] = keyVal;
        dataColumn_[ind] = dataStr;

        ind++;
    }

    file.close();
    std::cout << "Finished loading file" << std::endl;
}

// Helper to write the dictionary and encoded column to a file
bool DictionaryCodec::WriteEncodedColumnFile(const std::string& outputFile, const std::vector<std::string>& columnData) const {
    std::cout << "Writing file." << std::endl;

    std::ofstream file(outputFile);
    file << columnData.size() << "\n"; // First line is size of data
    if (!file.is_open()) return false;

    // Write dictionary --> format: key data
    for (size_t i = 0; i < encodedColumn_.size(); i++) {
        file << encodedColumn_[i] << "\n" << columnData[i] << "\n";
    }
    file.close();

    return true;
}

// Multi-threaded dictionary builder
void DictionaryCodec::BuildDictionary(const std::vector<std::string>& columnData, unsigned int numThreads) {
    std::cout << "Building dictionary." << std::endl;

    std::mutex localMutex;
    int code = 0;

    auto encodeChunk = [&](size_t start, size_t end) {
        std::unordered_map<std::string, int> localDictionary;
        for (size_t i = start; i < end; ++i) {
            const auto& item = columnData[i];
            if (localDictionary.find(item) == localDictionary.end()) {
                localDictionary[item] = code++;
            }
        }
        std::lock_guard<std::mutex> lock(localMutex);
        for (const auto& [key, value] : localDictionary) {
            dictionary_[key] = value;
        }
    };

    size_t chunkSize = columnData.size() / numThreads;
    std::vector<std::thread> threads;

    for (size_t i = 0; i < numThreads; ++i) {
        size_t start = i * chunkSize;
        size_t end = (i == numThreads - 1) ? columnData.size() : start + chunkSize;
        threads.emplace_back(encodeChunk, start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

// Encoding: Perform dictionary encoding on a column file
bool DictionaryCodec::EncodeColumnFile(const std::string& inputFile, const std::string& outputFile) {
    auto columnData = LoadColumnFile(inputFile);
    BuildDictionary(columnData);

    // Encode column using dictionary
    encodedColumn_.reserve(columnData.size());
    for (const auto& item : columnData) {
        encodedColumn_.push_back(dictionary_[item]);
    }

    return WriteEncodedColumnFile(outputFile, columnData);
}

// Test encoding speed based on number of threads and output graph
void DictionaryCodec::TestEncodingSpeed(const std::string& inputFile) {
    auto columnData = LoadColumnFile(inputFile);

    size_t numThreads = std::thread::hardware_concurrency();
    std::cout << "Max number of threads: " << numThreads << std::endl;
    
    std::vector<size_t> x_data;          // To store the number of threads
    std::vector<double> y_data;          // To store average time durations
    int numRuns = 10;

    for (size_t t = 1; t < numThreads; ++t) {
        double totalTime = 0.0;

        for (int i = 0; i < numRuns; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            BuildDictionary(columnData, t);
            auto end = std::chrono::high_resolution_clock::now();
            dictionary_.clear();

            // Calculate the duration in milliseconds
            std::chrono::duration<double, std::milli> duration = end - start;
            totalTime += duration.count();
        }

        double averageTime = totalTime / numRuns;
        x_data.push_back(t);
        y_data.push_back(averageTime);
    }

    // Save data to a file for gnuplot
    std::ofstream dataFile("timing_data.dat");
    for (size_t i = 0; i < x_data.size(); ++i) {
        dataFile << x_data[i] << " " << y_data[i] << "\n";
    }
    dataFile.close();

    std::cout << "Timing data saved to timing_data.dat for gnuplot.\n";
    // Execute the gnuplot script
    system("gnuplot -persist src/plot.gp");
}

// Query: Check if a data item exists in the encoded column, return indices if found
std::vector<size_t> DictionaryCodec::QueryItem(const std::string& dataItem) {
    std::vector<size_t> results;

    std::shared_lock lock(dictionaryMutex_);

    auto it = dictionary_.find(dataItem);
    if (it == dictionary_.end()) {
        return results;
    }

    size_t maxI = encodedColumn_.size();
    size_t key = it->second;
    for (size_t i = 0; i < maxI; i++) {
        if (encodedColumn_[i] == key) {
            results.push_back(i);
        }
    }
    return results;
}

std::vector<size_t> DictionaryCodec::SIMDQueryItem(const std::string& dataItem) {
    std::vector<size_t> results;

    std::shared_lock lock(dictionaryMutex_);

    // Find the dictionary entry for `dataItem`
    auto it = dictionary_.find(dataItem);
    if (it == dictionary_.end()) {
        return results;
    }

    size_t maxI = encodedColumn_.size();
    size_t key = it->second;

    // SIMD register for the key (assuming size_t is 64-bit)
    __m256i keyVec = _mm256_set1_epi64x(static_cast<long long>(key));

    // Process 4 elements at a time with AVX2
    size_t i = 0;
    for (; i + 3 < maxI; i += 4) {
        // Load 4 elements from encodedColumn_ into a SIMD register
        __m256i columnVec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&encodedColumn_[i]));

        // Compare each element in the chunk to `keyVec`
        __m256i cmpResult = _mm256_cmpeq_epi64(columnVec, keyVec);

        // Get a mask of matching elements
        int mask = _mm256_movemask_epi8(cmpResult);

        // For each matching bit in the mask, add the index to results
        for (int j = 0; j < 4; ++j) {
            if ((mask >> (j * 8)) & 0xFF) {  // Each 64-bit match uses 8 bits in the mask
                results.push_back(i + j);
            }
        }
    }

    // Handle remaining elements (if any) in a scalar loop
    for (; i < maxI; ++i) {
        if (encodedColumn_[i] == key) {
            results.push_back(i);
        }
    }

    return results;
}

// Dictionary-assisted prefix search
std::vector<size_t> DictionaryCodec::SearchByPrefix(const std::string& prefix) const {
    std::vector<size_t> results;
    size_t prefixLen = prefix.size();

    // Lock reading mutex
    std::shared_lock lock(dictionaryMutex_);

    for (const auto& [key, code] : dictionary_) {
        if (key.compare(0, prefixLen, prefix) == 0) {

            size_t maxI = encodedColumn_.size();
            for (size_t i = 0; i < maxI; i++) {
                if (encodedColumn_[i] == code) {
                    results.push_back(i);
                }
            }
        }
    }
    return results;
}

// Query by prefix without SIMD
std::vector<size_t> DictionaryCodec::QueryByPrefix(const std::string& prefix) const {
    return SearchByPrefix(prefix);
}

// SIMD-assisted prefix search
std::vector<size_t> DictionaryCodec::SIMDQueryByPrefix(const std::string& prefix) const {
    std::vector<size_t> results;
    size_t prefixLen = prefix.size();

    // Lock reading mutex
    std::shared_lock lock(dictionaryMutex_);

    // Prepare SIMD register for prefix (up to 32 characters for AVX2)
    char paddedPrefix[32] = {0};  // Zero-padding for shorter prefixes
    std::memcpy(paddedPrefix, prefix.data(), prefixLen);
    __m256i prefixVec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(paddedPrefix));

    for (const auto& [key, code] : dictionary_) {
        if (key.size() >= prefixLen) {
            // Load the first 32 bytes of the key
            __m256i keyVec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(key.data()));

            // Compare prefix with the beginning of the key
            __m256i cmpResult = _mm256_cmpeq_epi8(prefixVec, keyVec);
            int mask = _mm256_movemask_epi8(cmpResult);

            // Check if the first `prefixLen` bytes match
            if ((mask & ((1 << prefixLen) - 1)) == ((1 << prefixLen) - 1)) {
                size_t maxI = encodedColumn_.size();

                // Prepare SIMD register for code comparison
                __m256i codeVec = _mm256_set1_epi64x(static_cast<long long>(code));

                // Process encodedColumn_ in chunks of 4 (for 64-bit integers)
                size_t i = 0;
                for (; i + 3 < maxI; i += 4) {
                    // Load 4 elements from encodedColumn_ into a SIMD register
                    __m256i columnVec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&encodedColumn_[i]));

                    // Compare each element in the chunk to `codeVec`
                    __m256i cmpResultCode = _mm256_cmpeq_epi64(columnVec, codeVec);

                    // Get a mask of matching elements
                    int maskCode = _mm256_movemask_epi8(cmpResultCode);

                    // For each matching bit in the mask, add the index to results
                    for (int j = 0; j < 4; ++j) {
                        if ((maskCode >> (j * 8)) & 0xFF) {  // Each 64-bit match uses 8 bits in the mask
                            results.push_back(i + j);
                        }
                    }
                }

                // Handle remaining elements (if any) in a scalar loop
                for (; i < maxI; ++i) {
                    if (encodedColumn_[i] == code) {
                        results.push_back(i);
                    }
                }
            }
        }
    }
    return results;
}

// Baseline column search (without dictionary encoding) for performance comparison
std::vector<size_t> DictionaryCodec::BaselineSearch(const std::string& dataItem) {
    std::vector<size_t> indices;

    size_t len = GetDataSize();
    for (size_t i = 0; i < len; ++i) {
        if (dataColumn_[i] == dataItem) {
            indices.push_back(i);
        }
    }
    return indices;
}

// Baseline column search (without dictionary encoding) for performance comparison
std::vector<size_t> DictionaryCodec::BaselinePrefixSearch(const std::string& prefix) {
    std::vector<size_t> indices;
    size_t prefixLen = prefix.size();

    size_t len = GetDataSize();
    for (size_t i = 0; i < len; ++i) {
        if (dataColumn_[i].compare(0, prefixLen, prefix) == 0) {
            indices.push_back(i);
        }
    }
    return indices;
}