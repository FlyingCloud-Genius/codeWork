#include <time.h>
#include <iostream>

using namespace std;

void swap(int* array, int index1, int index2) {
    int temp = array[index1];
    array[index1] = array[index2];
    array[index2] = temp;
}

void sequentialOddEvenSort(int array[], int length) {
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        //odd sort
        for (int i = 1; i < length; i += 2) {
            if (array[i] < array[i - 1]) {
                swap(array, i, i - 1);
                sorted = false;
            }
        }

        //even sort
        for (int i = 2; i < length; i += 2) {
            if (array[i] < array[i - 1]) {
                swap(array, i, i - 1);
                sorted = false;
            }
        }
    }
}

int main() {
    int array_length[5] = {10, 100, 1000, 10000, 100000};
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
        sequentialOddEvenSort(array, length);
        end_time = clock();

        cout << "Name: Yunteng Yang" << endl;
        cout << "student ID: 116010264" << endl;
		cout << "size of the array: " << array_length[i] << endl;
        cout << "assignment 1, odd-even sort, sequential implementation." << endl;
        cout << "runTime is " << (float) (end_time - start_time) * 1000 / CLOCKS_PER_SEC << " ms" << endl;
    }
}

