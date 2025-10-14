#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

// Funções auxiliares
vector<string> leitorDeArquivo(string nome_do_arquivo);
vector<string> leitorDeLinha(string linha); // A função não precisa mais do contador
bool isNumeric(const string& str);
string strType(const string& str);

// Lista de Palavras reservadas
vector<string> palavrasReservadas = {
        // Instruções da máquina hipotética
        "ADD",      // 0 Adição  
        "SUB",      // 1 Subtração 
        "MULT",     // 2 Multiplicação
        "DIV",      // 3 Divisão 
        "JMP",      // 4 Salto incondicional 
        "JMPN",     // 5 Salto se ACC é negativo 
        "JMPP",     // 6 Salto se ACC é positivo 
        "JMPZ",     // 7 Salto se Acc é igual a zero 
        "COPY",     // 8 Copia valor entre endereços de memória, m2 <- m1 
        "LOAD",     // 9 Carrega valor da memória para o acumulador 
        "STORE",    // 10 Armazena valor do acumulador na memória 
        "INPUT",    // 11 Lê um valor da entrada padrão para a memória 
        "OUTPUT",   // 12 Escreve um valor da memória na saída padrão 
        "STOP",     // 13 Suspende a execução 
        "SPACE",    // 14 Reserva espaço na memória
        "CONST"     // 15 Define uma constante na memória
};

// Lista da Sintaxe do Programa
vector<tuple<string, string, string>> regrasSintaticas = {
        // --- INSTRUÇÕES ---
        // A maioria das instruções segue o formato "Opcode Endereço" e ocupa 2 palavras de memória
       
        // 0 = Nada
        // 1 = Endereço de memória
        // 2 = Palavra Reservada
        // 3 = Constante
        {"ADD",    "1",    "0"},    // 0
        {"SUB",    "1",    "0"},    // 1 
        {"MULT",   "1",    "0"},    // 2
        {"DIV",    "1",    "0"},    // 3
        {"JMP",    "1",    "0"},    // 4
        {"JMPN",   "1",    "0"},    // 5
        {"JMPP",   "1",    "0"},    // 6
        {"JMPZ",   "1",    "0"},    // 7
        {"COPY",   "1",    "1"},    // 8
        {"LOAD",   "1",    "0"},    // 9
        {"STORE",  "1",    "0"},    // 10
        {"INPUT",  "1",    "0"},    // 11
        {"OUTPUT", "1",    "0"},    // 12
        {"STOP",   "0",    "0"},    // 13
        {"SPACE",  "0 | 3","0"},    // 14
        {"CONST",  "3",    "0"},    // 15
        {"XXX:",   "2",    "0"}     // 16 LABEL CASE
};

// Main
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Erro: Forneca um nome de arquivo como argumento.\n";
        cerr << "Uso: " << argv[0] << " arquivo.asm\n";
        return 1;
    }

    vector<string> lista_de_tokens = leitorDeArquivo(argv[1]);

    cout << "Tokens encontrados:\n";
    for (const string& token : lista_de_tokens) {
        cout << token << ": " << strType(token) << "\n";
    }
    cout << "\nTotal de tokens: " << lista_de_tokens.size() << endl;

    return 0;
}

/* Função que recebe um nome de arquivo e retorna um vetor de strings
	contendo os tokens (palavras) desse arquivo.*/
vector<string> leitorDeArquivo(string nome_do_arquivo) {
    vector<string> v_final;
    ifstream arquivo(nome_do_arquivo);
    string linha;

    if (arquivo.is_open()) {
        while (getline(arquivo, linha)) {

            vector<string> v_linha = leitorDeLinha(linha);
            v_final.insert(v_final.end(), v_linha.begin(), v_linha.end());
        }
        arquivo.close();
    } else {
        cerr << "Erro: Nao foi possivel abrir o arquivo '" << nome_do_arquivo << "'\n";
    }
    return v_final;
}

/* Função que recebe uma linha e retorna as palavras dela em um vetor.*/
vector<string> leitorDeLinha(string linha) {
	vector<string> v;
    string palavra = "";

    // Adiciona um espaço no final da linha para garantir que a última palavra seja capturada
    linha += " ";

    for (char chr : linha) {
    	// Converte o caractere para maiúsculo para atender ao requisito "aceitar maiúsculas e minúsculas" 
        chr = toupper(chr);
        
        // Ignora espaços, tabs e outros caracteres de espaçamento
        if (chr == ' ' || chr == '\t'|| chr == '\n' || chr == ',' || chr == ':') {
            if (!palavra.empty()) {
                v.push_back(palavra);
                palavra = "";
            }
        } else {
            palavra += chr;
        }
    }
    return v;
}

// Função para verificar se a string é um número (simplificado)
bool isNumeric(const string& str) {
    if (str.empty()) return false;
    for (char const &c : str) {
        if (isdigit(c) == 0) return false;
    }
    return true;
}


/* Função que recebe uma string e procura essa string no vector de palavras reservadas.
   Caso não seja uma palavra reservada então é possívelmente um rótulo. */
 
string strType(const string& str) {
    // 1. Verifica se é uma palavra reservada
    auto it = find(palavrasReservadas.begin(), palavrasReservadas.end(), str);
    if (it != palavrasReservadas.end()) {
        return to_string(distance(palavrasReservadas.begin(), it)); // Retorna o índice (0 a 17)
    }

    // 2. Verifica se é um identificador (label/símbolo)
    // Regra: Começa com letra ou '_', seguido por letras, '_' ou números.
    if (!str.empty() && (isalpha(str[0]) || str[0] == '_')) {
        bool identificadorValido = true;
        for (size_t i = 1; i < str.length(); ++i) {
            if (!std::isalnum(str[i]) && str[i] != '_') {
                identificadorValido = false;
                break;
            }
        }
        if (identificadorValido) {
            return "1";
        }
    }
    
    // 3. Verifica se é uma constante numérica
    if (isNumeric(str)) {
        return "3";
    }

    // 4. Se não for nenhum dos anteriores, é inválido
    return "-1";
}




