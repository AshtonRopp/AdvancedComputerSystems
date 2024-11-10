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
#include <memory>

class DictionaryCodec {
public:
    // Constructor
    DictionaryCodec();

    // Encoding: Perform dictionary encoding on a column file and generate an encoded output
    bool EncodeColumnFile(const std::string& inputFile, const std::string& outputFile);

    // Query: Check if a data item exists in the encoded column, and return indices if it does
    std::vector<unsigned int> QueryItem(const std::string& dataItem);

    // Query with Prefix: Search for items matching a prefix, returning unique items and their indices
    std::unordered_map<std::string, std::vector<size_t>> QueryByPrefix(const std::string& prefix) const;

    // Baseline Column Search (without dictionary encoding) for performance comparison
    std::vector<unsigned int> BaselineSearch(const std::string& dataItem);

    // Helper to load encoded data from file
    void LoadEncodedFile(const std::string& inputFile);

    // Getter for dataColumn_
    const std::string& GetData(size_t index) const { return dataColumn_[index]; }

    // Returns size of dataColumn_
    size_t GetDataSize() const { return dataSize_; }


private:
    // Dictionary and encoded data storage
    std::unordered_map<std::string, int> dictionary_;               // Maps data items to unique integer codes
    std::unordered_map<int, std::vector<unsigned int>> keyIndeces_; // Maps keys to indeces
    std::unique_ptr<std::string[]> dataColumn_;                     // Unencoded data
    std::vector<int> encodedColumn_;                                // Encoded column data as integers (keys)
    mutable std::shared_mutex dictionaryMutex_;                     // Mutex for thread-safe access to dictionary
    size_t dataSize_;

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
