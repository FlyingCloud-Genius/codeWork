mpic++ mandelbrot.cpp -lX11 -o mandelbrot.out
for i in 300 500 800 1000 3000 5000 8000 10000
	do
		./mandelbrot.out $i $i
	done
