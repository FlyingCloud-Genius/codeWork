#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <time.h>

#define MASTER 0

using namespace std;


typedef struct complextype
        {
        float real, imag;
        } Compl;

int main (int argc, char* argv[]){
	int X_RESN = atoi(argv[1]);
    int Y_RESN = atoi(argv[2]);
	
	//mpi magic
	MPI_Init(&argc, &argv);
	//running processers' number
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (size < 2) {
		fprintf(stderr, "Must use at least two process for this example\n");
	}

	//rank of the processer
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Compl   z, c;
    double  lengthsq, temp;
    int *output = (int *)malloc(sizeof(int) * (X_RESN * Y_RESN));

	int sub_divide = X_RESN * Y_RESN / size;
	int tag = 0;
	
	struct timeval timeStart, timeEnd, timeSystemStart; 
    double runTime = 0, systemRunTime; 
    gettimeofday(&timeStart, NULL);


	// processing units
	int x_start, x_end, y_start, y_end;
	x_start = rank * sub_divide % X_RESN;
	x_end = (x_start + sub_divide) % X_RESN;
	if (x_end == 0) {
		x_end = X_RESN;
	}
	y_start = rank * sub_divide / X_RESN;
	y_end = (rank * sub_divide + sub_divide) / X_RESN;

	printf("start processing with x_start: %d, x_end: %d, y_start: %d, y_end: %d\n", x_start, x_end, y_start, y_end);

	int i = x_start, j = y_start, k;
	int *result_array = (int *)malloc(sizeof(int) * sub_divide);
	int index = 0;
	while (true) {
    	while (true) {
    	    z.real = z.imag = 0.0;
    	    c.real = ((float) i - Y_RESN/2)/(Y_RESN/4);  //scale factors for 800 x 800 window 
    	    c.imag = ((float) j - X_RESN/2)/(X_RESN/4);
    	    k = 0;

    	    do  {                            // iterate for pixel color
	
			    temp = z.real*z.real - z.imag*z.imag + c.real;
			    z.imag = 2.0*z.real*z.imag + c.imag;
			    z.real = temp;
			    lengthsq = z.real*z.real+z.imag*z.imag;
			    k++;
    	    } while (lengthsq < 12 && k < 100); //lengthsq and k are the threshold
    	    if (k >= 100) {
    	    	result_array[index] = 1;
    	    } else {
				result_array[index] = 0;
			}
			index++;
			i++;
			if (i == X_RESN || (i == x_end && j == y_end - 1) || index == sub_divide) {
				i = 0;
				break;
			}
    	}
		j++;
		if (j > y_end || (i == x_end && j == y_end - 1) || index == sub_divide) {
			printf("i: %d, j: %d, k: %d\n", i, j, k);
			break;
		}
	}

	printf("gathering result...\n");

	MPI_Gather(result_array, sub_divide, MPI_INT, output, sub_divide, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == MASTER) {
		
		gettimeofday( &timeEnd, NULL );
    	runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
    	printf("Name: Yunteng Yang\n");
    	printf("Student ID: 116010264\n");
    	printf("MPI static schedulling for assignment 2\n");
		printf("total number of tasks: %d\n", size);
		printf("height and width is: %d\n", X_RESN);
    	printf("runTime is %lfs\n", runTime);

		//draw the outcome
		/*Window          win;       
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

    	if (  (display = XOpenDisplay (display_name)) == NULL ) {
       		fprintf (stderr, "drawon: cannot connect to X server %s\n",
        	                    XDisplayName (display_name) );
      		exit (-1);
      	}


		screen = DefaultScreen (display);
		display_width = DisplayWidth (display, screen);
		display_height = DisplayHeight (display, screen);


		width = X_RESN;
		height = Y_RESN;



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

		XColor color;
		color.red=10000; //range from 0~65535
		color.green=10000;
		color.blue=10000;	
	
		Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color);
		//set the color and attribute of the graphics content
		XSetForeground (display, gc, color.pixel);
		XSetBackground (display, gc, BlackPixel (display, screen));
		XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

		for (int i=0;i<X_RESN;i++){
		for (int j=0;j<Y_RESN;j++){
		  if(output[j*Y_RESN+i]==1){
		    XDrawPoint (display, win, gc, i, j);
		    usleep(1);
		    //XDrawPoint cannot draw too fast, otherwise the image cannot be drawn
		    //normally you could try to not use the usleep(1), it only black images are shown, try to use this
		  }
		}
		}


		usleep(250000);
		XFlush (display);
		cout<<"finish running"<<endl;
		sleep(100); */
		return 0;
	}

	MPI_Finalize();
}

