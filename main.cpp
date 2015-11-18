#include <vector> // vector
#include <string> // string
#include <iostream> // cout, cerr

#include "Netpbm_Image.hpp"

using namespace std;
using namespace ConcurrentProgramming;

/* Funcao _main:
 * Verdadeira funcao principal do codigo. A funcao "main" chamara esta funcao.
 *   que fara todo o servico do programa. Porem, diferentemente da main,
 *   os argumentos estao contidos em um vetor de strings, alem de nao incluirem
 *   o proprio nome do programa como argumento, se assemelhando a main do java
 *   e facilitando a manipulacao de tais argumentos. E como a funcao eh inline,
 *   a perda de performance eh pifia para nao dizer nula.
 * Parametros:
 *   args: argumentos da linha de comando.
 * Esta funcao nao possui retorno.
 */
inline void _main(const vector<string>& args) {
	
	// Erros podem acontecer, portanto, bloco try-catch.
	try {
		
		// Flag que indica se o codigo vai ser executado em paralelo ou nao.
		bool parallel = false;
		
		// Nome do arquivo sobre o qual aplicar-se-a o filtro.
		string file_name;
		
		// Se apenas o argumento de modo nao for especificado, assumir-se-a
		// execucao sequencial. O usuario sera notificado, no entanto.
		if (args.size() < 1u) {
			cerr << "WARNING! No mode specified. Assuming sequential." << endl;
		
		// Caso o argumento de modo seja especificado:
		} else {
			
			// Caso o argumento seja "sequential":
			if (args[0] == "sequential") {
				// A execucao nao sera em paralelo.
				parallel = false;
				
			// Caso o argumento seja "parallel":
			} else if (args[0] == "parallel") {
				// A execucao serao em paralelo.
				parallel = true;
				
			// Caso nao seja nenhum desses,
			} else {
				
				 // Lanca uma excecao de modo invalido.
				throw out_of_range("Mode is neither sequential nor parallel.");
			}
		}
		
		// Se o argumento de nome de arquivo nao for fornecido:
		if (args.size() < 2u) {
			// Solicita tal informacao ao usuario.
			cout << "Enter the smooth file: ";
			cin >> file_name;
		} else {
			// Caso contrario, atribui o nome de arquivo fornecido.
			file_name = args[1];
		}
		
		// Carrega a imagem.
		Netpbm_Image* img = new Netpbm_Image(file_name);
		
		// E aplica o devido filtro, explicitando que se deseja salvar
		// o resultado automaticamente e imprimir o tempo de execucao na
		// saida padrao.
		if (parallel) {
			img->smooth_parallel(true, true);
		} else {
			img->smooth_sequential(true, true);
		}
		
		delete img;
	
	// Pega excecoes genercias do sistema:
	} catch (exception& e) {
		
		// Infroma, na saida de erro padrao, que uma excecao ocorreu.
		cerr << "Exception ocurred:" << endl;
		
		// Informa a mensagem associada com a excecao.
		cerr << e.what() << endl;
		
	// Pega qualquer tipo de excecao nao pega:
	} catch (...) {
		
		// Informa, na saida de erro padrao, que ocorreu uma excecao
		// desconhecida; pois se fosse conhecida, teria sido pela pelo
		// catch anterior.
		cerr << "An unknown error occurred!" << endl;
		
	}
	
}
/* Funcao main:
 * Funcao principal / ponto de entrada do programa.
 * Parametros:
 *   argc: numero de argumentos da linha de comando, contando a propria
 *     invocacao do programa.
 *   argv: valor dos argumentos da linha de comando como string de C
 *     (vetor de char).
 * Retorno:
 *   status de saida do programa, retornado para o sistema operacional.
 */
int main(int argc, char* argv[]) {
	
	// Empacota os argumentos, menos a propria invocacao do programa,
	// em um vetor de strings e chama a verdadeira funcao principal, "_main".
	_main(vector<string>(argv + 1, argv + argc));
	
	// Retorna zero (encerrou com sucesso) para o sistema operacional.
	return 0;
}
