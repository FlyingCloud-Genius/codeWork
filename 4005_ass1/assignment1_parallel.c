#include <iostream>
#include <time.h>
#include <mpi.h>

using namespace std;

void parallelOddSort(int array[], int length) {
    
}

void parallelEvenSort(int array[], int length) {

}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int process_size;
    MPI_Comm_size(MPI_COMM_WORLD, &process_size);

    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

	int array_length[7] = {10, 100, 1000, 10000, 100000, 1000000, 10000000};
	clock_t start_time, end_time;
	int length;
    for (int i = 0; i < sizeof(array_length) / sizeof (array_length[0]); i++) {
		length = array_length[i];
        int array[length];
		srand(time(0));
        for (int j = 0; j < length; j++) {
            array[j] = rand() % 1000;
        }

        start_time = clock();

        //executing parallel odd-even sort

        if (process_size < 2) {
            fprintf(stderr, "Process size must be greater than 1 for %s\n", process_size);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        parallelOddEvenSort(array, length);

        end_time = clock();

        cout << "Name: Yunteng Yang" << endl;
        cout << "student ID: 116010264" << endl;
		cout << "size of the array: " << array_length[i] << endl;
        cout << "assignment 1, odd-even sort, sequential implementation." << endl;
        cout << "runTime is " << (float) (end_time - start_time) * 1000 / CLOCKS_PER_SEC << " ms" << endl;
    }
}
