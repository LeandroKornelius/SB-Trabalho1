#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <cctype>

using namespace std;

// ============================================================================
// CONSTANTES E TIPOS
// ============================================================================

enum TokenType {
    INVALID = 0,
    ADD = 1,
    SUB = 2,
    MULT = 3,
    DIV = 4,
    JMP = 5,
    JMPN = 6,
    JMPP = 7,
    JMPZ = 8,
    COPY = 9,
    LOAD = 10,
    STORE = 11,
    INPUT = 12,
    OUTPUT = 13,
    STOP = 14,
    SPACE = 15,
    CONST = 16,
    PLUS = 17,
    LABEL = 20,
    NUMBER = 30
};

const vector<string> RESERVED_WORDS = {
    "",      "ADD",   "SUB",   "MULT",  "DIV",
    "JMP",   "JMPN",  "JMPP",  "JMPZ",  "COPY",
    "LOAD",  "STORE", "INPUT", "OUTPUT","STOP",
    "SPACE", "CONST", "+"
};

const vector<vector<int>> SYNTAX_RULES = {
    {ADD, LABEL},           {SUB, LABEL},         {MULT, LABEL},
    {DIV, LABEL},           {JMP, LABEL},         {JMPN, LABEL},
    {JMPP, LABEL},          {JMPZ, LABEL},        {COPY, LABEL, LABEL},
    {LOAD, LABEL},          {STORE, LABEL},       {INPUT, LABEL},
    {OUTPUT, LABEL},        {STOP},               {CONST, NUMBER},
    {LABEL},                {LABEL, SPACE},       {LABEL, SPACE, NUMBER},
    {LABEL, ADD, LABEL},    {LABEL, SUB, LABEL},  {LABEL, MULT, LABEL},
    {LABEL, DIV, LABEL},    {LABEL, JMP, LABEL},  {LABEL, JMPN, LABEL},
    {LABEL, JMPP, LABEL},   {LABEL, JMPZ, LABEL}, {LABEL, COPY, LABEL, LABEL},
    {LABEL, LOAD, LABEL},   {LABEL, STORE, LABEL},{LABEL, INPUT, LABEL},
    {LABEL, OUTPUT, LABEL}, {LABEL, STOP},        {LABEL, CONST, NUMBER},
    // Suporte para aritmética de endereços (LABEL + NUMBER)
    {ADD, LABEL, PLUS, NUMBER},     {SUB, LABEL, PLUS, NUMBER},
    {MULT, LABEL, PLUS, NUMBER},    {DIV, LABEL, PLUS, NUMBER},
    {JMP, LABEL, PLUS, NUMBER},     {JMPN, LABEL, PLUS, NUMBER},
    {JMPP, LABEL, PLUS, NUMBER},    {JMPZ, LABEL, PLUS, NUMBER},
    {LOAD, LABEL, PLUS, NUMBER},    {STORE, LABEL, PLUS, NUMBER},
    {INPUT, LABEL, PLUS, NUMBER},   {OUTPUT, LABEL, PLUS, NUMBER},
    {LABEL, ADD, LABEL, PLUS, NUMBER},    {LABEL, SUB, LABEL, PLUS, NUMBER},
    {LABEL, MULT, LABEL, PLUS, NUMBER},   {LABEL, DIV, LABEL, PLUS, NUMBER},
    {LABEL, JMP, LABEL, PLUS, NUMBER},    {LABEL, JMPN, LABEL, PLUS, NUMBER},
    {LABEL, JMPP, LABEL, PLUS, NUMBER},   {LABEL, JMPZ, LABEL, PLUS, NUMBER},
    {LABEL, LOAD, LABEL, PLUS, NUMBER},   {LABEL, STORE, LABEL, PLUS, NUMBER},
    {LABEL, INPUT, LABEL, PLUS, NUMBER},  {LABEL, OUTPUT, LABEL, PLUS, NUMBER}
};

const int MAX_ADDRESS = 216;

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

struct SymbolTableEntry {
    string label;
    int address;
};

struct PendingReference {
    string label;
    vector<int> positions;
    vector<int> offsets;  // Offset para cada posição (0 se não houver offset)
};

class Assembler {
private:
    // Contadores
    int currentLine;
    int currentAddress;
    int currentPosition;
    int wordCount;
    int lastToken;
    int pendingOffset;  // Offset temporário para expressões LABEL + NUMBER
    
    // Estruturas de dados
    vector<int> addressList;
    vector<SymbolTableEntry> symbolTable;
    vector<PendingReference> pendingReferences;
    
    // Métodos auxiliares
    void processFile(const string& filename);
    void processLine(const string& line);
    vector<string> tokenizeLine(const string& line);
    vector<int> analyzeTokens(const vector<string>& tokens);
    int analyzeLexeme(const string& str, int position, const vector<string>& allTokens);
    bool isSyntaxValid(const vector<int>& tokens);
    bool isLabel(const string& str);
    bool isNumber(const string& str);
    int findSymbol(const string& label);
    int findPending(const string& label);
    void addToSymbolTable(const string& label, int address);
    void addToPendingList(const string& label, int position);
    void resolvePendingReferences();
    void processReservedWord(int tokenType);
    void processLabelReference(const string& label, int position);
    void processLabelDefinition(const string& label);
    void processNumber(const string& str);
    
    // Métodos de exibição
    void showSymbolTable();
    void showPendingReferences();
    void showRawOutput();
    void showFinalOutput();
    void showAll();

public:
    Assembler();
    void compile(const string& filename);
    void displayOutput(const string& option);
};

// ============================================================================
// IMPLEMENTAÇÃO DO ASSEMBLER
// ============================================================================

Assembler::Assembler() 
    : currentLine(1), currentAddress(0), currentPosition(0), 
      wordCount(0), lastToken(0), pendingOffset(0) {
    addressList.resize(MAX_ADDRESS, 0);
}

void Assembler::compile(const string& filename) {
    try {
        processFile(filename);
    } catch (const runtime_error& e) {
        throw runtime_error("Falha na compilacao: " + string(e.what()));
    }
}

void Assembler::processFile(const string& filename) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        throw runtime_error("Nao foi possivel abrir o arquivo '" + filename + "'");
    }
    
    string line;
    while (getline(file, line)) {
        processLine(line);
        currentLine++;
    }
    
    file.close();
}

void Assembler::processLine(const string& line) {
    vector<string> tokens = tokenizeLine(line);
    if (tokens.empty()) return;
    
    pendingOffset = 0;  // Reset offset no início de cada linha
    vector<int> tokenTypes = analyzeTokens(tokens);
    
    if (!isSyntaxValid(tokenTypes)) {
        throw runtime_error("Erro Sintatico na linha [" + 
                          to_string(currentLine) + "]: " + line);
    }
    
    pendingOffset = 0;  // Reset offset no final de cada linha também
}

vector<string> Assembler::tokenizeLine(const string& line) {
    vector<string> tokens;
    string word;
    string processedLine = line + " ";
    
    for (char ch : processedLine) {
        char upperCh = toupper(ch);
        
        if (upperCh == ' ' || upperCh == '\t' || upperCh == ',' || upperCh == ':') {
            if (!word.empty()) {
                tokens.push_back(word);
                word.clear();
            }
        } else {
            word += upperCh;
        }
    }
    
    return tokens;
}

vector<int> Assembler::analyzeTokens(const vector<string>& tokens) {
    vector<int> tokenTypes;
    int position = 0;
    
    for (size_t i = 0; i < tokens.size(); i++) {
        // Detecta padrão LABEL + NUMBER antes de processar o token atual
        if (isLabel(tokens[i]) && 
            i + 2 < tokens.size() && 
            tokens[i+1] == "+" && 
            isNumber(tokens[i+2])) {
            // Seta o offset antes de processar o label
            pendingOffset = stoi(tokens[i+2]);
        }
        
        tokenTypes.push_back(analyzeLexeme(tokens[i], position, tokens));
        position++;
    }
    
    return tokenTypes;
}

int Assembler::analyzeLexeme(const string& str, int position, const vector<string>& allTokens) {
    // Verifica se é palavra reservada
    auto it = find(RESERVED_WORDS.begin(), RESERVED_WORDS.end(), str);
    
    if (it != RESERVED_WORDS.end()) {
        int tokenType = distance(RESERVED_WORDS.begin(), it);
        processReservedWord(tokenType);
        lastToken = tokenType;
        return tokenType;
    }
    
    // Verifica se é um rótulo
    if (isLabel(str)) {
        int symbolIndex = findSymbol(str);
        
        if (symbolIndex >= 0) {
            // Rótulo já definido
            if (position == 0) {
                throw runtime_error("Erro Semantico na linha [" + 
                                  to_string(currentLine) + 
                                  "]: Rotulo ja definido");
            }
            processLabelReference(str, symbolIndex);
        } else {
            // Rótulo não definido ainda
            if (position == 0) {
                processLabelDefinition(str);
            } else {
                // Adiciona à lista de pendências
                addToPendingList(str, currentPosition);
            }
        }
        
        lastToken = LABEL;
        return LABEL;
    }
    
    // Verifica se é um número
    if (isNumber(str)) {
        processNumber(str);
        return NUMBER;
    }
    
    throw runtime_error("Erro Lexico na linha [" + 
                      to_string(currentLine) + 
                      "]: Token '" + str + "' invalido");
}

void Assembler::processReservedWord(int tokenType) {
    if (tokenType == SPACE) {
        addressList[currentPosition] = 0;
        currentPosition++;
        wordCount++;
        currentAddress++;
    } else if (tokenType != CONST && tokenType != PLUS && tokenType != INVALID) {
        addressList[currentPosition] = tokenType;
        currentPosition++;
        wordCount++;
        currentAddress++;
    }
}

void Assembler::processLabelReference(const string& label, int symbolIndex) {
    int offset = pendingOffset;  // Captura o offset atual
    addressList[currentPosition] = symbolTable[symbolIndex].address + offset;
    currentAddress++;
    currentPosition++;
    wordCount++;
    pendingOffset = 0;  // Reset offset
}

void Assembler::processLabelDefinition(const string& label) {
    addToSymbolTable(label, currentAddress);
}

void Assembler::processNumber(const string& str) {
    wordCount++;
    int value = stoi(str);
    
    if (lastToken == STOP) {
        currentAddress += value;
        currentPosition++;
    } else if (lastToken == PLUS) {
        // Este é um offset para uma expressão LABEL + NUMBER
        pendingOffset = value;
        wordCount--;  // Não conta o número como palavra separada
    } else {
        addressList[currentPosition] = value;
        currentAddress++;
        currentPosition++;
    }
}

bool Assembler::isSyntaxValid(const vector<int>& tokens) {
    if (tokens.empty()) return true;
    return find(SYNTAX_RULES.begin(), SYNTAX_RULES.end(), tokens) != SYNTAX_RULES.end();
}

bool Assembler::isLabel(const string& str) {
    if (str.empty() || (!isalpha(str[0]) && str[0] != '_')) {
        return false;
    }
    
    for (size_t i = 1; i < str.length(); i++) {
        if (!isalnum(str[i]) && str[i] != '_') {
            return false;
        }
    }
    
    return true;
}

bool Assembler::isNumber(const string& str) {
    if (str.empty()) return false;
    
    for (char ch : str) {
        if (!isdigit(ch)) return false;
    }
    
    return true;
}

int Assembler::findSymbol(const string& label) {
    for (size_t i = 0; i < symbolTable.size(); i++) {
        if (symbolTable[i].label == label) {
            return i;
        }
    }
    return -1;
}

int Assembler::findPending(const string& label) {
    for (size_t i = 0; i < pendingReferences.size(); i++) {
        if (pendingReferences[i].label == label) {
            return i;
        }
    }
    return -1;
}

void Assembler::addToSymbolTable(const string& label, int address) {
    symbolTable.push_back({label, address});
}

void Assembler::addToPendingList(const string& label, int position) {
    wordCount++;
    int pendingIndex = findPending(label);
    int offset = pendingOffset;  // Captura o offset atual
    
    if (pendingIndex >= 0) {
        // Já existe na lista de pendências
        pendingReferences[pendingIndex].positions.push_back(currentAddress);
        pendingReferences[pendingIndex].offsets.push_back(offset);
    } else {
        // Cria nova entrada
        PendingReference newPending;
        newPending.label = label;
        newPending.positions.push_back(currentAddress);
        newPending.offsets.push_back(offset);
        pendingReferences.push_back(newPending);
    }
    
    currentAddress++;
    currentPosition++;
    pendingOffset = 0;  // Reset offset
}

void Assembler::resolvePendingReferences() {
    for (const auto& pending : pendingReferences) {
        int symbolIndex = findSymbol(pending.label);
        
        if (symbolIndex == -1) {
            throw runtime_error("Erro Semantico: rotulo nao definido: " + pending.label);
        }
        
        int baseAddress = symbolTable[symbolIndex].address;
        
        for (size_t i = 0; i < pending.positions.size(); i++) {
            int pos = pending.positions[i];
            int offset = pending.offsets[i];
            addressList[pos] = baseAddress + offset;
        }
    }
}

void Assembler::showSymbolTable() {
    cout << "====================\n";
    cout << "=Tabela de Simbolos=\n";
    cout << "====================\n\n";
    
    for (const auto& entry : symbolTable) {
        cout << entry.label << " (&" << entry.address << ")\n";
    }
    cout << "\n";
}

void Assembler::showPendingReferences() {
    cout << "=====================\n";
    cout << "=Lista de Pendencias=\n";
    cout << "=====================\n\n";
    
    for (const auto& pending : pendingReferences) {
        cout << pending.label << " [ ";
        for (size_t i = 0; i < pending.positions.size(); i++) {
            cout << pending.positions[i];
            if (pending.offsets[i] > 0) {
                cout << "+" << pending.offsets[i];
            }
            cout << " ";
        }
        cout << "]\n";
    }
    cout << "\n";
}

void Assembler::showRawOutput() {
    // Mostra saída não tratada (com pendências como linked list)
    vector<int> tempList = addressList;
    
    for (const auto& pending : pendingReferences) {
        int previous = -1;
        for (int pos : pending.positions) {
            tempList[pos] = previous;
            previous = pos;
        }
    }
    
    for (int i = 0; i < wordCount; i++) {
        cout << tempList[i] << " ";
    }
    cout << "\n";
}

void Assembler::showFinalOutput() {
    resolvePendingReferences();
    
    for (int i = 0; i < wordCount; i++) {
        cout << addressList[i] << " ";
    }
    cout << "\n";
}

void Assembler::showAll() {
    showSymbolTable();
    showPendingReferences();
    
    cout << "======================\n";
    cout << "= Codigo sem correcao=\n";
    cout << "=    de pendencias   =\n";
    cout << "======================\n";
    showRawOutput();
    
    cout << "\n";
    cout << "==============\n";
    cout << "=    Final   =\n";
    cout << "==============\n\n";
    showFinalOutput();
}

void Assembler::displayOutput(const string& option) {
    if (option == "all") {
        showAll();
    } else if (option == "o1") {
        showRawOutput();
    } else if (option == "o2") {
        showFinalOutput();
    } else {
        cout << "Insira um argumento valido: all, o1, o2.\n";
    }
}

// ============================================================================
// FUNÇÃO PRINCIPAL
// ============================================================================

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Erro: Forneca um nome de arquivo como argumento.\n";
        cerr << "Uso: " << argv[0] << " arquivo.asm [all|o1|o2]\n";
        return 1;
    }
    
    try {
        Assembler assembler;
        assembler.compile(argv[1]);
        assembler.displayOutput(argv[2]);
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }
    
    return 0;
}
