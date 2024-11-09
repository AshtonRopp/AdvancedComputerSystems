#include "Codec.h"
#include <iostream>
#include <string.h>  // strcmp 


int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Error: please refer to README.md for correct usage." << std::endl;
        return 0;
    }

    if (strcmp(argv[1], "write_encoding") == 0) {
        DictionaryCodec dict;

        dict.EncodeColumnFile("src/Column.txt", "src/Output.txt");
    }

    else if (strcmp(argv[1], "query_items") == 0) {
        DictionaryCodec dict;

    }

    else if (strcmp(argv[1], "query_prefix") == 0) {

    }

    else {
        std::cout << "Error: please refer to README.md for correct usage." << std::endl;
    }

    return 0;
} 