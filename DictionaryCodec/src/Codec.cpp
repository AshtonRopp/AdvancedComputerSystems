// Codec.cpp
#include "Codec.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <thread>
#include <immintrin.h>  // For SIMD

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

    unsigned int ind = 0;
    int keyVal;
    std::string dataStr;

    while (file >> keyVal) {
        keyIndeces_[keyVal].push_back(ind);
        file >> dataStr;
        dictionary_[dataStr] = keyVal;
        dataColumn_.push_back(dataStr);

        ind++;
    }

    file.close();

}

// Helper to write the dictionary and encoded column to a file
bool DictionaryCodec::WriteEncodedColumnFile(const std::string& outputFile, const std::vector<std::string>& columnData) const {
    std::cout << "Writing file." << std::endl;

    std::ofstream file(outputFile);
    if (!file.is_open()) return false;

    // Write dictionary --> format: key data
    for (unsigned int i = 0; i < encodedColumn_.size(); i++) {
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
std::vector<unsigned int> DictionaryCodec::QueryItem(const std::string& dataItem) {
    std::vector<unsigned int> indices;

    std::shared_lock lock(dictionaryMutex_);
    std::shared_lock lock2(keyIndecesMutex_);

    auto it = dictionary_.find(dataItem);
    if (it == dictionary_.end()) return indices;

    auto it2 = keyIndeces_.find(it->second);
    if (it2 == keyIndeces_.end()) return indices;

    return keyIndeces_[dictionary_[dataItem]];
}

// SIMD-assisted prefix search
std::unordered_map<std::string, std::vector<size_t>> DictionaryCodec::SIMDSearchByPrefix(const std::string& prefix) const {
    std::unordered_map<std::string, std::vector<size_t>> results;
    size_t prefixLen = prefix.size();

    for (const auto& [key, code] : dictionary_) {
        if (key.compare(0, prefixLen, prefix) == 0) {
            std::vector<size_t> indices;
            for (size_t i = 0; i < encodedColumn_.size(); ++i) {
                if (encodedColumn_[i] == code) {
                    indices.push_back(i);
                }
            }
            results[key] = indices;
        }
    }
    return results;
}

// Query by prefix, using SIMD where applicable
std::unordered_map<std::string, std::vector<size_t>> DictionaryCodec::QueryByPrefix(const std::string& prefix) const {
    return SIMDSearchByPrefix(prefix);
}

// Baseline column search (without dictionary encoding) for performance comparison
std::vector<size_t> DictionaryCodec::BaselineSearch(const std::string& dataItem, const std::string& inputFile) {
    std::vector<size_t> indices;
    auto columnData = LoadColumnFile(inputFile);

    for (size_t i = 0; i < columnData.size(); ++i) {
        if (columnData[i] == dataItem) {
            indices.push_back(i);
        }
    }
    return indices;
}
