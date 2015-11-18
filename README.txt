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
utilitário make para compilar. Em particular usar o comando do linux:
make
compilará o código.

Caso prefira compilar manualmente, use o seguinte comando no linux:
mpic++ -std=c++11 main.cpp -o smooth -w -fopenmp -O3 -march=native

E será criado o binario "smooth" para rodar o programa.
-------------------------------------------------------------------------------

2. Como executar:

É fortemente sugerido que se execute o modo sequencial via:
make run-sequential
e o modo paralelo via:
make run-parallel

Mas segue as instruções abaixo, caso queira-se rodar manualmente.s

Para executar o sequencial a partir de onde ele está, use:
./smooth sequential File

Onde File é o arquivo sobre o qual deseja-se aplicar o filtro. Este parâmetro
pode ser omitido, que fará com que o programa pergunte ao usuário qual deve
ser o arquivo.

Para executa-lo de modo paralelo, use:
mpirun -np 16 --hostfile hosts.txt ./smooth parallel File

Onde File é exatamente a mesma coisa do que o parametro File da execução
sequencial.

-------------------------------------------------------------------------------

3. Alteração do código para alteração do ponto de vista dos resultados:

É possível alterar o código para que se exiba os tempos em unidades diferentes.
(Por padrão, o programa exibe os tempos de execução em segundos).

Para isso, basta alterar as linhas do arquivo "Netpbm_Image.hpp":
716:	double_seconds ellapsed_time = end - start;
804:	double_seconds ellapsed_time = end - start;

O tipo da variável ("double_seconds", no caso acima) pode ser alterado para mudar a
unidade de tempo. Suas possibilidades são:

double_hours: horas;
double_minutes: minutos;
double_seconds: segundos (padrão);
double_milliseconds: milissegundos;
double_microseconds: microssegundos;
double_nanoseconds: nanossegundos.
-------------------------------------------------------------------------------

4. Agradecimentos:

Agradecemos ao professor Julio Cezar Estrella pela atenção e prestatividade,
e a todos que chegaram neste ponto do documento; fato que significa que,
provavelmente, leram ele inteiro.
-------------------------------------------------------------------------------
Copyright (C) 2015 Loys Henrique Sacomano Gibertoni, Sady Sell Neto
===============================================================================