mpic++ mandelbrot_dynamic.cpp -lX11 -o mandelbrot_dynamic.out
for i in 200 400
	do
		for j in 300 500 800 1000 3000 5000 8000 10000
			do
				for k in 2 4 8 10 20 25 40 50
					do
						mpirun -n $k ./mandelbrot_dynamic.out $i $j $j
					done
			done
	done
