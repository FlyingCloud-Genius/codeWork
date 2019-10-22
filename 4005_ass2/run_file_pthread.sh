mpic++ mandelbrot_pthread.cpp -lX11 -lpthread -o mandelbrot_pthread.out
for i in 300 500 800 1000 3000 5000 8000 10000
	do
		for j in 2 4 8 10 20 25 40 50 100
			do
				./mandelbrot_pthread.out $j $i $i
			done
	done
