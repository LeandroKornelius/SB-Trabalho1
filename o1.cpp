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
int contadorDePosicao = 0;
int lastToken = 0;

/* ==========================
   =   Lista de Enderecos   =
   ==========================
   = número que tem em cada =
   = endereco, ou seja, o   =
   = código em sí           =
   ==========================
*/ 
vector<int> listaDeEnderecos(216, 0);

/* ====================================
   =       Lista de Pendencias        =
   ====================================
   = O tipo do identificador          =
   = do rótulo deve ser 'string'.     =
   ====================================
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
    "ADD",   //0
    "SUB",   //1
    "MULT",  //2
    "DIV",   //3
    "JMP",   //4
    "JMPN",  //5
    "JMPP",  //6
    "JMPZ",  //7
    "COPY",  //8
    "LOAD",  //9
    "STORE", //10
    "INPUT", //11
    "OUTPUT",//12
    "STOP",  //13 
    "SPACE", //14
    "CONST"  //15
};

/* ==================================
   = Lista da sintaxe do programa   =
   ==================================
*/
 vector< vector<int>> regrasSintaticas={
        {0, 20    },    // ADD
        {1, 20    },    // SUB 
        {2, 20    },    // MULT
        {3, 20    },    // DIV 
        {4, 20    },    // JMP 
        {5, 20    },    // JMPN 
        {6, 20    },    // JMPP 
        {7, 20    },    // JMPZ 
        {8, 20, 20},    // COPY 
        {9, 20    },    // LOAD 
        {10,20    },    // STORE
        {11,20    },    // INPUT
        {12,20    },    // OUTPUT
        {13       },    // STOP
        {15,30    },    // CONST
        {20,14    },    // LABEL SPACE 
        {20,14, 30},    // LABEL SPACE CONST
        {20,0, 20 },    // LABEL ADD
        {20,1, 20 },    // LABEL SUB 
        {20,2, 20 },    // LABEL MULT
        {20,3, 20 },    // LABEL DIV 
        {20,4, 20 },    // LABEL JMP 
        {20,5, 20 },    // LABEL JMPN 
        {20,6, 20 },    // LABEL JMPP 
        {20,7, 20 },    // LABEL JMPZ 
        {20,8, 20, 20}, // LABEL COPY 
        {20,9, 20 },    // LABEL LOAD 
        {20,10,20 },    // LABEL STORE
        {20,11,20 },    // LABEL INPUT
        {20,12,20 },    // LABEL OUTPUT
        {20,13    },    // LABEL STOP
        {20,15,30 }     // LABEL CONST
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
    cout << "=====================\n";
    cout << "Tabela de Símbolos\n";
    for(tuple<string, int> t : tabelaDeSimbolos)
    {
    	cout << get<0>(t) << " (&" << get<1>(t) << ")\n"; 
    }
    cout << "\n";
    cout << "=====================\n";
    cout << "Lista de Pendencias\n";
    for(tuple<string, vector<int>> t : listaDePendencias)
    {
    	cout << get<0>(t) << " [ ";
    	for(int i :get<1>(t))
    	{
    		cout << i << " ";
    	}
    	cout << "]\n";
    }
    cout << "Tabela Final\n";
    for(int i: listaDeEnderecos)
    {
    	cout << i << " ";
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
            vector<string> v_linha2 = percorreLinha(linha);
            // Lógica de impressão (para depuração)
            
            if (!v_linha2.empty()) {
                cout << "[" << contadorDeLinha << "] ";
                for(const string& str : v_linha2) 
                {
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
    vector<int> v_t1;
    for(const string& str: v)
    {
        v_t1.push_back(analisadorLexico(str, contadorDePalavra));
        contadorDePalavra ++;
    }
    //cout << "\n";
    return v_t1;
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
    {
        lastToken = distance(palavrasReservadas.begin(), it);
        //cout << "\n" << str << "(&" << contadorDeEndereco << ") " << contadorDePosicao << " " << lastToken << "\n";
        //SPACE
        if(lastToken == 14 || lastToken == 15)
        {
        	//SPACE
        	if(lastToken == 14)
        	{
        		listaDeEnderecos[contadorDePosicao] = 0;
    			contadorDePosicao++;
        	}
        	if(lastToken == 15)
        	{}
        }
        else
        {
    		listaDeEnderecos[contadorDePosicao] = (lastToken + 1);
    		contadorDePosicao ++;
    	}
    	contadorDeEndereco ++;
    	return lastToken;
    } 
        
    if (isLabel(str))
    {
    	int t0 = procuraNaTabelaDeSimbolos(str);
    	// Achou na tabela de simbolos
    	if (t0>= 0)
    	{
    		// Se esta no inicio é pq achou outra definicao
    		if (contadorDePalavra == 0)
    			throw runtime_error("Erro Semantico na linha [" + to_string(contadorDeLinha) + "]: Rótulo já definido");
    		// Primeira definicao da label
    		else
    		{
    			//cout << "****" << get<1>(tabelaDeSimbolos[t0]) << "****";
    			//listaDeEnderecos[contadorDePosicao] = get<1>(tabelaDeSimbolos[t0]);
    			contadorDeEndereco++;
    			contadorDePosicao++;
    			//cout << "Já definido "  << str << "\n";
    		}
    	}
    	// Não achou na tabela de simbolos
    	else
    	{
    	// Esta no inicio 
    	if (contadorDePalavra == 0)
    	{
    		//É rótulo adicionando na tabela de simbolos...\n";
    		tabelaDeSimbolos.push_back(make_tuple(str, contadorDeEndereco));
    	}
    	// Não esta no inicio
    	else
    	{
    		int t1 = procuraNalistaDePendencias(str);
    		//já foi inserido na lista adicionando mais uma pendencia...\n";
    		if (t1 >= 0)
    		{
    			vector<int> v = get<1>(listaDePendencias[t1]);
    			v.push_back(contadorDeEndereco);
    			listaDePendencias[t1] = make_tuple(str, v);
    		}
    		//primeira ocorrência e não foi definido adicionando na lista de pendencias...\n";
    		else
    		{
    			vector<int> v = {contadorDeEndereco};
    		    listaDePendencias.push_back(make_tuple(str, v));
    		}
    		contadorDeEndereco++;
    		contadorDePosicao++;
    	}
    	}
    	
    	return 20;	
    }
    
    if (isNumeric(str));
    {
    	if(lastToken == 14)
    	{
   			contadorDeEndereco += stoi(str);
   			contadorDePosicao++;
   		}
   		else
   		{
   			listaDeEnderecos[contadorDePosicao] = stoi(str);
   			contadorDeEndereco ++;
   			contadorDePosicao++;
   		}
   		return 30;
    }

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
