#include <mpi.h>
#include <cstdio>
#include <time.h>
#include <cstdlib>


void swap(int array[], int index1, int index2) {
	int temp = array[index1];
	array[index1] = array[index2];
	array[index2] = temp;
}


int main(int argc, char** argv) {

	int arrayLength = atoi(argv[1]);

	double startTime, endTime;
	
	MPI_Init(&argc, &argv);

	int processSize;
	MPI_Comm_size(MPI_COMM_WORLD, &processSize);
	if (processSize < 2) {
		fprintf(stderr, "Must use at least two process for this example\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	int processRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
	//initializing the array and send the numbers to each process
	int array[arrayLength];
	if (processRank == 0) {
		srand(time(0));
		for (int i = 0; i < arrayLength; i++) {
			array[i] = rand() % 1000;
			printf("%d ", array[i]);
		}
		printf("initializing the array finished... sending the array to other process...\n");
		for (int i = 1; i < processSize; i++) {
			MPI_Send(&array, arrayLength, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	} else {
		MPI_Status status;
		MPI_Recv(&array, arrayLength, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		for (int i = 0; i < arrayLength; i++) {
			printf("%d ", array[i]);
		}
	}

	startTime = MPI_Wtime();
	
	//Odd-Even Sort	
	int sorted = 0;
	int trigger = 0;
	int startPoint = processRank * (arrayLength / processSize);
	int endPoint = (processRank + 1) * (arrayLength / processSize);
	int partitionLength = endPoint - startPoint;
	for (int i = 0; i < arrayLength; i++) {
		int tempInt;
		MPI_Status status;
		if (trigger == 0) {
			//odd sort
			if (startPoint % 2 == 1) {
				//the first element is odd
				for (int i = startPoint + 2; i < endPoint; i += 2) {
					if (array[i] < array[i - 1]) {
						swap(array, i, i - 1);						
					}
				}
				if (partitionLength % 2 == 1 /* && processRank != processSize - 1 */) {// one more element at the front of the array
					if (processRank > 0) { //no receive and send back with process rank 0
						MPI_Recv(&tempInt, 1, MPI_INT, processRank - 1, 2, MPI_COMM_WORLD, &status);
						if (tempInt > array[startPoint]) {
							MPI_Send(&array[startPoint], 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
							array[startPoint] = tempInt;
						} else {
							MPI_Send(&tempInt, 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
						}
					}
				} else {//one element at the front and one element at the end or no element left
					if (processRank < processSize - 1) {
						// receive and send back
						if (processRank > 0) { //no receive and send back with process rank 0
							MPI_Recv(&tempInt, 1, MPI_INT, processRank - 1, 2, MPI_COMM_WORLD, &status);
							if (tempInt > array[startPoint]) {
								MPI_Send(&array[startPoint], 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
								array[startPoint] = tempInt;
							} else {
								MPI_Send(&tempInt, 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
							}
						}
						// send to the next and receive
						MPI_Send(&array[endPoint - 1], 1, MPI_INT, processRank + 1, 2, MPI_COMM_WORLD);
						MPI_Recv(&tempInt, 1, MPI_INT, processRank + 1, 1, MPI_COMM_WORLD, &status);
						array[endPoint - 1] = tempInt;
					} else {
						if (processRank == processSize - 1) {// receive the sent one and send back
							MPI_Recv(&tempInt, 1, MPI_INT, processRank - 1, 2, MPI_COMM_WORLD, &status);
							if (tempInt > array[startPoint]) {//send back the smaller one
								MPI_Send(&array[startPoint], 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
								array[startPoint] = tempInt;
							} else {
								MPI_Send(&tempInt, 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
							}
						}
					}
				}
			} else {
				//the first element is even
				for (int i = startPoint + 1; i < endPoint; i += 2) {
					if (array[i] < array[i - 1]) {
						swap(array, i, i - 1);
					}
				}
				if (partitionLength % 2 == 1 && processRank < processSize - 1) {
					MPI_Send(&array[endPoint - 1], 1, MPI_INT, processRank + 1, 2, MPI_COMM_WORLD);
					MPI_Recv(&tempInt, 1, MPI_INT, processRank + 1, 1, MPI_COMM_WORLD, &status);
					array[endPoint - 1] = tempInt;
				} else if (partitionLength % 2 == 1 && processRank == processSize - 1) {
					// no send and receive and the end
					
				} else if (partitionLength % 2 == 0) {
					// nothing need to be done
				}
			}
		} else {
			//even sort
			if (startPoint % 2 == 1) {
				// the first element is odd
				for (int i = startPoint + 1; i < endPoint; i += 2) {
					if (array[i] < array[i - 1]) {
						swap(array, i, i - 1);
					}
				}
				if (partitionLength % 2 == 1 && processRank < processSize - 1) {
					MPI_Send(&array[endPoint - 1], 1, MPI_INT, processRank + 1, 2, MPI_COMM_WORLD);
					MPI_Recv(&tempInt, 1, MPI_INT, processRank + 1, 1, MPI_COMM_WORLD, &status);
					array[endPoint - 1] = tempInt;
				} else if (partitionLength % 2 == 0 && processRank < processSize - 1) {
					// do nothing with no element left
				} else {
					// do nothing with largest rank
				}
			} else {
				// the first element is even
				for (int i = startPoint + 2; i < endPoint; i += 2) {
					if (array[i] < array[i - 1]) {
						swap(array, i, i - 1);
					}
				}
				if (partitionLength % 2 == 1 && processRank < processSize - 1) {
					if (processRank > 0) { //no receive and send back with process rank 0
						MPI_Recv(&tempInt, 1, MPI_INT, processRank - 1, 2, MPI_COMM_WORLD, &status);
						if (tempInt > array[startPoint]) {
							MPI_Send(&array[startPoint], 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
							array[startPoint] = tempInt;
						} else {
							MPI_Send(&tempInt, 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
						}
					}
				} else if (partitionLength % 2 == 0 && processRank < processSize - 1) {
					//receive the previous end and send back
					if (processRank > 0) { //no receive and send back with process rank 0
						MPI_Recv(&tempInt, 1, MPI_INT, processRank - 1, 2, MPI_COMM_WORLD, &status);
						if (tempInt > array[startPoint]) {
							MPI_Send(&array[startPoint], 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
							array[startPoint] = tempInt;
						} else {
							MPI_Send(&tempInt, 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
						}
					}
					// send out the end and receive
					MPI_Send(&array[endPoint - 1], 1, MPI_INT, processRank + 1, 2, MPI_COMM_WORLD);
					MPI_Recv(&tempInt, 1, MPI_INT, processRank + 1, 1, MPI_COMM_WORLD, &status);
					array[endPoint - 1] = tempInt;
				} else {
					// end of the process rank
					MPI_Recv(&tempInt, 1, MPI_INT, processRank - 1, 2, MPI_COMM_WORLD, &status);
					if (tempInt > array[startPoint]) {
						MPI_Send(&array[startPoint], 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
						array[startPoint] = tempInt;
					} else {
						MPI_Send(&tempInt, 1, MPI_INT, processRank - 1, 1, MPI_COMM_WORLD);
					}
				}
			}
		}
		trigger = (trigger + 1) % 2;
	
		
		/* if (processRank == 0) {
			int sortedBuf[processSize];
			int buffer;
			for (int i = 0; i < processSize; i++) {
				MPI_Recv(&buffer, 1, MPI_INT, i, 3, MPI_COMM_WORLD, processRank - 1);
				sortedBuf[i] = buffer;
			}
			sorted = 1;
			for (int i = 0; i < processSize; i++) {
				if (sortedBuf[i] == 0) {
					sorted = 0;
					break;
				}
			}
			//broadcast the result
			MPI_Bcast();
		} */
	}

	endTime = MPI_Wtime();

	MPI_Finalize();
	//information printed
	/* for (int i = 0; i < processSize; i++) {
		if (processRank == i) {
			printf("partition %d with subarray: ", processRank);
			for (int j = startPoint; j < endPoint; j++) {
				printf("%d ", array[j]);		
			}
		}
	}
	printf("\n"); */
	if (processRank == 0) {
		printf("Name: Yunteng Yang\n");
	    printf("student ID: 116010264\n");
		printf("size of the array: %d\n", arrayLength);
	    printf("assignment 1, odd-even sort, parallel implementation.\n");
		printf("usage of processes: %d", processSize);
	    printf("runTime is: %fms\n", (double) (endTime - startTime) * 1000); 
	}
}
