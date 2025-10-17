#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <tuple>

using namespace std;

/* =====================================
    = Declaracao de Funcoes Auxiliares =
    ====================================
*/
void leitorDeArquivo(const string& nome_do_arquivo);
void analisadorSintatico(const string& str);

vector<string> percorreLinha(const string& str);

vector<int> verificaTokens(const vector<string>& v);

int analisadorLexico(const string& str, int n);

int idxTS(const string& str);
int idxLP(const string& str);
bool isNum(const string& str);
bool isLabel(const string& str);
bool inSyntax(const vector<int>& v);

void showTabelaDeSimbolos();
void showListaDePendencias();
void showFinal();
void sequenciaFinal();

/* ======================
    = Variaveis Globais =
    =====================
*/
int contadorDeLinha = 1;
int contadorDeEndereco = 0;
int contadorDePosicao = 0;
int contadorDePalavra = 0;
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
    "",      //0
    "ADD",   //1
    "SUB",   //2
    "MULT",  //3
    "DIV",   //4
    "JMP",   //5
    "JMPN",  //6
    "JMPP",  //7
    "JMPZ",  //8
    "COPY",  //9
    "LOAD",  //10
    "STORE", //11
    "INPUT", //12
    "OUTPUT",//13
    "STOP",  //14 
    "SPACE", //15
    "CONST"  //16
};

/* ==================================
   = Lista da sintaxe do programa   =
   ==================================
*/
 vector< vector<int>> syntaxRules={
        {1, 20    },    // ADD
        {2, 20    },    // SUB 
        {3, 20    },    // MULT
        {4, 20    },    // DIV 
        {5, 20    },    // JMP 
        {6, 20    },    // JMPN 
        {7, 20    },    // JMPP 
        {8, 20    },    // JMPZ 
        {9, 20, 20},    // COPY 
        {10, 20    },   // LOAD 
        {11,20    },    // STORE
        {12,20    },    // INPUT
        {13,20    },    // OUTPUT
        {14       },    // STOP
        {16,30    },    // CONST
        {20,15    },    // LABEL SPACE 
        {20,15, 30},    // LABEL SPACE CONST
        {20,1, 20 },    // LABEL ADD
        {20,2, 20 },    // LABEL SUB 
        {20,3, 20 },    // LABEL MULT
        {20,4, 20 },    // LABEL DIV 
        {20,5, 20 },    // LABEL JMP 
        {20,6, 20 },    // LABEL JMPN 
        {20,7, 20 },    // LABEL JMPP 
        {20,8, 20 },    // LABEL JMPZ 
        {20,9, 20, 20}, // LABEL COPY 
        {20,10, 20 },   // LABEL LOAD 
        {20,11,20 },    // LABEL STORE
        {20,12,20 },    // LABEL INPUT
        {20,13,20 },    // LABEL OUTPUT
        {20,14    },    // LABEL STOP
        {20,16,30 }     // LABEL CONST
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
    
    showTabelaDeSimbolos();
    showListaDePendencias();
    showFinal();
    
    return 0;
}

void sequenciaFinal()
{
	for(tuple<string, vector<int>> t : listaDePendencias)
	{
		int pos = idxTS(get<0>(t));
		
		if (pos == -1)
			throw runtime_error("Erro Semantico rótulo não definido: " + get<0>(t));
			
		int endereco = get<1>(tabelaDeSimbolos[idxTS(get<0>(t))]);
		
		for(int i: get<1>(t))
		{
			listaDeEnderecos[i] = endereco;
		}
	}
}

void leitorDeArquivo(const string& nome_do_arquivo) 
{
    ifstream arquivo(nome_do_arquivo);
    string linha;

    if (arquivo.is_open()) 
    {
        while (getline(arquivo, linha)) 
        {
            analisadorSintatico(linha);
            contadorDeLinha++;
        }
        arquivo.close();
    } 
    else 
    {
        cerr << "Erro: Nao foi possivel abrir o arquivo '" << nome_do_arquivo << "'\n";
    }
}

void analisadorSintatico(const string& linha) 
{
    vector<string> tokens = percorreLinha(linha);
    if (tokens.empty()) return;

    vector<int> tokensVerificados = verificaTokens(tokens);
    vector<string> tokensSintaticamenteCorretos;
    
    if(inSyntax(tokensVerificados))
    {
        for(int i: tokensVerificados)
        {
            tokensSintaticamenteCorretos.push_back(to_string(i));
        }
        return;
    }
    throw runtime_error("Erro Sintatico na linha:\n[" + to_string(contadorDeLinha) + "]" + linha + "\n" );
}

/*
	Verifica se o vector de inteiros esta presente
	nas regras sintaticas.
*/
bool inSyntax(const vector<int>& v)
{
    if (v.empty()) return true;
    auto it = find(syntaxRules.begin(), syntaxRules.end(), v);
    return (it != syntaxRules.end());
}
/*
	Gera uma lista com o tipo do token
*/
vector<int> verificaTokens(const vector<string>& v)
{
	int posicaoDePalavra = 0;
    vector<int> v_t1;
    for(const string& str: v)
    {
        v_t1.push_back(analisadorLexico(str, posicaoDePalavra));
        posicaoDePalavra ++;
    }
    return v_t1;
}

vector<string> percorreLinha(const string& str)
{
    vector<string> v;
    string palavra = "";
    string linha = str + " ";
    
    for (char chr : linha) 
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

int analisadorLexico(const string& str, int posicaoDePalavra) 
{
    auto it = find(palavrasReservadas.begin(), palavrasReservadas.end(), str);
    if (it != palavrasReservadas.end())
    {
        lastToken = distance(palavrasReservadas.begin(), it);
        if(lastToken == 15 || lastToken == 16)
        {
        	// SPACE
        	if(lastToken == 15)
        	{
        		listaDeEnderecos[contadorDePosicao] = 0;
    			contadorDePosicao++;
    			contadorDePalavra ++;
    			contadorDeEndereco ++;
        	}
        	// CONST
        	if(lastToken == 16)
        	{}
        }
        else
        {
    		listaDeEnderecos[contadorDePosicao] = lastToken;
    		contadorDePosicao ++;
    		contadorDePalavra ++;
    		contadorDeEndereco ++;
    	}
    	return lastToken;
    } 
        
    if (isLabel(str))
    {
    	int t0 = idxTS(str);
    	// Achou na tabela de simbolos
    	if (t0>= 0)
    	{
    		// Se esta no inicio eh pq achou outra definicao
    		if (posicaoDePalavra == 0)
    			throw runtime_error("Erro Semantico na linha [" + to_string(contadorDeLinha) + "]: Rótulo já definido");
    		// Nao esta no inicio, pode colocar na listaDeEnderecos
    		else
    		{
    			listaDeEnderecos[contadorDePosicao] = get<1>(tabelaDeSimbolos[t0]);
    			contadorDeEndereco++;
    			contadorDePosicao++;
    			contadorDePalavra ++;
    		}
    	}
    	
    	// Não achou na tabela de simbolos
    	else
    	{
    		// Se esta no inicio eh pq esta sendo definido 
			if (posicaoDePalavra == 0)
			{
				tabelaDeSimbolos.push_back(make_tuple(str, contadorDeEndereco));
			}
			
			// Nao esta no inicio portanto precisa ser definido
			else
			{
				contadorDePalavra ++;
				int t1 = idxLP(str);
				
				// Esta na lista de pendencia, adicionando mais uma pendencia
				if (t1 >= 0)
				{
					vector<int> v = get<1>(listaDePendencias[t1]);
					v.push_back(contadorDeEndereco);
					listaDePendencias[t1] = make_tuple(str, v);
				}
				// Nao esta na lista de pendencia, criando primeira pendencia";
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
    
    if (isNum(str))
    {
    	contadorDePalavra ++;
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

int idxTS(const string& str)
{
    for(size_t i = 0; i < tabelaDeSimbolos.size(); ++i)
    {
        if(get<0>(tabelaDeSimbolos[i]) == str)
            return i;
    }
    return -1;
}

int idxLP(const string& str)
{
    for(size_t i = 0; i < listaDePendencias.size(); ++i)
    {
        if(get<0>(listaDePendencias[i]) == str)
            return i;
    }
    return -1;
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

bool isNum(const string& str) 
{
    if (str.empty()) return false;
    for (char c : str) 
    {
        if (isdigit(c) == 0) return false;
    }
    return true;
}

void showTabelaDeSimbolos()
{
	cout << "\n";
    cout << "====================\n";
    cout << "=Tabela de Símbolos=\n";
    cout << "====================\n";
    cout << "\n";
    for(tuple<string, int> t : tabelaDeSimbolos)
    {
    	cout << get<0>(t) << " (&" << get<1>(t) << ")\n"; 
    }
    cout << "\n";
}

void showListaDePendencias()
{
	cout << "=====================\n";
    cout << "=Lista de Pendencias=\n";
    cout << "=====================\n";
    cout << "\n";
    for(tuple<string, vector<int>> t : listaDePendencias)
    {
    	cout << get<0>(t) << " [ ";
    	for(int i :get<1>(t))
    	{
    		cout << i << " ";
    	}
    	cout << "]\n";
    }
    cout << "\n";
}

void showFinal()
{
	cout << "==============\n";
    cout << "=    Final   =\n";
    cout << "==============\n";
    cout << "\n";
    sequenciaFinal();
    for(int i = 0; i < contadorDePalavra; i++)
    {
    	cout << listaDeEnderecos[i] << " ";
    }
	cout << "\n";
}
