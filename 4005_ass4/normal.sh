for i in 6  12 16
	do
		mpirun -np $i ./heat_sim_mpi 200 200
	done
