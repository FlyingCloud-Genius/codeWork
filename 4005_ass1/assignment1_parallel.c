#include <iostream>
#include <time.h>
#include <mpi.h>

using namespace std;

void parallelOddEvenSort(int array[], int length) {

}


int main() {
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
        parallelOddEvenSort(array, length);
        end_time = clock();

        cout << "Name: Yunteng Yang" << endl;
        cout << "student ID: 116010264" << endl;
		cout << "size of the array: " << array_length[i] << endl;
        cout << "assignment 1, odd-even sort, sequential implementation." << endl;
        cout << "runTime is " << (float) (end_time - start_time) * 1000 / CLOCKS_PER_SEC << " ms" << endl;
    }
}
