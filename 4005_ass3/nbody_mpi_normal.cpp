#include <mpi.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define G 10
#define MAX_W 100
#define MIN_W 50
#define iteration 2000
#define T 0.01
#define WALL 10

int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int bodyNum;

struct Body {
    double px, py;
    double vx, vy;
    double w;
};

int main (int argc,char *argv[]) {
	SCREEN_WIDTH = atoi(argv[1]);
	SCREEN_HEIGHT = atoi(argv[1]);
    bodyNum = atoi(argv[2]);

	int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	//create self difined datatype
	MPI_Datatype MyType;
	// old type in the datatype
    MPI_Type_contiguous(5, MPI_DOUBLE, &MyType);
	//commit to the program
    MPI_Type_commit(&MyType);

    MPI_Status status;
    int division = bodyNum / size;

	int i, j, k;
    double vx[bodyNum];
    double vy[bodyNum];
    double distanceX, distanceY;
    double distance;
    double F;
    double start, end, total;

    struct Body *local;
    local = (struct Body*)malloc(division * sizeof(struct Body));
    struct Body* nbody = (struct Body*)malloc(bodyNum * sizeof(struct Body));

	start = MPI_Wtime();
	if (rank == 0) {
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

		int scr = DefaultScreen(display);
		int pm = XCreatePixmap(display, win, SCREEN_WIDTH, SCREEN_HEIGHT, DefaultDepth(display, scr));

		srand(time(0));
		//initialization
		for(i = 0;i < bodyNum; i++) {
			nbody[i].px = rand() % SCREEN_WIDTH;
		 	nbody[i].py = rand() % SCREEN_HEIGHT;
			nbody[i].vx = 0;
			nbody[i].vy = 0;
			nbody[i].w = rand() % (MAX_W-MIN_W) + MIN_W;
		}

        for (i = 1; i < size; i++) {
            MPI_Send(nbody, bodyNum, MyType, i, i, MPI_COMM_WORLD);
		}

		//refreshing
		for(k = 0; k < iteration; k++) {
			XSetForeground(display, gc, 0);
			XFillRectangle(display, pm, gc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			//calculate the force and velocity
		   	for(i = division * rank; i < division * (rank + 1); i++) {
				
				for(j = 0; j < bodyNum; j++) {
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
		    for(i = division * rank; i < division * (rank + 1); i++) {
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
		    }

			int startPoint = division * rank;
			for(i = division * rank; i < division * (rank + 1); i++) {
                local[i - startPoint].px = nbody[i].px;
                local[i - startPoint].py = nbody[i].py;
                local[i - startPoint].vy = nbody[i].vy;
                local[i - startPoint].vx = nbody[i].vx;
                local[i - startPoint].w = nbody[i].w;
            }
			MPI_Gather(local, (division / size) * (rank + 1) - (division / size) * rank, MyType, nbody, (division / size) * (rank + 1) - (division / size) * rank, MyType, 0, MPI_COMM_WORLD);

		    XSetForeground(display, gc, WhitePixel(display, scr));
			//plotting
		    for(i = 0; i < bodyNum; i++) {
				XDrawPoint(display, pm, gc, nbody[i].py, nbody[i].px);
		    }
			XCopyArea(display, pm, win, gc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);

			for (j = 1; j < size; j++) {
				MPI_Send(nbody, bodyNum, MyType, j, j, MPI_COMM_WORLD);
			}
		}

		XFreePixmap(display, pm);
		XCloseDisplay(display);
		end = MPI_Wtime();
	} else {
        MPI_Recv(nbody, bodyNum, MyType, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		//refreshing
		for(k = 0; k < iteration; k++) {
			//calculate the force and velocity
		   	for(i = division * rank; i < division * (rank + 1); i++) {
				for(j = 0; j < bodyNum; j++) {
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
		    for(i = division * rank; i < division * (rank + 1); i++) {
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
		    }

			int startPoint = division * rank;
			for(i = division * rank; i < division * (rank + 1); i++) {
                local[i - startPoint].px = nbody[i].px;
                local[i - startPoint].py = nbody[i].py;
                local[i - startPoint].vy = nbody[i].vy;
                local[i - startPoint].vx = nbody[i].vx;
                local[i - startPoint].w = nbody[i].w;
            }

			MPI_Gather(local, (division / size) * (rank + 1) - (division / size) * rank, MyType, nbody, (division / size) * (rank + 1) - (division / size) * rank, MyType, 0, MPI_COMM_WORLD);
            MPI_Recv(nbody, bodyNum, MyType, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		
		
	}

	//end
	if (rank == 0) {
		// time calculation
		total = end - start;
		// End info
		printf("Execution Time is: %f s\n", total);
	}
	MPI_Finalize();
	return 0;
}
