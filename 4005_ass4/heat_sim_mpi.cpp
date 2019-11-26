#include <stdlib.h>
#include "mpi.h"
#include <stdio.h>
#include <math.h>
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/Xos.h>
#include <time.h>


using namespace std;

typedef struct complextype {
	float real, imag;
} Compl;

void outputMatrix(float *tem, int N, int M) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			printf("%f ", tem[i * N + j]);
		}
		printf("\n");
	}
}

/* void jacobiDrawing(float *tem, float *oldTem, int N, int M) {
	float alpha = 0.05; // all the calculaiton melted to alpha
	float term1, term2, term3;

	for (int counter = 0; counter < 10; counter++) {
		for (int i = 1; i < N - 1; i++) {
			for (int j = 1; j < M - 1; j++) {
				term1 = oldTem[i * N + j];
				term2 = alpha * (oldTem[(i - 1) * N + j] - 2 * oldTem[i * N + j] + oldTem[(i + 1) * N + j]);
				term3 = alpha * (oldTem[i * N + (j - 1)] - 2 * oldTem[i * N + j] + oldTem[i * N + (j + 1)]);
				tem[i * N + j] = term1 + term2 + term3;
			} 
		}
		oldTem = tem;
		//outputMatrix(tem, N, M);
	}
} */

/* void drawing(float *tem, float *oldTem, int N, int M) {
	Window          win;       
    char            *window_name = "test", *display_name = NULL;                 
    Display         *display;
    GC              gc;   //this is a graphic content, it could be a pixel color
    unsigned long            valuemask = 0;
      XGCValues       values; //value of the graphics content
    XSizeHints      size_hints;
    Pixmap          bitmap;
    XSetWindowAttributes attr[1];
    int             width, height,              
                    x, y,                        
                    border_width,                 
                    display_width, display_height, 
                    screen;                     

    if ((display = XOpenDisplay (display_name)) == NULL) {
  		fprintf (stderr, "drawon: cannot connect to X server %s\n", XDisplayName (display_name));
		exit (-1);
	}


  	screen = DefaultScreen (display);
  	display_width = DisplayWidth (display, screen);
  	display_height = DisplayHeight (display, screen);

    width = N;
    height = M;
    x = 0;
    y = 0;


    border_width = 4;
    win = XCreateSimpleWindow (display, RootWindow (display, screen),
                          x, y, width, height, border_width, 
                          WhitePixel (display, screen), WhitePixel (display, screen)); //Change to WhitePixel (display, screen) if you want a white background

    size_hints.flags = USPosition|USSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = width;
    size_hints.height = height;
    size_hints.min_width = 300;
    size_hints.min_height = 300;

    XSetNormalHints (display, win, &size_hints);
    XStoreName(display, win, window_name);

    gc = XCreateGC (display, win, valuemask, &values);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetForeground (display, gc, WhitePixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel (display, screen);

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow (display, win);
    XSync(display, 0);

    XFlush (display);

	//color initialization from blue to red
	float red[20] = {0, 0, 0.68, 0.88, 0.69, 1, 1, 1, 1, 1,
					 1, 1, 1, 1, 0.93, 1, 1, 1, 1, 0.55};
	float green[20] = {0, 0, 0.85, 1, 0.89, 1, 1, 1, 1, 1,
					 0.97, 1, 1, 1, 0.87, 0.65, 0.55, 0.20, 0, 0};
	float blue[20] = {0.55, 1, 0.90, 1, 0.87, 1, 1, 1, 1, 1,
					 0.79, 0.94, 0, 0.88, 0.38, 0, 0, 0.20, 0, 0};
    XColor color[20];
	for (int i = 0; i < 20; i++) {
		color[i].red = red[i] * 65535;
		color[i].green = green[i] * 65535;
		color[i].blue = blue[i] * 65535;
		color[i].flags = DoRed | DoGreen | DoBlue;
		XAllocColor(display, DefaultColormap(display, screen), &color[i]);
	}

    //set the color and attribute of the graphics content
    XSetForeground (display, gc, color[0].pixel);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

	//iterating jacobi and draw points out
	
	for (int counter = 0; counter < 1000; counter++) {
		jacobiDrawing(tem, oldTem, N, M);
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++) {
				int colorNum = tem[i * N + j] / 500;
				XSetForeground(display, gc, color[colorNum].pixel);
				XDrawPoint(display, win, gc, i, j);
				usleep(1);
			}
		}
	}
} */

int main(int argc, char **argv) {
	int N = atoi(argv[1]); //x
	int M = atoi(argv[2]); //y

	MPI_Init(&argc, &argv);
	
	int size, taskId;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskId);

	//float *oldTem = (float *) malloc(N * M * sizeof(float));
	float oldTem[N * M];
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			oldTem[N * i + j] = 500;
		}
	}
		
	//assigning temperature to the wall at the top
	for (int j = N / 2 - 50; j < N / 2 + 50; j++) {
		oldTem[j] = 10000;
	}

	float alpha = 0.05; // all the calculaiton melted to this place
	float term1, term2, term3;
	int localSize = N * (taskId + 1) / size - N * taskId / size;
	//float *tem = (float *) malloc(localSize * M * sizeof(float));
	float tem[localSize * M];
	for (int i = 0; i < localSize; i++) {
		for (int j = 0; j < M; j++) {
			tem[i * N + j] = oldTem[(i + taskId / size * N) * N + j];
		}
	}

	//iterating
	double timeStart, timeEnd;
	if (taskId == 0) {
		timeStart = MPI_Wtime();
	}

	int startN, endN, index;
	startN = 0;
	endN = localSize;
	if (taskId == 0) {
		startN = 1;
	} else if (taskId == size - 1) {
		endN = endN - 1;
	}
	//printf ("end: %d, start: %d\n", endN, startN);
	//printf("initializing finished...\n");
	for (int counter = 0; counter < 10000; counter++) {
		//printf("counter: %d\n", counter);
		for (int i = startN; i < endN; i++) {
			//printf("%d\n", i);
			for (int j = 1; j < M - 1; j++) {
				index = (N * taskId / size + i) * N + j;
				term1 = oldTem[index];
				term2 = alpha * (oldTem[index - N] - 2 * oldTem[index] + oldTem[index + N]);
				term3 = alpha * (oldTem[index - 1] - 2 * oldTem[index] + oldTem[index + 1]);
				//printf("N: %f, E: %f, S: %f, W: %f, cur: %f\n", oldTem[index + N], oldTem[index + 1], oldTem[index - N], oldTem[index - 1], tem[i * N + j]);
				tem[i * N + j] = term1 + term2 + term3;
			} 
		}
		//printf("sending the result to: %d\n", N * taskId / size * M);
		MPI_Allgather(&tem, localSize * M, MPI_FLOAT, &oldTem, localSize * M, MPI_FLOAT, MPI_COMM_WORLD);
		//outputMatrix(oldTem, N, M);
	}


	//initializing the problem
	if (taskId == 0) {
		double runTime;
		
		//start jacobi
		//outputMatrix(oldTem, N, M);
		timeEnd = MPI_Wtime();
		runTime = timeEnd - timeStart;
		printf("runTime is %lfs\n", runTime); 
		printf("finish...\n");
	}
	

	return 0;
}
