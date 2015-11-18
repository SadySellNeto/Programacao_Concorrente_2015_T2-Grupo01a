all: clean
	mpic++ -std=c++11 main.cpp -o smooth -w -fopenmp -O3 -march=native
	chmod u+x sample_time.sh

clean:
	rm -f smooth
	rm -f *~

run:
	echo "Will now sample 10 times..."
	./sample_time.sh

run-sequential:
	./smooth sequential

run-parallel:
	mpirun --hostfile hosts.txt ./smooth parallel

zip:
	rm -f Trabalho2-grupo01a-turmaA.zip
	zip -r Trabalho2-grupo01a-turmaA.zip Makefile hosts.txt README.txt sample_time.sh main.cpp Netpbm_Image.hpp 
