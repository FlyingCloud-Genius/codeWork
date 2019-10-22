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
	int num_task = atoi(argv[1]);
	int X_RESN = atoi(argv[2]);
    int Y_RESN = atoi(argv[3]);
	
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
    int i,j,k;
    double  lengthsq, temp;
    int *output = (int *)malloc(sizeof(int) * (X_RESN * Y_RESN));

	int sub_divide = X_RESN * Y_RESN / num_task;
	int tag = 0;

	//scheduling 
	if (rank == MASTER) {
		struct timeval timeStart, timeEnd, timeSystemStart; 
    	double runTime = 0, systemRunTime; 
    	gettimeofday(&timeStart, NULL );
		
		queue<int> avail_core;
    	for (int p = 1;p < size;p++){
      		avail_core.push(p);
    	}    
		int current_task = 0;
	    int next_core;
    	int start_index;
    	int recv_rank;
    	int recv_start_index;
    	int finished_task = 0;
		int *recv_array=new int[sub_divide+2]; // the first element is rank, the second element is start_index
		
		while (finished_task < num_task) {
			while (avail_core.empty() == 0 && current_task < num_task) {
				next_core = avail_core.front();
				avail_core.pop();
				start_index = sub_divide * current_task;
				MPI_Send(&start_index, 1, MPI_INT, next_core, tag, MPI_COMM_WORLD);
				current_task++;
			}
			MPI_Status status;
			MPI_Recv(recv_array, sub_divide + 2, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);

			recv_rank = recv_array[0];
			recv_start_index = recv_array[1];
			for (int index = 0; index < sub_divide; index++) {
				output[index + recv_start_index] = recv_array[index + 2];
			}
			avail_core.push(recv_rank);
			finished_task++;
		}

		int stop_signal=-1;
    	for (int p=1;p<size;p++){
        	MPI_Send(&stop_signal,1,MPI_INT,p,tag,MPI_COMM_WORLD);
    	}

		gettimeofday( &timeEnd, NULL );
    	runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
    	printf("Name: Yunteng Yang\n");
    	printf("Student ID: 116010264\n");
    	printf("MPI dynamic schedulling for assignment 2\n");
		printf("running processse: %d\n", size);
		printf("total number of tasks: %d\n", num_task);
		printf("height and width is: %d\n", X_RESN);
    	printf("runTime is %lfs\n", runTime);

		//draw the outcome
		Window          win;       
    	char            *window_name = "test", *display_name = NULL;                  //    initialization for a window 
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

      	/* get screen size */
		screen = DefaultScreen (display);
		display_width = DisplayWidth (display, screen);
		display_height = DisplayHeight (display, screen);

		//set window size
		width = X_RESN;
		height = Y_RESN;


		//set window position 

		x = 0;
		y = 0;

		// create opaque window 

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

		/* create graphics context */
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
		sleep(100);
		return 0;
	}

	// processing unit
	if (rank != MASTER) {
		int start_index;
		int *send_array = new int[sub_divide + 2];
		MPI_Status status;
		int x_start, x_end, y_start, y_end;

		while(1) {
			MPI_Recv(&start_index, 1, MPI_INT, MASTER, tag, MPI_COMM_WORLD, &status);
			if (start_index == -1) {
				break;
			}
			send_array[0] = rank;
			send_array[1] = start_index;
			x_start = start_index % X_RESN;
			x_end = (start_index + sub_divide) % X_RESN;
			if (x_end == 0) {
				x_end = X_RESN;
			}
			y_start = start_index / X_RESN;
			y_end = (start_index + sub_divide) / X_RESN;
			int i = x_start, j = y_start;
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
		    	      send_array[index + 2] = 1;

		    	    } else {
						send_array[index + 2] = 0;
					}
					index++;
					i++;

					if (i == X_RESN || (i == x_end && j == y_end) || index==sub_divide) {
						i = 0;
						break;
					}
		    	}
				j++;
				if (j > y_end || (i == x_end && j == y_end) || index == sub_divide) {
					break;
				}
			}
			MPI_Send(send_array, sub_divide + 2, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();
}

