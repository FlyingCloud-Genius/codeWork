#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <sys/time.h>
#include <pthread.h>

using namespace std;

typedef struct complextype {
	float real, imag;
} Compl;

typedef struct thread_data {
	int taskId;
	float *tem;
	float *oldTem;
	int N;
	int M;
	int threadNum;
};

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
}

void drawing(float *tem, float *oldTem, int N, int M) {
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
}*/

void *jacobiIteration(void *data) {
	float alpha = 0.05; // all the calculaiton melted to this place

	thread_data *input = (thread_data *) data;
	int taskId = input->taskId;
	int N = input->N;
	int M = input->M;
	float *tem = input->tem;
	float *oldTem = input->oldTem;
	int threadNum = input->threadNum;
	
	float term1, term2, term3;
	
	int start;
	//outputMatrix(oldTem, N, M);
	if (taskId * N / threadNum == 0) {
		start = 1;
	} else {
		start = taskId * N / threadNum;
	}
	int bound;
	if ((taskId + 1) * N / threadNum == N) {
		bound = N - 1;
	} else {
		bound = (taskId + 1) * N / threadNum;
	}

	for (int i = start; i < (taskId + 1) * N / threadNum; i++) {
		for (int j = 1; j < M - 1; j++) {
			term1 = oldTem[i * N + j];
			term2 = alpha * (oldTem[(i - 1) * N + j] - 2 * oldTem[i * N + j] + oldTem[(i + 1) * N + j]);
			term3 = alpha * (oldTem[i * N + (j - 1)] - 2 * oldTem[i * N + j] + oldTem[i * N + (j + 1)]);
			tem[i * N + j] = term1 + term2 + term3;
		} 
	}
}

void *oldTemUpdate(void *data) {
	thread_data *input = (thread_data *) data;
	int taskId = input->taskId;
	int N = input->N;
	int M = input->M;
	float *tem = input->tem;
	float *oldTem = input->oldTem;
	int threadNum = input->threadNum;

	for (int i = taskId * N / threadNum; i < (taskId + 1) * N / threadNum; i++) {
		for (int j = 0; j < M; j++) {
			oldTem[i * N + j] = tem[i * N + j];
		}
	}
}

int main(int argc, char **argv) {
	int N = atoi(argv[1]); //x
	int M = atoi(argv[2]); //y
	int threadNum = atoi(argv[3]); //thread number

	//initializing the problem
	float tem[N * M];
	float oldTem[N * M];

	int rc;
	pthread_t thread[threadNum];
	thread_data data[threadNum];
	
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			tem[N * i + j] = 500;
			oldTem[N * i + j] = 500;
		}
	}

	//assigning temperature to the wall at the top
	for (int j = N / 2 - 50; j < N / 2 + 50; j++) {
		tem[j] = 10000;
		oldTem[j] = 10000;
	}

	printf("start jacobi...\n");
	
	struct timeval timeStart, timeEnd, timeSystemStart;
	double runTime;
    gettimeofday(&timeStart, NULL );

	//jacobi iteration or call drawing (drawing contains jacobi iteration)
	for (int counter = 0; counter < 10000; counter++) {
		for (int i = 0; i < threadNum; i++) {
			data[i].taskId = i;
			data[i].tem = tem;
			data[i].oldTem = oldTem;
			data[i].N = N;
			data[i].M = M;//partiitoning the input
			data[i].threadNum = threadNum;
			rc = pthread_create(&thread[i], NULL, jacobiIteration, &data[i]);
		}

		for (int i = 0; i < threadNum; i++) {
			pthread_join(thread[i], NULL);
		}

		for (int i = 0; i < threadNum; i++) {
			data[i].taskId = i;
			data[i].tem = tem;
			data[i].oldTem = oldTem;
			data[i].N = N;
			data[i].M = M;
			data[i].threadNum = threadNum;
			rc = pthread_create(&thread[i], NULL, oldTemUpdate, &data[i]);
		}

		for (int i = 0; i < threadNum; i++) {
			pthread_join(thread[i], NULL);
		}
	}
	//drawing(tem, oldTem, N, M);

    gettimeofday( &timeEnd, NULL ); 
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;  
    printf("runTime is %lfs\n", runTime); 
	
	printf("finish...\n");
	return 0;
}
