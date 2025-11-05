#pragma once

#include <string>
#include <vector>

struct Macro {
    std::string name; // macro name, uppercase
    std::vector<std::string> args; // formal args, uppercase (max 2)
    std::vector<std::string> body; // body lines (already normalized to uppercase)
};