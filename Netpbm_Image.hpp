/**
 * Universidade de Sao Paulo - USP
 * Instituto de Ciencias Matematicas e Computacao - ICMC
 * Departamento de Sistemas de Computacao - SSC
 * Programacao Concorrente - SSC0143
 * Professor Doutor Julio Cezar Estrella
 * 
 * Nomes / Nos. USP:
 *   Loys Henrique Sacomano Gibertoni - 8532377
 *   Sady Sell Neto - 8532418
 * Data: 15 de novembro de 2015
 * 
 * Copyright (C) 2015 Loys Henrique Sacomano Gibertoni, Sady Sell Neto
 */

/**
 * Netpbm_Image.hpp:
 * Biblioteca para manipulacao de imagens .pgm e .ppm e para aplicacao
 * do estencil smooth.
 */

// Impede que o arquivo seja incluido mais de uma vez.
// (Equivalente a #ifndef ... #define ... #endif, porem mais poderoso).
#pragma once

#include <vector> // vector
#include <tuple> // tuple
#include <chrono> // system_clock, steady_clock, duration, time_point
#include <stdexcept> // invalid_argument
#include <fstream> // ifstream, ofstream
#include <iostream> // istream, cout, cerr
#include <mpi.h>

// Usando namespaces padrao, para evitar qualificao completa de nomes.
using namespace std;
using namespace std::chrono;

namespace ConcurrentProgramming {
	
	// O "using" eh a alernativa do C++ ao typedef do C.
	
	// Alias para duracoes com representacao double, para facilitar
	// a codificao e compreensao do codigo.
	template<class period>
	using double_duration = duration<double, period>;
	
	// Alias para duracoes mais comuns, com representacao double.
	using double_hours = double_duration<hours::period>;
	using double_minutes = double_duration<minutes::period>;
	using double_seconds = double_duration<seconds::period>;
	using double_milliseconds = double_duration<milliseconds::period>;
	using double_microseconds = double_duration<microseconds::period>;
	using double_nanoseconds = double_duration<nanoseconds::period>;
	
	// Alias para duracao do clock com representacao double.
	using double_clock_duration = double_duration<steady_clock::duration::period>;
	
	// Alias para tipos sem sinal, para ficar mais facil referncia-los.
	using byte = unsigned char;
	using word = unsigned int;
	using dword = unsigned long int;
	
	/* Funcao extract_name:
	 * Extrai o nome cru (sem extensao) de um arquivo dado seu nome completo
	 *   (com extensao).
	 * Parametros:
	 *   file_name: arquivo cujo nome cru sera extraido.
	 * Retorno:
	 *    nome cru extraido.
	 */
	inline string extract_name(const string& file_name) {
		// Retorna uma substring
		// a partir do indice 0 da string original
		// ateh o indice do ultimo ponto (delimitador da extensao).
		return file_name.substr(0, file_name.find_last_of("."));
	}
	
	/* Classe Netpbm_Image:
	 * Armazena o conteudo de uma image .pgm ou .ppm como vetor de bytes,
	 * e expoe metodos para realizacao do estencil smooth.
	 */
	class Netpbm_Image {
		
	private:
		
		/* Constantes de classe:
		 * windows_radius: raio da janela do smooth, isto e,
		 *   a distancia do centro da imagem ate a borda.
		 *   Para o exercicio proposto, windows_radius eh 2.
		 * window_size: tamanho de um lado da janela (quadrada).
		 * window_area: area da janela.
		 */
		static const word windows_radius = 2;
		static const word window_size = 2 * windows_radius + 1;
		static const word window_area = window_size * window_size;
		
		/* Campos:
		 * width: largura da imagem.
		 * height: altura da imagem.
		 * dimension: numero de cores da imagem;
		 *   1, no caso de .pgm (pois eh escala de cinza);
		 *   3, no caso de .ppm (pois tem tres cores: R, G e B).
		 * max_value: valor de intensidade maxima no arquivo.
		 * file_name: nome do arquivo para o qual se realiza entrada/saida.
		 * data: conteudo da imagem, como vetor de bytes.
		 */
		
		dword width;
		dword height;
		byte dimension;
		byte max_value;
		
		string file_name;
		
		vector<byte> data;
		
	private:
		
		/* Metodo (auxiliar) estatico ignore_blanks:
		 * Ignora sequencia de caracteres brancos da entrada, deixando
		 *   a stream de entrada no proximo caracter valido.
		 * Parametros:
		 *   source: stream de entrada a ter brancos lidos.
		 * Este metodo nao possui retorno.
		 */
		inline static void ignore_blanks(istream& source) {
			
			// Caractere auxilar.
			char c;
			do {
				// Le da stream no caractere auxiliar, ate ele nao ser
				// branco.
				c = source.get();
			} while (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f');
			
			// Pelo metodo proposto, sera lido um caractere nao-branco,
			// e ele se "perdera" da stream. Portanto, deve-se devolver
			// este caractere na stream.
			source.unget();
			
		}

		/* Metodo (auxiliar) estatico read_next:
		 * Le a proxima informacao relevante do cabecalho de um arquivo
		 *   .pgm ou .ppm, ignorando brancos e comentarios.
		 * Parametros:
		 *   source: stream de entrada a ter o conteudo lido.
		 * Retorno:
		 *   conteudo lido como string.
		 */
		inline static string read_next(istream& source) {
			
			// Le e descarta caracteres brancos.
			ignore_blanks(source);
			
			// Proximo conteudo a ser lido.
			string token;
			
			do {
				
				// Le o proximo conteudo valido do arquivo.
				source >> token;
				
				// Se comecar com '#' eh porque eh comentario.
				// Portanto descarta o conteudo da stream ate o
				// o proximo '\n', com um numero maximo de 70
				// caracteres, de acordo com a especificao
				// destes tipos de cabecalhos.
				if (token[0] == '#') {
					source.ignore(70, '\n');
				}
			
			// Repete a leitura enquanto um comentario for lido,
			// assim descartando os comentarios.
			} while (token[0] == '#');
			
			// Retorna o conteudo lido.
			return token;
			
		}
		
		/* Metodo read_header:
		 * Le os dados signifcativos (largura, altura, valor maximo de cor)
		 *   do cabecalho dos arquivos .pgm e .ppm e posiciona o cursor de
		 *   leitura para a leitura dos dos efetivos.
		 * Parametros:
		 *   source: stream de entrada a ter brancos lidos.
		 * Este metodo nao possui retorno.
		 */
		inline void read_header(istream& source) {
			
			// Le o primeiro dado signifcativo: a largura.
			this->width = stoi(read_next(source));
			
			// Le o proximo dado signifcativo: a altura.
			this->height = stoi(read_next(source));
			
			// Le o proximo dado signifcativo: o valor maximo de cor.
			this->max_value = stoi(read_next(source));
			
			// Ignora caracteres brancos, deixando a stream pronta para
			// ler dados.
			ignore_blanks(source);
			
		}
		
		/* Metodo binary_reopen (sobrecarregado):
		 * Reabre um arquivo de leitura em modo binario, recuperando a
		 *   posicao que estava antes desta operacao
		 * Parametros:
		 *   f: arquivo a ser reaberto.
		 * Este metodo nao possui retorno.
		 */
		inline void binary_reopen(ifstream& f) const {
			
			// Obtem a posicao de leitura atual do arquivo.
			ifstream::pos_type pos = f.tellg();
			
			// Fecha o arquivo, e o reabre, para leitura em modo binario.
			f.close();
			f.open(this->file_name, ifstream::in | ifstream::binary);
			
			// Retorna a posicao de leitura para a posicao que estava.
			f.seekg(pos);
			
		}
		
		/* Metodo binary_reopen (sobrecarregado):
		 * Reabre um arquivo de escrita em modo binario, recuperando a
		 *   posicao que estava antes desta operacao
		 * Parametros:
		 *   f: arquivo a ser reaberto.
		 * Este metodo nao possui retorno.
		 */
		inline void binary_reopen(ofstream& f) const {
			
			// Obtem a posicao de escrita atual do arquivo.
			ofstream::pos_type pos = f.tellp();
			
			// Fecha o arquivo, e o reabre, para escrita em modo binario,
			// e anexando suas novas escritas ao fim do arquivo, assim
			// evitando que se perca o arquivo atual.
			f.close();
			f.open(this->file_name, ofstream::out | ofstream::app | ofstream::binary);
			
			// Retorna a posicao de escrita para a posicao que estava.
			f.seekp(pos);
			
		}
		
		/* Metodo map_position_2d (sobrecarregado):
		 * Mapeia posicoes x e y / linha e coluna em uma posicao unica e
		 *   absoluta.
		 * Parametros:
		 *   row: linha / posicao x.
		 *   col: coluna / posicao y.
		 * Retorno:
		 *   posicao absoluta.
		 */
		inline dword map_position_2d(dword row, dword col) const {
			return row * this->width + col;
		}
		
		/* Metodo map_position_2d (sobrecarregado):
		 * Mapeia uma posicao unica e absoluta em duas posicoes:
		 *   x e y.
		 * Parametros:
		 *   pos: posicao absoluta;
		 * Retorno:
		 *   tupla com dois inteiros: posicoes x e y, respectivamente.
		 */
		inline tuple<dword, dword> map_position_2d(dword pos) const {
			return tuple<dword, dword>(pos / this->width, pos % this->width);
		}
		
		/* Metodo map_position_3d (sobrecarregado):
		 * Mapeia posicoes x, y e z / linha, coluna  e cor em uma posicao
		 *   unica e absoluta.
		 * Parametros:
		 *   row: linha / posicao x.
		 *   col: coluna / posicao y.
		 *   color: cor / posicao z.
		 * Retorno:
		 *   posicao absoluta.
		 */
		inline dword map_position_3d(dword row, dword col, byte color) const {
			return row * this->width * this->dimension + col * this->dimension + color;
		}
		
		/* Metodo map_position_2d (sobrecarregado):
		 * Mapeia uma posicao unica e absoluta em tres posicoes:
		 *   x, y e z.
		 * Parametros:
		 *   pos: posicao absoluta;
		 * Retorno:
		 *   tupla com dois inteiros: posicoes x, y e z, respectivamente.
		 */
		inline tuple<dword, dword, byte> map_position_3d(dword pos) const {
			return tuple<dword, dword, dword>(
				pos / this->dimension / this->width,
				pos / this->dimension % this->width,
				pos % this->dimension
			);
		}
		
		/* Metodo map_access (sobrecarregado):
		 * Acessa um vetor 1d via mapeamento de acesso, como se o acesso
		 *   fosse realizado em um vetor 3d, ou seja, fornecendo tres
		 *   indices.
		 * Parmetros:
		 *   row: linha / primeiro indice do suposto vetor 3d.
		 *   col: coluna / segundo indice do suposto vetor 3d.
		 *   color: cor / terceiro indice do suposto vetor 3d.
		 * Retorno: 
		 *   referencia para o elemento lido no vetor.
		 *   (por ser referencia, permite leitura e escrita nessa posicao).
		 */
		inline byte& map_access(dword row, dword col, byte color) {
			return this->data[this->map_position_3d(row, col, color)];
		}
		
		/* Metodo map_access (sobrecarregado):
		 * Acessa um vetor 1d via mapeamento de acesso, como se o acesso
		 *   fosse realizado em um vetor 3d, ou seja, fornecendo tres
		 *   indices.
		 * Parmetros:
		 *   row: linha / primeiro indice do suposto vetor 3d.
		 *   col: coluna / segundo indice do suposto vetor 3d.
		 *   color: cor / terceiro indice do suposto vetor 3d.
		 * Retorno: 
		 *   referencia constante para o elemento lido no vetor.
		 *   (por ser referencia constante, permite leitura nessa posicao).
		 */
		inline const byte& map_access(dword row, dword col, byte color) const {
			return this->data[this->map_position_3d(row, col, color)];
		}
		
		/* Metodo ASCII_PGM:
		 * Realiza a leitura do conteudo de um arquivo P2, isto eh,
		 *   .pgm em formato texto.
		 * Parametros:
		 *   source: arquivo de leitura do qual os dados serao lidos.
		 * Este metodo nao possui retorno.
		 */
		void ASCII_PGM(ifstream& source) {
			
			// Arquivos .pgm tem apenas uma dimensao de cor (cinza).
			this->dimension = 1;
			
			// Valor textual a ser lido.
			string value;
			
			// Percorre altura * largura (tamanho da imagem) bytes.
			for (dword i = 0; i < this->height * this->width; i++) {
				
				// Le os bytes e armazena-os no vetor.
				source >> value;
				this->data.push_back(stoi(value));
				
			}
			
		}
		
		/* Metodo ASCII_PPM:
		 * Realiza a leitura do conteudo de um arquivo P3, isto eh,
		 *   .ppm em formato texto.
		 * Parametros:
		 *   source: arquivo de leitura do qual os dados serao lidos.
		 * Este metodo nao possui retorno.
		 */
		void ASCII_PPM(ifstream& source) {
			
			// Arquivos .ppm tem tres dimensoes de cores (R, G e B).
			this->dimension = 3;
			
			// Valor textual a ser lido.
			string value;
			
			// Percorre altura * largura (tamanho da imagem) dados.
			for (dword i = 0; i < this->height * this->width; i++) {
				
				// Le o valor R e armazena-o no vetor.
				source >> value;
				this->data.push_back(stoi(value));
				
				// Le o valor G e armazena-o no vetor.
				source >> value;
				this->data.push_back(stoi(value));
				
				// Le o valor B e armazena-o no vetor.
				source >> value;
				this->data.push_back(stoi(value));
				
			}
			
		}
		
		/* Metodo ASCII_PGM:
		 * Realiza a leitura do conteudo de um arquivo P5, isto eh,
		 *   .pgm em formato binario.
		 * Parametros:
		 *   source: arquivo de leitura do qual os dados serao lidos.
		 * Este metodo nao possui retorno.
		 */
		void binary_PGM(ifstream& source) {
			
			// Arquivos .pgm tem apenas uma dimensao de cor (cinza).
			this->dimension = 1;
			
			// Reabre o arquivo em modo binario, ja que seu conteudo
			// sera binario deste ponto em diante.
			this->binary_reopen(source);
			
			// Redimensiona o vetor para caber os dados a serem lidos.
			this->data.resize(this->height * this->width);
			
			// Le no vetor altura * largura (tamanho da imagem) bytes.
			source.read((char*)this->data.data(), this->height * this->width * sizeof(byte));
			
		}
		
		/* Metodo ASCII_PPM:
		 * Realiza a leitura do conteudo de um arquivo P6, isto eh,
		 *   .ppm em formato binario.
		 * Parametros:
		 *   source: arquivo de leitura do qual os dados serao lidos.
		 * Este metodo nao possui retorno.
		 */
		void binary_PPM(ifstream& source) {
			
			// Arquivos .ppm tem tres dimensoes de cores (R, G e B).
			this->dimension = 3;
			
			// Reabre o arquivo em modo binario, ja que seu conteudo
			// sera binario deste ponto em diante.
			this->binary_reopen(source);
			
			// Redimensiona o vetor para caber os dados a serem lidos.
			// Deve ter tres vezes o tamanho da imagem (altura * largura)
			// pois cada pixel (cujo total eh altura * largura) serao lidos
			// tres dados: o valor R, o valor G e o valor B.
			this->data.resize(3 * this->height * this->width);
			
			// Le no vetor 3 (R, G e B) * altura * largura (tamanho da
			// imagem) bytes.
			source.read((char*)this->data.data(), 3 * this->height * this->width * sizeof(byte));
			
		}
		
		/* Metodo smooth (sobrecarregado):
		 * Aplica o estencil smooth sobre a imagem, podendo ser feito de 
		 *   forma sequencial ou paralela (OpenMP).
		 * Parmetros:
		 *   parallel: Flag que indica se o estencil deve ser aplicado
		 *   de forma paralela ou não.
		 * Retorno: 
		 *   Ponteiro para a imagem resultante.
		 */
		inline Netpbm_Image* smooth(bool parallel) {
			
			// Cria uma nova imagem com os mesmos parâmetros da original;
			Netpbm_Image* smoothed = new Netpbm_Image(this->width, this->height, this->max_value, this->dimension);
			
			// Calcula o número de pixels a serem calculados pelo algoritmo;
			dword size = this->height * this->width;
			
			// Diretiva omp parallel for, já que este FOR pode ser executado
			// 	em pararelo, possuindo as seguintes cáusulas:
			// default(none): por padrao declara como erro o uso de variaveis externas;
			// shared: variaveis ja declaradas que serao compartilhadas por todas as threads;
			// if: condicao para que a diretiva seja utilizada, em outras palavras, o for só
			//  será executado em paralelo se a flag estiver setada.
			#pragma omp parallel for default(none) shared(smoothed, size) if(parallel)
			// Laco para percorrer todos os pixels da imagem;
			for (dword i = 0; i < size; i++) { 
				
				byte count = 0; // Contagem de pixels utilizados na media;
				vector<word> sums(this->dimension, 0); // Vetor das somas dos valores das cores de cada pixel;
				
				// Transforma o i, que eh a posicao de um valor de um pixel em uma tupla
				// que contem a linha e a coluna do pixel ao qual o valor pertence;
				tuple<dword, dword> pos = this->map_position_2d(i);
				long int row = get<0>(pos);
				long int col = get<1>(pos);
				
				// Lacos que percorrem o quadrado 5x5 cuja media sera utilizada como valor do pixel;
				for (long int x = row - 2; x <= row + 2; x++) {
					for (long int y = col - 2; y <= col + 2; y++) {
						
						// Se o pixel atual do quadrado estiver dentro da area da imagem;
						if (x >= 0 && x < this->height && y >= 0 && y < this->width) {
							
							// Eele deve ser considerado no calculo da media,
							// Entao o contador eh incrementado
							count++;
							// E o valor de cada cor do pixel eh somado a sua respectiva soma;
							for (byte k = 0; k < this->dimension; k++) {
								sums[k] += this->map_access(x, y, k);
							}
							
						}
						
					}
				}
				
				// Para cada cor do pixel atual, escreve a media calculada da cor correspondente;
				for (byte k = 0; k < this->dimension; k++) {
					smoothed->map_access(row, col, k) = sums[k] / count;
				}
			}
			
			// Retorna a imagem resultante.
			return smoothed;
			
		}
		
		/* Metodo smooth (sobrecarregado):
		 * Aplica o estencil smooth sobre uma parte da imagem, de forma paralela.
		 * Parmetros:
		 *   first: primeira linha de interesse a ser calculada (inclusive);
		 * 	 last: primeira linha fora do interesse (exclusive).
		 * Retorno: 
		 *   Ponteiro para a imagem resultante.
		 */
		inline Netpbm_Image* smooth(dword first, dword last) {
			
			// Cria uma nova imagem com os mesmos parâmetros da original;
			Netpbm_Image* smoothed = new Netpbm_Image(this->width, last - first, this->max_value, this->dimension);
			
			// Calcula o número de pixels a serem calculados pelo algoritmo;
			dword size = this->height * this->width;
			
			// Calcula as posicoes de interesse;
			dword first_pos = this->map_position_2d(first, 0);
			dword last_pos = this->map_position_2d(last, 0);
			
			if (first_pos >= last_pos) {
				delete smoothed;
				return nullptr;
			}
			
			// Diretiva omp parallel for, já que este FOR pode ser executado
			// 	em pararelo, possuindo as seguintes cáusulas:
			// default(none): por padrao declara como erro o uso de variaveis externas;
			// shared: variaveis ja declaradas que serao compartilhadas por todas as threads.
			#pragma omp parallel for default(none) shared(smoothed, size, first, first_pos, last_pos)
			// Laco para percorrer todos os pixels da imagem;
			for (dword i = first_pos; i < last_pos; i++) {
				
				byte count = 0; // Contagem de pixels utilizados na media;
				vector<word> sums(this->dimension, 0); // Vetor das somas dos valores das cores de cada pixel;
				
				// Transforma o i, que eh a posicao de um valor de um pixel em uma tupla
				// que contem a linha e a coluna do pixel ao qual o valor pertence;
				tuple<dword, dword> pos = this->map_position_2d(i);
				long int row = get<0>(pos);
				long int col = get<1>(pos);
				
				// Lacos que percorrem o quadrado 5x5 cuja media sera utilizada como valor do pixel;
				for (long int x = row - 2; x <= row + 2; x++) {
					for (long int y = col - 2; y <= col + 2; y++) {
						
						// Se o pixel atual do quadrado estiver dentro da area da imagem;
						if (x >= 0 && x < this->height && y >= 0 && y < this->width) {
							
							// Eele deve ser considerado no calculo da media,
							// Entao o contador eh incrementado
							count++;
							// E o valor de cada cor do pixel eh somado a sua respectiva soma;
							for (byte k = 0; k < this->dimension; k++) {
								sums[k] += this->map_access(x, y, k);
							}
							
						}
						
					}
				}
				
				// Para cada cor do pixel atual, escreve a media calculada da cor correspondente;
				for (byte k = 0; k < this->dimension; k++) {
					smoothed->data[this->map_position_3d(row, col, k) - first * this->width * this->dimension] = sums[k] / count;
				}
			}
			
			// Retorna a imagem resultante.
			return smoothed;
			
		}
		
		/* Construtor (sobrecarregado):
		 * Constroi um novo objeto com dimensoes suficientes para comportar
		 *   uma certa demanda de dados, porem com vetor nao-inicializado.
		 * Parametros:
		 *   width: largura do objeto;
		 *   height: altura do objeto;
		 *   max_value: valor maximo de cor do objeto;
		 *   dimension: dimensao (numero de cores) do objeto;
		 */
		Netpbm_Image(dword width, dword height, byte max_value, byte dimension) {
			
			// Atribui os valores obtidos por parametro.
			this->width = width;
			this->height = height;
			this->dimension = dimension;
			this->max_value = max_value;
			
			// Inicializa o nome do arquivo como invalido.
			this->file_name = "";
			
			// Redimensiona o vetor para que ele comporte a demanda
			// de dados exigida.
			this->data.resize(height * width * dimension);
			
		}
		
	public:
		
		/* Construtor (sobrecarregado):
		 * Constroi um novo objeto com seus dados obtidos a partir
		 *   de um arquivo.
		 * Parametros:
		 *   file_name: nome do arquivo cujos dados serao lidos.
		 */
		Netpbm_Image(const string& file_name) {
			
			// Atribui o nome de arquivo obtido.
			this->file_name = file_name;
			
			// Cria uma stream de leitura para este arquivo.
			ifstream reader(file_name);
			
			// Le o tipo de arquivo.
			string file_type;
			reader >> file_type;
			
			// Le os dados essenciais do cabecalho do arquivo.
			read_header(reader);
			
			// Le o conteudo de imagem do arquivo, de acordo
			// com o tipo lido, ou lanca eventuais erros
			// de tipo invalido.
			if (file_type == "P2") {
				this->ASCII_PGM(reader);
			} else if (file_type == "P3") {
				this->ASCII_PPM(reader);
			} else if (file_type == "P5") {
				this->binary_PGM(reader);
			} else if (file_type == "P6") {
				this->binary_PPM(reader);
			} else {
				reader.close();
				throw invalid_argument("Not a P2 or P3 or P5 or P6 netpbm image.");
			}
			
		}
		
		/* Metodo (getter) get_width:
		 * Recupera a largura da imagem.
		 * Este metodo nao possui parametros
		 * Retorno:
		 *   largura recuperada.
		 */
		dword get_width() const {
			return this->width;
		}
		
		/* Metodo (getter) get_height:
		 * Recupera a altura da imagem.
		 * Este metodo nao possui parametros
		 * Retorno:
		 *   alutra recuperada.
		 */
		dword get_height() const {
			return this->height;
		}
		
		/* Metodo (getter) get_height:
		 * Recupera o valor maximo de cor da imagem.
		 * Este metodo nao possui parametros
		 * Retorno:
		 *   valor maximo de cor recuperado.
		 */
		byte get_max_value() const {
			return this->max_value;
		}
		
		/* Metodo file_type:
		 * Recupera o tipo de arquivo (pgm ou ppm)
		 * Este metodo nao possui parametros.
		 * Retorno:
		 *   tipo de arquivo.
		 */
		string file_type() const {
			return this->dimension == 1 ? "pgm" : "ppm";
		}
		
		/* Metodo smooth_sequential:
		 * Aplica o estencil smooth sobre a imagem de forma sequencial, 
		 *   podendo escrever em disco o resultado e imprimir em tela o tempo de execucao.
		 * Parmetros:
		 *   will_save: Flag que indica se o resultado deve ser escrito em disco;
		 *   echo_time: Flag que indica se o tempo de execucao do filtro deve ser impresso
		 * Retorno: 
		 *   Ponteiro para a imagem resultante.
		 */
		Netpbm_Image* smooth_sequential(bool will_save = false, bool echo_time = false) {
			
			// Armazena o tempo de inicio do algoritmo;
			system_clock::time_point start = system_clock::now();
			
			// Aplica o algoritmo de smooth;
			Netpbm_Image* smoothed = this->smooth(false);
			
			// Armazena o tempo de termino do algoritmo e calcula o tempo de execucao;
			system_clock::time_point end = system_clock::now();
			double_seconds ellapsed_time = end - start;
			
			// Escreve em disco a imagem se a flag estiveer setada;
			if (will_save) {
				smoothed->save(extract_name(this->file_name) + "_result_seq." + this->file_type());
			}
			
			// Imprime em tela o tempo de execucao do algoritmo se a flag estiver setada;
			if (echo_time) {
				cout << ellapsed_time.count() << 's' << endl;
			}
			
			// Retorna a imagem gerada.
			return smoothed;
			
		}
		
		/* Metodo smooth_parallel:
		 * Aplica o estencil smooth sobre a imagem de forma paralela, 
		 *   podendo escrever em disco o resultado e imprimir em tela o tempo de execucao.
		 * Parmetros:
		 *   will_save: Flag que indica se o resultado deve ser escrito em disco;
		 *   echo_time: Flag que indica se o tempo de execucao do filtro deve ser impresso
		 * Retorno: 
		 *   Ponteiro para a imagem resultante.
		 */
		Netpbm_Image* smooth_parallel(bool will_save = false, bool echo_time = false) {
			
			// Armazena o tempo de inicio do algoritmo;
			system_clock::time_point start = system_clock::now();
			
			int processors; // Numero de processos usados pelo MPI;
			word payload; // Numero de linhas da imagem a serem processadas por cada processo;
			
			// Cria uma imegm que aramazenara o resultado da aplicacao do estencil,
			// Com os mesmos parametros da original;
			Netpbm_Image* smoothed = new Netpbm_Image(this->width, this->height, this->max_value, this->dimension);
			
			// Inicializa o ambiente MPI;
			MPI_Init(nullptr, nullptr);
			
			int rank; // Identificador do processo;
			
			MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtem o identificar do processo atual; 
			MPI_Comm_size(MPI_COMM_WORLD, &processors); // Obtem o numero toral de processos;
			payload = this->height / processors; // Calcula a carga de cada processo;
			
			// Cria uma imagem escrava para o processo atual realizar sua parte do algoritmo;
			Netpbm_Image* slave = new Netpbm_Image(this->width, payload, this->max_value, this->dimension);
			
			// Distribui os dados da imagem entre os processos;
			MPI_Scatter(this->data.data(), payload * this->width * this->dimension, MPI_UNSIGNED_CHAR, slave->data.data(), payload * this->width * this->dimension, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
			
			// Resultado do smooth escravo;
			Netpbm_Image* slave2 = slave->smooth(true);
			delete slave;
			
			// Calcula o smooth no restante (que nao foi enviado via scatter) da imagem;
			if (rank == 0) {
				// Resultado do smooth dessa parte "restante";
				Netpbm_Image* other_slave = this->smooth(payload * processors, this->height);
				
				if (other_slave != nullptr) {

					word first_pos = map_position_3d(payload * processors, 0, 0);
					word last_pos = map_position_3d(this->height, 0, 0);
					
					// Substitui na imagem final o resultado;
					for (long int i = first_pos; i < last_pos; i++) {
						smoothed->data[i] = other_slave->data[i - first_pos];
					}
					
					delete other_slave;

				}
				
			}			
			// Reune os resultados dos processos;
			MPI_Gather(slave2->data.data(), payload * this->width * this->dimension, MPI_UNSIGNED_CHAR, smoothed->data.data(), payload * this->width * this->dimension, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
			
			delete slave2;
			
			// Finaliza o ambiente MPI;
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Finalize();
			
			// Armazena o tempo de termino do algoritmo e calcula o tempo de execucao;
			system_clock::time_point end = system_clock::now();
			double_seconds ellapsed_time = end - start;
			
			if (rank == 0) {
				
				// Escreve em disco a imagem se a flag estiveer setada;
				if (will_save) {
					smoothed->save(extract_name(this->file_name) + "_result_par." + this->file_type());
				}
			
				// Imprime em tela o tempo de execucao do algoritmo se a flag estiver setada;
				if (echo_time) {
					cout << ellapsed_time.count() << 's' << endl;
				}
				
			}
			
			// Retorna a imagem gerada.
			return smoothed;
			
		}
		
		/* Metodo save:
		 * Salva os conteudos da imagem em um arquivo.
		 *   Por decisao de projeto, eles serao salvos em modo binario.
		 * Parametros:
		 *   file_name: nome do arquivo para o qual os dados serao salvos.
		 * Este metodo nao possui retorno.
		 */
		void save(const string& file_name) {
			
			// Atribui o nome deste novo arquivo.
			this->file_name = file_name;
			
			// Cria uma stream de escrita para este arquivo.
			ofstream writer(file_name);
			
			// Salva o tipo de imagem (P5 para .pgm e P6 para .ppm)
			// junto com os dados essenciais (largura, altura, valor maximo
			// de cor).
			writer << (this->dimension == 1 ? "P5" : "P6") << endl;
			writer << this->width << ' ' << this->height << ' ' << +this->max_value << endl;
			
			// Reabre o arquivo em modo binario.
			binary_reopen(writer);
			
			// Escreve o altura * largura (tamanho da imagem) pixels, cada
			// um com dimensao dados, assim escrevendo
			// altura * largura * dimensao bytes.
			writer.write((char*)this->data.data(), this->height * this->width * this->dimension * sizeof(byte));
			
		}
		
	};
	
}
