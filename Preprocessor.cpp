#include "Preprocessor.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <iostream>

static bool isIdentChar(char c) {
    return (std::isalnum(static_cast<unsigned char>(c)) || c == '_');
}

std::string Preprocessor::toUpper(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::toupper(c); });
    return out;
}

std::string Preprocessor::trim(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) ++i;
    if (i == s.size()) return "";
    size_t j = s.size() - 1;
    while (j > i && (s[j] == ' ' || s[j] == '\t' || s[j] == '\r' || s[j] == '\n')) --j;
    return s.substr(i, j - i + 1);
}

std::string Preprocessor::collapseSpaces(const std::string& s) {
    std::string out;
    bool inSpace = false;
    for (char c : s) {
        if (c == ' ' || c == '\t') {
            if (!inSpace) { out.push_back(' '); inSpace = true; }
        } else {
            out.push_back(c);
            inSpace = false;
        }
    }
    return out;
}

std::vector<std::string> Preprocessor::splitArgs(const std::string& s) {
    std::vector<std::string> parts;
    std::string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == ',') {
            std::string t = trim(cur);
            if (!t.empty()) parts.push_back(t);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    std::string t = trim(cur);
    if (!t.empty()) parts.push_back(t);
    return parts;
}

void Preprocessor::storeMacro(std::ifstream& fin, const std::string& firstLineRaw) {
    
    // limite de 2 macros
    if (macros.size() >= 2) {
        throw std::runtime_error("Erro: Mais de 2 macros definidas no programa (Limite da especificacao).");
    }

    std::string firstLine = toUpper(trim(firstLineRaw));

    // Parse do cabeçalho da macro: NOME: MACRO [args]
    size_t colon = firstLine.find(':');
    if (colon == std::string::npos) {
        throw std::runtime_error("Erro: Definicao de macro deve usar 'LABEL: MACRO ...'");
    }
    std::string name = trim(firstLine.substr(0, colon));
    size_t macroPos = firstLine.find("MACRO", colon);
    if (macroPos == std::string::npos) {
        throw std::runtime_error("Erro: Linha de definicao de macro nao contem 'MACRO'.");
    }

    Macro m;
    m.name = name;

    // Parse dos argumentos
    std::string argsPart = trim(firstLine.substr(macroPos + 5));
    if (!argsPart.empty()) {
        auto parts = splitArgs(argsPart);
        for (auto& p : parts) {
            std::string up = toUpper(p);
            m.args.push_back(up);
        }
    }

    // verifica quantidade de args
    if (m.args.size() > 2) {
        throw std::runtime_error("Erro: Macro '" + m.name + "' definida com mais de 2 argumentos (Limite da especificacao).");
    }

    // Leitura do corpo da macro
    std::string line;
    while (std::getline(fin, line)) {
        
        // remove comentarios da macro
        std::string noComment;
        size_t commentPos = line.find(';'); // Procura o ';' na linha crua
        if (commentPos != std::string::npos) {
            noComment = line.substr(0, commentPos); // Pega só a parte antes
        } else {
            noComment = line; // Linha inteira
        }
        

        std::string norm = toUpper(trim(noComment)); // Usa 'noComment'
        
        if (norm.empty()) continue; // Pula linhas em branco

        // Verifica o fim da macro
        if (norm == "ENDMACRO") break;
        
        // Normaliza espaçamento e armazena a linha do corpo
        norm = collapseSpaces(norm);
        m.body.push_back(norm);
    }
    
    // Armazena a macro finalizada no vetor
    macros.push_back(std::move(m));
}

bool Preprocessor::isMacroCall(const std::string& line, const Macro*& outMacro, std::vector<std::string>& callArgs) const {
    // line is already normalized to uppercase and trimmed & collapsed
    for (const auto& m : macros) {
        // match only if the line starts with macro name and next char is space or end or '(' (but spec uses comma)
        if (line.size() >= m.name.size() && line.compare(0, m.name.size(), m.name) == 0) {
            if (line.size() == m.name.size()) {
                outMacro = &m;
                callArgs.clear();
                return true;
            }
            char next = line[m.name.size()];
            if (next == ' ' || next == '\t') {
                std::string rest = trim(line.substr(m.name.size()));
                // rest contains arguments possibly separated by commas
                // some calls might write: NAME ARG1, ARG2
                if (!rest.empty()) {
                    auto parts = splitArgs(rest);
                    callArgs.clear();
                    for (auto& p : parts) callArgs.push_back(toUpper(p));
                } else callArgs.clear();
                outMacro = &m;
                return true;
            }
        }
    }
    return false;
}

std::string Preprocessor::substituteArgsInLine(const std::string& line, const std::vector<std::string>& formals, const std::vector<std::string>& actuals) {
    if (formals.empty()) return line;
    std::string out;
    out.reserve(line.size() + 16);

    size_t i = 0;
    while (i < line.size()) {
        if (isIdentChar(line[i])) {
            size_t j = i;
            while (j < line.size() && isIdentChar(line[j])) ++j;
            std::string token = line.substr(i, j - i);
            bool replaced = false;
            for (size_t k = 0; k < formals.size() && k < actuals.size(); ++k) {
                if (token == formals[k]) {
                    out += actuals[k];
                    replaced = true;
                    break;
                }
            }
            if (!replaced) out += token;
            i = j;
        } else {
            out.push_back(line[i]);
            ++i;
        }
    }
    return out;
}

void Preprocessor::expandMacro(std::ofstream& fout, const Macro& macro, const std::vector<std::string>& args, int depth) {
    if (depth > 20) throw std::runtime_error("Macro expansion exceeded maximum depth (possible recursion)");
    // expand each body line, substitute args, and write; if expanded line is another macro call, expand recursively
    for (auto bline : macro.body) {
        // substitute formal arguments
        std::string replaced = substituteArgsInLine(bline, macro.args, args);
        // normalize spacing and trim
        replaced = trim(collapseSpaces(replaced));
        if (replaced.empty()) continue;

        // check if this line is a macro call itself
        const Macro* inner = nullptr;
        std::vector<std::string> innerArgs;
        if (isMacroCall(replaced, inner, innerArgs)) {
            expandMacro(fout, *inner, innerArgs, depth + 1);
        } else {
            fout << replaced << "\n";
        }
    }
}

void Preprocessor::process(const std::string& inputFile) {
    std::ifstream fin(inputFile);
    if (!fin.is_open()) throw std::runtime_error("Unable to open input file: " + inputFile);

    // create output filename by replacing extension with .pre
    std::string outFile = inputFile;
    size_t pos = outFile.find_last_of('.');
    if (pos == std::string::npos) outFile += ".pre";
    else outFile = outFile.substr(0, pos) + ".pre";

    std::ofstream fout(outFile);
    if (!fout.is_open()) throw std::runtime_error("Unable to create output file: " + outFile);

    std::string rawLine;
    while (std::getline(fin, rawLine)) {
        // Remove comments starting at ';' if any
        std::string noComment;
        size_t commentPos = rawLine.find(';');
        if (commentPos != std::string::npos) noComment = rawLine.substr(0, commentPos);
        else noComment = rawLine;

        std::string normalized = toUpper(trim(noComment));
        if (normalized.empty()) continue; // skip blank lines
        normalized = collapseSpaces(normalized);

        // If this is a macro definition starter (contains ': MACRO' pattern)
        // We rely on presence of token MACRO in the line
        if (normalized.find("MACRO") != std::string::npos) {
            storeMacro(fin, normalized);
            continue; // do not write macro definition to output
        }

        // check if this is a macro call
        const Macro* called = nullptr;
        std::vector<std::string> callArgs;
        if (isMacroCall(normalized, called, callArgs)) {
            expandMacro(fout, *called, callArgs, 0);
        } else {
            fout << normalized << "\n";
        }
    }

    // done
    fout.close();
    fin.close();
    std::cerr << "Preprocessing finished. Output: " << outFile << "\n";
}
