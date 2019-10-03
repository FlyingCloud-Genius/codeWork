#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

using namespace std;

int main() {
    int array_length = {10, 100, 1000, 10000, 100000, 1000000};
    for (int i = 0; i < array_length.length; i++) {
        int[] array = new int[array_length];
        for (int i = 0; i < array.length; i++) {
            array[i] = rand(time(0)) % 1000;
        }

        time_t start_time = time(0);
        sequentialOddEvenSort(array);
        time_t end_time = time(0);

        cout << "Name: Yunteng Yang" << endl;
        cout << "student ID: 116010264" << endl;
        cout << "assignment 1, odd-even sort, sequential implementation." << endl;
        cout << "runTime is " << start_time - end_time << endl;
    }
}

void sequentialOddEvenSort(int[] array) {
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        //odd sort
        for (int i = 1; i <array.length; i += 2) {
            if (array[i] > array[i - 1]) {
                swap(array, i, i - 1);
                sorted = false;
            }
        }

        //even sort
        for (int i = 2; i < array.length; i+= 2) {
            if (array[i] > array[i - 1]) {
                swap(array, i, i - 1);
                sorted = false;
            }
        }
    }
}

void swap(int[] array, int index1, int index2) {
    int temp = array[index1];
    array[index1] = array[index2]
    array[index2] = temp;
}


