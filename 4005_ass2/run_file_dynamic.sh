mpic++ mandelbrot_dynamic.cpp -lX11 -o mandelbrot_dynamic.out
for i in 50 100 200 400 500
	do
		for j in 300 500 800 1000 3000 5000 8000 10000
			do
				for k in 2 4 8 20 25 50
					do
						echo $i
						mpirun -n $k ./mandelbrot_dynamic.out $i $j $j
					done
			done
	done
