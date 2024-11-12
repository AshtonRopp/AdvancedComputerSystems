// Codec.cpp
#include "Codec.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <thread>
#include <immintrin.h>  // For SIMD
#include <future>

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
        keyIndeces_[keyVal].push_back(ind);
        file >> dataStr;
        dictionary_[dataStr] = keyVal;
        dataColumn_[ind] = dataStr;

        ind++;
    }

    file.close();
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
void DictionaryCodec::BuildDictionary(const std::vector<std::string>& columnData) {
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

    size_t numThreads = std::thread::hardware_concurrency();
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

// Query: Check if a data item exists in the encoded column, return indices if found
std::vector<size_t> DictionaryCodec::QueryItem(const std::string& dataItem) {
    std::vector<size_t> indices;

    std::shared_lock lock(dictionaryMutex_);

    auto it = dictionary_.find(dataItem);
    if (it == dictionary_.end()) return indices;

    auto it2 = keyIndeces_.find(it->second);
    if (it2 == keyIndeces_.end()) return indices;

    return keyIndeces_[dictionary_[dataItem]];
}

// Dictionary-assisted prefix search
std::vector<size_t> DictionaryCodec::SearchByPrefix(const std::string& prefix) const {
    std::vector<size_t> results;
    size_t prefixLen = prefix.size();

    // Lock reading mutex
    std::shared_lock lock(dictionaryMutex_);

    for (const auto& [key, code] : dictionary_) {
        if (key.compare(0, prefixLen, prefix) == 0) {

            auto it = keyIndeces_.find(code);
            if (it != keyIndeces_.end()) {
                for (size_t i = 0; i < it->second.size(); i++)
                    results.push_back(it->second[i]);
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

    // Load the prefix into SIMD registers
    const char* prefixData = prefix.data();
    __m256i prefixVec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(prefixData));

    for (const auto& [key, code] : dictionary_) {
        // Perform SIMD prefix comparison
        if (key.size() >= prefixLen) {
            const char* keyData = key.data();
            __m256i keyVec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(keyData));

            // Compare the SIMD register data for the prefix length
            __m256i cmpResult = _mm256_cmpeq_epi8(prefixVec, keyVec);
            int mask = _mm256_movemask_epi8(cmpResult);

            // Check if the prefix matches (first `prefixLen` bytes must be equal)
            if ((mask & ((1 << prefixLen) - 1)) == ((1 << prefixLen) - 1)) {
                auto it = keyIndeces_.find(code);
                if (it != keyIndeces_.end()) {
                    results.insert(results.end(), it->second.begin(), it->second.end());
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
        if (dataColumn_[i].compare(0, prefixLen, prefix)) {
            indices.push_back(i);
        }
    }
    return indices;
}
