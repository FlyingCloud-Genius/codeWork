#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

using namespace std;

typedef struct complextype {
	float real, imag;
} Compl;

void outputMatrix(vector<vector<float>> tem, int N, int M) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			printf("%f ", tem[i][j]);
		}
		printf("\n");
	}
}

vector<vector<float>> jacobiDrawing(vector<vector<float>> tem, int N, int M) {
	float alpha = 0.05; // all the calculaiton melted to alpha
	vector<vector<float>> oldTem = tem;
	float term1, term2, term3;

	for (int i = 1; i < N - 1; i++) {
		for (int j = 1; j < M - 1; j++) {
			term1 = oldTem[i][j];
			term2 = alpha * (oldTem[i - 1][j] - 2 * oldTem[i][j] + oldTem[i + 1][j]);
			term3 = alpha * (oldTem[i][j - 1] - 2 * oldTem[i][j] + oldTem[i][j + 1]);
			tem[i][j] = term1 + term2 + term3;
		} 
	}
	oldTem = tem;
	//outputMatrix(tem, N, M);
	return tem;
}

void drawing(vector<vector<float>> tem, int N, int M) {
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

  	/* set window size */
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

	//color initialization
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
	
	for (int counter = 0; counter < 10000; counter++) {
		tem = jacobiDrawing(tem, N, M);
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++) {
				int colorNum = tem[i][j] / 500;
				XSetForeground(display, gc, color[colorNum].pixel);
				XDrawPoint(display, win, gc, i, j);
				usleep(1);
			}
		}
	}
}

vector<vector<float>> jacobiIteration(vector<vector<float>> tem, int N, int M) {
	float alpha = 0.05; // all the calculaiton melted to this place

	vector<vector<float>> oldTem = tem;
	float term1, term2, term3;
	
	//outputMatrix(oldTem, N, M);
	for (int counter = 0; counter < 10000; counter++) {
		for (int i = 1; i < N - 1; i++) {
			for (int j = 1; j < M - 1; j++) {
				term1 = oldTem[i][j];
				term2 = alpha * (oldTem[i - 1][j] - 2 * oldTem[i][j] + oldTem[i + 1][j]);
				term3 = alpha * (oldTem[i][j - 1] - 2 * oldTem[i][j] + oldTem[i][j + 1]);
				tem[i][j] = term1 + term2 + term3;
			} 
		}
		oldTem = tem;
		outputMatrix(tem, N, M);
	}
	return tem;
}

int main(int argc, char **argv) {
	int N = atoi(argv[1]); //x
	int M = atoi(argv[2]); //y

	//initializing the problem
	vector<vector<float>> a;
	
	for (int i = 0; i < N; i++) {
		vector<float> temp;
		for (int j = 0; j < M; j++) {
			temp.push_back(500);
		}
		a.push_back(temp);
	}

	//assigning temperature to the wall at the top
	for (int j = N / 2 - 50; j < N / 2 + 50; j++) {
		a[0][j] = 10000;
	}

	printf("start jacobi...\n");
	//jacobi iteration or call drawing (drawing contains jacobi iteration)
	//a = jacobiIteration(a, N, M);
	drawing(a, N, M);
	
	printf("finish\n");
	return 0;
}
