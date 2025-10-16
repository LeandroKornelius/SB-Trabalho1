#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <tuple> // Necessário para usar std::tuple

using namespace std;

/* =====================================
    = Declaracao de Funcoes Auxiliares =
    ====================================
*/
void leitorDeArquivo(string nome_do_arquivo);
vector<string> analisadorSintatico(const string& linha);
vector<string> percorreLinha(string str);
vector<int> verificaTokens(const vector<string>& v); // Parâmetro como const reference

int analisadorLexico(const string& str, int contadorDePalavra); // Protótipo simplificado
int procuraNaTabelaDeSimbolos(const string& str);
int procuraNalistaDePendencias(const string& str);
bool isNumeric(const string& str);
bool isLabel(const string& str);
bool pertenceRegrasSintaticas(const vector<int>& v); // Parâmetro como const reference

/* ======================
    = Variaveis Globais =
    =====================
*/
int contadorDeLinha = 1;
int contadorDeEndereco = 0;

/* ===================================
   =       Lista de Pendencias       =
   ===================================
   = CORREÇÃO: O tipo do identificador
   = do rótulo deve ser 'string'.
   ===================================
*/
vector<tuple<string, vector<int>>> listaDePendencias;

/* ========================
   =   Tabela de Simbolos   =
   ========================
*/
vector<tuple<string, int>> tabelaDeSimbolos;

/* ================================
   = Lista de palavras reservadas =
   ================================
*/
vector<string> palavrasReservadas = 
{
    "ADD", "SUB", "MULT", "DIV", "JMP", "JMPN", "JMPP", "JMPZ", 
    "COPY", "LOAD", "STORE", "INPUT", "OUTPUT", "STOP", 
    "SPACE", "CONST"
};

/* ==================================
   = Lista da sintaxe do programa   =
   ==================================
*/
vector<vector<int>> regrasSintaticas = {        
    {0, 20}, {1, 20}, {2, 20}, {3, 20}, {4, 20}, {5, 20}, {6, 20}, {7, 20}, 
    {8, 20, 20}, {9, 20}, {10, 20}, {11, 20}, {12, 20}, {13}, {15, 30}, 
    {20, 14}, {20, 14, 30}, {20, 0, 20}, {20, 1, 20}, {20, 2, 20}, {20, 3, 20}, 
    {20, 4, 20}, {20, 5, 20}, {20, 6, 20}, {20, 7, 20}, {20, 8, 20, 20}, 
    {20, 9, 20}, {20, 10, 20}, {20, 11, 20}, {20, 12, 20}, {20, 13}, {20, 15, 30}
};

int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        cerr << "Erro: Forneca um nome de arquivo como argumento.\n";
        cerr << "Uso: " << argv[0] << " arquivo.asm\n";
        return 1;
    }

    try {
        leitorDeArquivo(argv[1]);
    } catch (const runtime_error& e) {
        cerr << "Falha na compilacao: " << e.what() << endl;
        return 1;
    }

    cout << "O codigo compilou sem erros." << endl;
    return 0;
}

void leitorDeArquivo(string nome_do_arquivo) 
{
    ifstream arquivo(nome_do_arquivo);
    string linha;

    if (arquivo.is_open()) 
    {
        while (getline(arquivo, linha)) 
        {
            vector<string> v_linha = analisadorSintatico(linha);
            
            // Lógica de impressão (para depuração)
            if (!v_linha.empty()) {
                cout << "[" << contadorDeLinha << "] ";
                for(const string& str : v_linha) {
                    cout << str << " ";
                }
                cout << "\n";
            }
            contadorDeLinha++;
        }
        arquivo.close();
    } 
    else 
    {
        cerr << "Erro: Nao foi possivel abrir o arquivo '" << nome_do_arquivo << "'\n";
    }
}

vector<string> analisadorSintatico(const string& linha) 
{
    vector<string> tokens = percorreLinha(linha);
    if (tokens.empty()) return {}; // Retorna vetor vazio para linhas vazias

    vector<int> v_int = verificaTokens(tokens);
    vector<string> v2;
    
    if(pertenceRegrasSintaticas(v_int))
    {
        for(int i: v_int)
        {
            v2.push_back(to_string(i));
        }
        return v2;
    }
    throw runtime_error("Erro Sintatico na linha:\n[" + to_string(contadorDeLinha) + "]" + linha + "\n" );
}

bool pertenceRegrasSintaticas(const vector<int>& v)
{
    if (v.empty()) return true; // Linha vazia é sintaticamente correta
    auto it = find(regrasSintaticas.begin(), regrasSintaticas.end(), v);
    return (it != regrasSintaticas.end());
}

vector<int> verificaTokens(const vector<string>& v)
{
	int contadorDePalavra = 0;
    vector<int> v_temp;
    for(const string& str: v)
    {
        v_temp.push_back(analisadorLexico(str, contadorDePalavra));
        contadorDePalavra ++;
    }
    return v_temp;
}

vector<string> percorreLinha(string linha)
{
    vector<string> v;
    string palavra = "";
    linha += " ";
    
    for (char chr : linha) 
    {
        chr = toupper(chr);
        if (chr == ' ' || chr == '\t' || chr == ',' || chr == ':') 
        {
            if (!palavra.empty()) 
            {
                v.push_back(palavra);
                palavra = "";
            }
        } 
        else 
        {
            palavra += chr;
        }
    }
    return v;
}

int analisadorLexico(const string& str, int contadorDePalavra) 
{
    auto it = find(palavrasReservadas.begin(), palavrasReservadas.end(), str);
    if (it != palavrasReservadas.end()) 
        return distance(palavrasReservadas.begin(), it);
        
    if (isLabel(str))
        return 20;
    
    if (isNumeric(str))
        return 30;

    throw runtime_error("Erro Lexico na linha [" + to_string(contadorDeLinha) + "]: Token '" + str + "' invalido.");
}

int procuraNaTabelaDeSimbolos(const string& str)
{
    for(size_t i = 0; i < tabelaDeSimbolos.size(); ++i)
    {
        if(get<0>(tabelaDeSimbolos[i]) == str)
            return i; // Retorna o índice se encontrar
    }
    return -1; // Retorna -1 APÓS percorrer a lista inteira
}

int procuraNalistaDePendencias(const string& str)
{
    for(size_t i = 0; i < listaDePendencias.size(); ++i)
    {
        if(get<0>(listaDePendencias[i]) == str) // get<0> agora é string
            return i; // Retorna o índice se encontrar
    }
    return -1; // Retorna -1 APÓS percorrer a lista inteira
}


bool isLabel(const string& str)
{
    if (!str.empty() && (isalpha(str[0]) || str[0] == '_')) 
    {
        for (size_t i = 1; i < str.length(); ++i) {
            if (!isalnum(str[i]) && str[i] != '_') 
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool isNumeric(const string& str) 
{
    if (str.empty()) return false;
    for (char const &c : str) 
    {
        if (isdigit(c) == 0) return false;
    }
    return true;
}
