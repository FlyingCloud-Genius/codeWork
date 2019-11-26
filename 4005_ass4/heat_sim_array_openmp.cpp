#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <omp.h>


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

void oldTemUpdate(float *tem, float *oldTem, int N, int M) {
	int i, j;
	#pragma omp parallel for private(j)
	for (i = 0; i < M; i++) {
		for (j = 0; j < M; j++) {
			oldTem[i * N + j] = tem[i * N + j];
		}
	}
}

int main(int argc, char **argv) {
	int N = atoi(argv[1]); //x
	int M = atoi(argv[2]); //y
	int threadNum = atoi(argv[3]); //number of threads used
	omp_set_num_threads(threadNum);

	float tem[N * M];
	float oldTem[N * M];
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

	float alpha = 0.05; // all the calculaiton melted to this place
	float term1, term2, term3;

	//iterating
	clock_t timeStart, timeEnd;
	timeStart = clock();

	int i, j;
	for (int counter = 0; counter < 10000; counter++) {
		#pragma omp parallel for private(j) private (term1, term2, term3)
		for (i = 1; i < M - 1; i++) {
			for (j = 1; j < M - 1; j++) {
				term1 = oldTem[i * N + j];
				term2 = alpha * (oldTem[i * N + j - N] - 2 * oldTem[i * N + j] + oldTem[i * N + j + N]);
				term3 = alpha * (oldTem[i * N + j- 1] - 2 * oldTem[i * N + j] + oldTem[i * N + j + 1]);
				tem[i * N + j] = term1 + term2 + term3;
			} 
		}
		oldTemUpdate(tem, oldTem, N, M);
	}


	//initializing the problem
	double runTime;
	
	//start jacobi
	//outputMatrix(oldTem, N, M);
	timeEnd = clock();
	runTime = (timeEnd - timeStart) / CLOCKS_PER_SEC;
	printf("runTime is %lfs\n", runTime); 
	printf("finish...\n");
	

	return 0;
}
