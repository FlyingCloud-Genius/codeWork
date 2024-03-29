#include <iostream>
#include <cstdlib>
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

      /* set window size */
    int X_RESN=atoi(argv[1]);
    int Y_RESN=atoi(argv[2]);
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
	printf("height and width are: %d \n", X_RESN);
    
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
