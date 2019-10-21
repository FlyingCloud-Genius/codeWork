#include <iostream>
#include <cstdlib>
#include <fstream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <time.h>
using namespace std;

typedef struct complextype
        {
        float real, imag;
        } Compl;

int main (int argc, char* argv[]){
    Window          win;       
    char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
    Display         *display;
    GC              gc;   //this is a graphic content, it could be a pixel color
    unsigned long            valuemask = 0;
      XGCValues       values; //value of the graphics content
    XSizeHints      size_hints;
    Pixmap          bitmap;
    XSetWindowAttributes attr[1];
    int             width, height,                  /* window size */
                    x, y,                           /* window position */
                    border_width,                   /*border width in pixels */
                    display_width, display_height,  /* size of screen */
                    screen;                         /* which screen */

    if (  (display = XOpenDisplay (display_name)) == NULL ) {
       fprintf (stderr, "drawon: cannot connect to X server %s\n",
                            XDisplayName (display_name) );
      exit (-1);
      }

      /* get screen size */
      screen = DefaultScreen (display);
      display_width = DisplayWidth (display, screen);
      display_height = DisplayHeight (display, screen);

      /* set window size */
    int X_RESN=800;
    int Y_RESN=800;
    width = X_RESN;
    height = Y_RESN;

    /* set window position */

    x = 0;
    y = 0;

    /* create opaque window */

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

<<<<<<< HEAD:4005_ass2/mandelbrot.cpp
    XColor color;
    color.red=10000; //range from 0~65535
    color.green=10000;
    color.blue=10000;
=======
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
>>>>>>> 64988cc1621ae2324818a0b396e51cd740c1c9fd:4005_ass2/mandelbrot_dynamic.cpp

    Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color);
    //set the color and attribute of the graphics content
    XSetForeground (display, gc, color.pixel);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

<<<<<<< HEAD:4005_ass2/mandelbrot.cpp
    Compl   z, c;
    int i,j,k;
    double  lengthsq, temp;
    int *output=(int *)malloc(sizeof(int) * (X_RESN * Y_RESN));

    struct timeval timeStart, timeEnd, timeSystemStart; 
    double runTime=0, systemRunTime; 
    gettimeofday(&timeStart, NULL );

    for(i=0; i < X_RESN; i++){
        for(j=0; j < Y_RESN; j++) {
            z.real = z.imag = 0.0;
            c.real = ((float) j - Y_RESN/2)/(Y_RESN/4);                //scale factors for 800 x 800 window 
            c.imag = ((float) i - X_RESN/2)/(X_RESN/4);
            k = 0;

            do  {                                             // iterate for pixel color

            temp = z.real*z.real - z.imag*z.imag + c.real;
            z.imag = 2.0*z.real*z.imag + c.imag;
            z.real = temp;
            lengthsq = z.real*z.real+z.imag*z.imag;
            k++;
            } while (lengthsq < 12 && k < 100); //lengthsq and k are the threshold
            if (k >= 100) {
              output[i*Y_RESN+j]=1;
            }
        }
    }
    gettimeofday( &timeEnd, NULL ); 
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;  
    printf("runTime is %lf\n", runTime); 
    fstream output_array;
    output_array.open("output2.txt", ios::app|ios::out|ios::in);
    for(int i = 0; i < 800*800;i++){
        output_array<<output[i];
    }
    output_array.close();
    for (i=0;i<X_RESN;i++){
        for (int j=0;j<Y_RESN;j++){
            if(output[i*Y_RESN+j]==1){
                XDrawPoint (display, win, gc, i, j);
                usleep(1);
        //XDrawPoint cannot draw too fast, otherwise the image cannot be drawn
        //normally you could try to not use the usleep(1), it only black images are shown, try to use this
            }
        }
    }

    //usleep(250000);
    XFlush (display);
    cout<<"finish running"<<endl;
    sleep(10);
    return 0;
}
=======
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
					if (i == X_RESN || (i == x_end && j == y_end)) {
						i = 0;
						break;
					}
		    	}
				j++;
				if (j > y_end || (i == x_end && j == y_end)) {
					break;
				}
			}
			MPI_Send(send_array, sub_divide + 2, MPI_INT, MASTER, tag, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();
}
>>>>>>> 64988cc1621ae2324818a0b396e51cd740c1c9fd:4005_ass2/mandelbrot_dynamic.cpp
