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

    // Load from raw text file and save encoded data
    if (strcmp(argv[1], "write_encoding") == 0) {
        DictionaryCodec dict;

        dict.EncodeColumnFile("src/Column.txt", "src/Output.txt");
    }

    // Query tests demo
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

        // Setup random values for experiment
        std::vector<size_t> randVals;
        size_t num_tests = 100;
        for (size_t i = 0; i < num_tests; i++) {
            randVals.push_back(dist(rd));
        }

        std::cout << "Number of indexes returned for each method:" << std::endl;

        // Print out results for accuracy testing
        for (size_t i = 0; i < 10; i++) {
            std::cout << dict.QueryItem(dict.GetData(randVals[i])).size() << " ";
            std::cout << dict.SIMDQueryItem(dict.GetData(randVals[i])).size() << " ";
            std::cout << dict.BaselineSearch(dict.GetData(randVals[i])).size() << std::endl;
        }

        // Timing the QueryItem operation
        auto startQuery = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < num_tests; i++) {
            std::vector<size_t> queryResults = dict.QueryItem(dict.GetData(randVals[i]));
        }

        auto endQuery = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> queryDuration = endQuery - startQuery;
        std::cout << "QueryItem execution time: " << queryDuration.count()/num_tests << " seconds" << std::endl;

        // Timing the SIMDQueryItem operation
        auto startQuerySIMD = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < num_tests; i++) {
            std::vector<size_t> queryResults = dict.SIMDQueryItem(dict.GetData(randVals[i]));
        }

        auto endQuerySIMD = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> queryDurationSIMD = endQuerySIMD - startQuerySIMD;
        std::cout << "QueryItemSIMD execution time: " << queryDurationSIMD.count()/num_tests << " seconds" << std::endl;

        // Timing the BaselineSearch operation
        auto startBaseline = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < num_tests; i++) {
            std::vector<size_t> baselineResults = dict.BaselineSearch(dict.GetData(randVals[i]));
        }

        auto endBaseline = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> baselineDuration = endBaseline - startBaseline;
        std::cout << "BaselineSearch execution time: " << baselineDuration.count()/num_tests << " seconds" << std::endl;
    }

    // Prefix query tests demo
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

        // Setup random values for experiment
        std::vector<size_t> randVals;
        size_t num_tests = 100;
        for (size_t i = 0; i < num_tests; i++) {
            randVals.push_back(dist(rd));
        }

        std::cout << "Number of indexes returned for each method:" << std::endl;
        // Print out results for accuracy testing
        for (size_t i = 0; i < 10; i++) {
            std::cout << dict.QueryByPrefix(dict.GetData(randVals[i])).size() << " ";
            std::cout << dict.SIMDQueryByPrefix(dict.GetData(randVals[i])).size() << " ";
            std::cout << dict.BaselinePrefixSearch(dict.GetData(randVals[i])).size() << std::endl;
        }

        // Timing the QueryPrefix operation
        auto startQuery = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < num_tests; i++) {
            std::vector<size_t> queryResults = dict.QueryByPrefix(dict.GetData(randVals[i]));
        }

        auto endQuery = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> queryDuration = endQuery - startQuery;
        std::cout << "QueryByPrefix execution time: " << queryDuration.count()/num_tests << " seconds" << std::endl;

        // Timing the SIMDQueryPrefix operation
        auto startQuerySIMD = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < num_tests; i++) {
            std::vector<size_t> queryResults = dict.SIMDQueryByPrefix(dict.GetData(randVals[i]));
        }

        auto endQuerySIMD = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> queryDurationSIMD = endQuerySIMD - startQuerySIMD;
        std::cout << "QueryByPrefixSIMD execution time: " << queryDurationSIMD.count()/num_tests << " seconds" << std::endl;

        // Timing the BaselineSearch operation
        auto startBaseline = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < num_tests; i++) {
            std::vector<size_t> baselineResults = dict.BaselinePrefixSearch(dict.GetData(randVals[i]));
        }

        auto endBaseline = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> baselineDuration = endBaseline - startBaseline;
        std::cout << "BaselinePrefixSearch execution time: " << baselineDuration.count()/num_tests << " seconds" << std::endl;
    }

    // Prefix query tests demo
    else if (strcmp(argv[1], "encoding_speed") == 0) {

    }

    else {
        std::cout << "Error: please refer to README.md for correct usage." << std::endl;
    }

    return 0;
}
