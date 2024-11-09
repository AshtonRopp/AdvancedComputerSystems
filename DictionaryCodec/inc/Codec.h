// Codec.h
#ifndef DICTIONARY_CODEC_H
#define DICTIONARY_CODEC_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <immintrin.h>  // For SIMD instructions
#include <thread>

class DictionaryCodec {
public:
    // Constructor
    DictionaryCodec();

    // Encoding: Perform dictionary encoding on a column file and generate an encoded output
    bool EncodeColumnFile(const std::string& inputFile, const std::string& outputFile);

    // Query: Check if a data item exists in the encoded column, and return indices if it does
    std::vector<size_t> QueryItem(const std::string& dataItem) const;

    // Query with Prefix: Search for items matching a prefix, returning unique items and their indices
    std::unordered_map<std::string, std::vector<size_t>> QueryByPrefix(const std::string& prefix) const;

    // Baseline Column Search (without dictionary encoding) for performance comparison
    std::vector<size_t> BaselineSearch(const std::string& dataItem, const std::string& inputFile);

private:
    // Dictionary and encoded data storage
    std::unordered_map<std::string, int> dictionary_;    // Maps data items to unique integer codes
    std::vector<int> encodedColumn_;                     // Encoded column data as integers
    mutable std::shared_mutex dictionaryMutex_;          // Mutex for thread-safe access to dictionary

    // Helper function to populate the dictionary using multiple threads
    void BuildDictionary(const std::vector<std::string>& columnData);

    // Helper function to perform SIMD-based search for prefix matching in encoded data
    std::unordered_map<std::string, std::vector<size_t>> SIMDSearchByPrefix(const std::string& prefix) const;

    // Helper to load a column file into memory for processing
    std::vector<std::string> LoadColumnFile(const std::string& inputFile) const;

    // Helper to write the dictionary and encoded column to a file
    bool WriteEncodedColumnFile(const std::string& outputFile, const std::vector<std::string>& columnData) const;
};

#endif // DICTIONARY_CODEC_H
