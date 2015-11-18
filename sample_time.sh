for i in {1..10..1}
do
	
	f=/home/grupo01a/imagens/gray/flower_duo.pgm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt
	
	f=/home/grupo01a/imagens/gray/italian_valley.pgm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt
	
	f=/home/grupo01a/imagens/gray/large_elevation.pgm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt
	
	f=/home/grupo01a/imagens/gray/tower_bridge.pgm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt
	
	f=/home/grupo01a/imagens/rgb/flower_duo.ppm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt
	
	f=/home/grupo01a/imagens/rgb/italian_valley.ppm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt
	
	f=/home/grupo01a/imagens/rgb/large_elevation.ppm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt
	
	f=/home/grupo01a/imagens/rgb/tower_bridge.ppm
	echo "Sequential $i: $f"
	./smooth sequential $f > $f.seq$i.txt
	echo "Parallel $i: $f"
	mpirun -np 16 --hostfile hosts.txt ./smooth parallel $f > $f.par$i.txt

done
