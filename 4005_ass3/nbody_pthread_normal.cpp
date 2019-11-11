#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define G 10
#define MAX_W 100
#define MIN_W 50
#define iteration 2000
#define T 0.01
#define WALL 10

int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int bodyNum;
int threadNum;

struct Body {
    double px, py;
    double vx, vy;
    double w;
};

struct ThreadData {
	int threadRank;
};

struct Body nbody[505];
double vx[505];
double vy[505];

void *refresh(void *data) {
	ThreadData *input = (ThreadData *) data;

	int threadRank = input->threadRank;

    double distanceX, distanceY, distance;
	double F;

	//calculate the force and velocity
  	for(int i = bodyNum / threadNum * threadRank; i < (bodyNum / threadNum) * (threadRank + 1); i++) {
   		for(int j = 0; j < bodyNum; j++) {
   			if (j == i) continue;
       		distanceX = nbody[j].px - nbody[i].px;
       		distanceY = nbody[j].py - nbody[i].py;
       		distance = sqrt((distanceX * distanceX) + (distanceY * distanceY));
       		if(distance <= 3) continue;
  			F = G * nbody[j].w / (distance * distance);
   			vx[i] = vx[i] + distanceX / distance * F * T;
   			vy[i] = vy[i] + distanceY / distance * F * T;
      	}
    }
	//change the position and refresh the velocity
    for(int i = 0; i < bodyNum; i++) {
     	nbody[i].px = nbody[i].px + vx[i] * T;
      	nbody[i].py = nbody[i].py + vy[i] * T;
   		nbody[i].vx = vx[i];
   		nbody[i].vy = vy[i];
		if (nbody[i].px > SCREEN_WIDTH - WALL) {
            nbody[i].px = SCREEN_WIDTH - WALL;
            nbody[i].vx = -1 * abs(nbody[i].vx);
        }
        if (nbody[i].px < WALL) {
            nbody[i].px = WALL;
            nbody[i].vx = abs(nbody[i].vx);
        }
        if (nbody[i].py > SCREEN_HEIGHT - WALL) {
            nbody[i].py = SCREEN_HEIGHT - WALL;
            nbody[i].vy = -1 * abs(nbody[i].vy);
        }
        if (nbody[i].py < WALL) {
            nbody[i].py = WALL;
            nbody[i].vy = abs(nbody[i].vy);
        }
		printf("x:%f, y:%f\n", nbody[i].px, nbody[i].py);
    }
}

int main (int argc,char *argv[]) {
	SCREEN_WIDTH = atoi(argv[1]);
	SCREEN_HEIGHT = atoi(argv[1]);
    bodyNum = atoi(argv[2]);
	threadNum = atoi(argv[3]);

    Window          win;
    unsigned
    int             width, height,
                    x, y, 
                    border_width,
                    display_width, 
                    display_height, 
                    screen; 
    char            *display_name = NULL;
    GC              gc;
    unsigned
    long            valuemask = 0;
    XGCValues       values;
    Display         *display;
    XSizeHints      size_hints;
    XInitThreads();
    XSetWindowAttributes attr[1]; 
    if ((display = XOpenDisplay(display_name)) == NULL) {
        fprintf(stderr, "Something wrong with the X server %s\n",
                            XDisplayName(display_name));
       exit(-1);
    }

    screen = DefaultScreen(display);
    display_width = DisplayWidth(display, screen);
    display_height = DisplayHeight(display, screen);

    width = SCREEN_WIDTH;
    height = SCREEN_HEIGHT;

    x = 0;
    y = 0;

    border_width = 4;
    win = XCreateSimpleWindow (display, 
                            RootWindow(display, screen),
                            x, y, width, height, border_width, 
                            BlackPixel(display, screen), 
                            WhitePixel(display, screen));

    size_hints.flags = USPosition|USSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = width;
    size_hints.height = height;
    size_hints.min_width = 300;
    size_hints.min_height = 300;
     
    XSetNormalHints(display, win, &size_hints);

    gc = XCreateGC(display, win, valuemask, &values);

    XSetBackground(display, gc, WhitePixel(display, screen));
    XSetForeground(display, gc, BlackPixel(display, screen));
    XSetLineAttributes(display, gc, 1, LineSolid, CapRound, 
                    JoinRound);

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel(display, screen);
    XChangeWindowAttributes(display, win, 
        CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow(display, win);
    XSync(display, 0);

    struct timespec start, end;
    double total;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int i,k;
    double F;
    int scr = DefaultScreen(display);
    int pm = XCreatePixmap(display, win, SCREEN_WIDTH, SCREEN_HEIGHT, DefaultDepth(display, scr));
	
	pthread_t tid[threadNum];
	int rc;
	
    srand(time(0));
	//initialization
    for(i = 0;i < bodyNum; i++) {
    	nbody[i].px = rand() % SCREEN_WIDTH;
     	nbody[i].py = rand() % SCREEN_HEIGHT;
    	nbody[i].vx = 0;
    	nbody[i].vy = 0;
    	nbody[i].w = rand() % (MAX_W-MIN_W) + MIN_W;
    }

	//refreshing
    for(k = 0; k < iteration; k++) {
    	XSetForeground(display, gc, 0);
		XFillRectangle(display, pm, gc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		//thread create and join
        for (i = 0; i < threadNum; i++){
			ThreadData *data;
			data->threadRank = i;
            rc = pthread_create(&tid[i], NULL, &refresh, &data);
        }
        for (i = 0; i < threadNum; i++) {
            pthread_join(tid[i], NULL);
        }

        XSetForeground(display, gc, WhitePixel(display, scr));
		//plotting
        for(i = 0; i < bodyNum; i++) {      		
			XDrawPoint(display, pm, gc, nbody[i].py, nbody[i].px);			
        }
		XCopyArea(display, pm, win, gc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
    }

    // time calculation
    clock_gettime(CLOCK_MONOTONIC, &end);

    total = end.tv_sec - start.tv_sec + (double) ((end.tv_nsec - start.tv_nsec) / 1000000000.0);
    // End info
    printf("Execution Time is: %f s\n", total);

    XFreePixmap(display, pm);
	XCloseDisplay(display);
	return 0;
}
