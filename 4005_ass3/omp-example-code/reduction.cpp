#include <omp.h>
#include <vector>
#include <cassert>
#include <iostream>
using namespace std;

int main(void) {
    int size = 1<<10;
    vector<float> a(size);
    #pragma omp parallel for
    for(int i = 0; i < size; i++) {
        a[i] = i;
    }

    float sum = 0.0;

    //#pragma omp parallel for
    #pragma omp parallel for reduction (+:sum)
    for(int i = 0; i < size; i++) {
        sum += a[i];
    }

    assert(sum == (size*(size-1))/2);
    cout << "All tests passed\n";
    return 0;
}
