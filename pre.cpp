#include <iostream>
#include <string>
#include "Preprocessor.hpp"


int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.asm>\n";
        return 1;
    }
    std::string input = argv[1];
    try {
        Preprocessor pp;
        pp.process(input);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}