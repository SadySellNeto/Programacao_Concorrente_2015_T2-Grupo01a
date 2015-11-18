===============================================================================
Universidade de Sao Paulo - USP
Instituto de Ciencias Matematicas e Computacao - ICMC
Departamento de Sistemas de Computacao - SSC
Professor Doutor Julio Cezar Estrella

Autoria (Nome - No. USP):
Loys Henrique Sacomano Gibertoni - 8532377
Sady Sell Neto - 8532418

Copyright (C) 2015 Loys Henrique Sacomano Gibertoni, Sady Sell Neto
-------------------------------------------------------------------------------

1. Como compilar:

O .zip possui um makefile simples, mas operativo. Portanto basta usar o
utilit�rio make para compilar. Em particular usar o comando do linux:
make
compilar� o c�digo.

Caso prefira compilar manualmente, use o seguinte comando no linux:
mpic++ -std=c++11 main.cpp -o smooth -w -fopenmp -O3 -march=native

E ser� criado o binario "smooth" para rodar o programa.
-------------------------------------------------------------------------------

2. Como executar:

� fortemente sugerido que se execute o modo sequencial via:
make run-sequential
e o modo paralelo via:
make run-parallel

Mas segue as instru��es abaixo, caso queira-se rodar manualmente.s

Para executar o sequencial a partir de onde ele est�, use:
./smooth sequential File

Onde File � o arquivo sobre o qual deseja-se aplicar o filtro. Este par�metro
pode ser omitido, que far� com que o programa pergunte ao usu�rio qual deve
ser o arquivo.

Para executa-lo de modo paralelo, use:
mpirun -np 16 --hostfile hosts.txt ./smooth parallel File

Onde File � exatamente a mesma coisa do que o parametro File da execu��o
sequencial.

-------------------------------------------------------------------------------

3. Altera��o do c�digo para altera��o do ponto de vista dos resultados:

� poss�vel alterar o c�digo para que se exiba os tempos em unidades diferentes.
(Por padr�o, o programa exibe os tempos de execu��o em segundos).

Para isso, basta alterar as linhas do arquivo "Netpbm_Image.hpp":
716:	double_seconds ellapsed_time = end - start;
804:	double_seconds ellapsed_time = end - start;

O tipo da vari�vel ("double_seconds", no caso acima) pode ser alterado para mudar a
unidade de tempo. Suas possibilidades s�o:

double_hours: horas;
double_minutes: minutos;
double_seconds: segundos (padr�o);
double_milliseconds: milissegundos;
double_microseconds: microssegundos;
double_nanoseconds: nanossegundos.
-------------------------------------------------------------------------------

4. Agradecimentos:

Agradecemos ao professor Julio Cezar Estrella pela aten��o e prestatividade,
e a todos que chegaram neste ponto do documento; fato que significa que,
provavelmente, leram ele inteiro.
-------------------------------------------------------------------------------
Copyright (C) 2015 Loys Henrique Sacomano Gibertoni, Sady Sell Neto
===============================================================================