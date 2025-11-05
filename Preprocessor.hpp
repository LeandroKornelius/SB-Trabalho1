#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "Macro.hpp"

class Preprocessor {
private:
    std::vector<Macro> macros; // supports more but will warn if >2

    static std::string toUpper(const std::string& s);
    static std::string trim(const std::string& s);
    static std::string collapseSpaces(const std::string& s);
    static std::vector<std::string> splitArgs(const std::string& s);

    void storeMacro(std::ifstream& fin, const std::string& firstLine);
    bool isMacroCall(const std::string& line, const Macro*& outMacro, std::vector<std::string>& callArgs) const;
    void expandMacro(std::ofstream& fout, const Macro& macro, const std::vector<std::string>& args, int depth = 0);


    // replace formal args by actuals, but replace only whole tokens (alnum or '_')
    static std::string substituteArgsInLine(const std::string& line, const std::vector<std::string>& formals, const std::vector<std::string>& actuals);

public:
    Preprocessor() = default;
    void process(const std::string& inputFile);
};