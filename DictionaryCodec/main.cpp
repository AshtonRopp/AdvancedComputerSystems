// Main.cpp
#include "Codec.h"
#include <iostream>
#include <string.h>  // strcmp
#include <random>
#include <chrono>


int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Error: please refer to README.md for correct usage." << std::endl;
        return 0;
    }

    std::random_device rd;

    if (strcmp(argv[1], "write_encoding") == 0) {
        DictionaryCodec dict;

        dict.EncodeColumnFile("src/Column.txt", "src/Output.txt");
    }

    else if (strcmp(argv[1], "query_items") == 0) {
        // Create the DictionaryCodec instance
        DictionaryCodec dict;

        // Load the encoded file
        dict.LoadEncodedFile("src/Output.txt");

        // Get the maximum index from the data size
        size_t maxIndex = dict.GetDataSize();

        // Setup random number generator
        std::random_device rd;
        std::uniform_int_distribution<size_t> dist(0, maxIndex - 1);

        // Timing the QueryItem operation
        auto startQuery = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1000; i++) {
            size_t ind = dist(rd);  // Random index
            std::vector<size_t> queryResults = dict.QueryItem(dict.GetData(ind));
        }

        auto endQuery = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> queryDuration = endQuery - startQuery;
        std::cout << "QueryItem execution time: " << queryDuration.count() << " seconds" << std::endl;

        // Timing the BaselineSearch operation
        auto startBaseline = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1000; i++) {
            size_t ind = dist(rd);  // Random index
            std::vector<size_t> baselineResults = dict.BaselineSearch(dict.GetData(ind));
        }

        auto endBaseline = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> baselineDuration = endBaseline - startBaseline;
        std::cout << "BaselineSearch execution time: " << baselineDuration.count() << " seconds" << std::endl;
    }

    else if (strcmp(argv[1], "query_prefix") == 0) {
        // Create the DictionaryCodec instance
        DictionaryCodec dict;

        // Load the encoded file
        dict.LoadEncodedFile("src/Output.txt");

        // Get the maximum index from the data size
        size_t maxIndex = dict.GetDataSize();

        // Setup random number generator
        std::random_device rd;
        std::uniform_int_distribution<size_t> dist(0, maxIndex - 1);

        // Timing the QueryItem operation
        auto startQuery = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 100; i++) {
            size_t ind = dist(rd);  // Random index
            std::vector<size_t> queryResults = dict.QueryByPrefix(dict.GetData(ind));
        }

        auto endQuery = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> queryDuration = endQuery - startQuery;
        std::cout << "QueryByPrefix execution time: " << queryDuration.count() << " seconds" << std::endl;

        // Timing the BaselineSearch operation
        auto startBaseline = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 100; i++) {
            size_t ind = dist(rd);  // Random index
            std::vector<size_t> baselineResults = dict.BaselinePrefixSearch(dict.GetData(ind));
        }

        auto endBaseline = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> baselineDuration = endBaseline - startBaseline;
        std::cout << "BaselinePrefixSearch execution time: " << baselineDuration.count() << " seconds" << std::endl;
    }


    else if (strcmp(argv[1], "query_prefix_SIMD") == 0) {
        // Create the DictionaryCodec instance
        DictionaryCodec dict;

        // Load the encoded file
        dict.LoadEncodedFile("src/Output.txt");

        // Get the maximum index from the data size
        size_t maxIndex = dict.GetDataSize();

        // Setup random number generator
        std::random_device rd;
        std::uniform_int_distribution<size_t> dist(0, maxIndex - 1);

        // Timing the QueryItem operation
        auto startQuery = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 10; i++) {
            size_t ind = dist(rd);  // Random index
            std::vector<size_t> queryResults = dict.QueryByPrefix(dict.GetData(ind));
        }

        auto endQuery = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> queryDuration = endQuery - startQuery;
        std::cout << "QueryByPrefix execution time: " << queryDuration.count() << " seconds" << std::endl;

        // Timing the BaselineSearch operation
        auto startSIMD = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 10; i++) {
            size_t ind = dist(rd);  // Random index
            std::vector<size_t> SIMDqueryResults = dict.SIMDQueryByPrefix(dict.GetData(ind));
        }

        auto endSIMD = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> SIMD_Duration = endSIMD - startSIMD;
        std::cout << "SIMDQueryByPrefix execution time: " << SIMD_Duration.count() << " seconds" << std::endl;
    }

    else {
        std::cout << "Error: please refer to README.md for correct usage." << std::endl;
    }

    return 0;
}
