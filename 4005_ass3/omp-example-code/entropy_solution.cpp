#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

void print_vector(vector<float> &vec)
{
    const unsigned n = vec.size();
    printf("(");
    for (unsigned i = 0; i < n - 1; ++i)
        printf("%7.4f,", vec[i]);
    printf("%7.4f)\n", vec[n - 1]);
}

int main(void)
{
    const unsigned size = 9;
    vector<float> entropy0(size), entropy1(size);

#pragma omp parallel for
    for (unsigned i = 0; i < size; i++)
    {
        float d = i - float(size - 1) / 2.;
        entropy0[i] = exp(-d * d / 0.5);
        entropy1[i] = exp(-d * d / 10.);
    }

    float sum1 = 0,
          sum2 = 0;
#pragma omp parallel for reduction(+ \
                                   : sum1, sum2)
    for (unsigned i = 0; i < size; i++)
    {
        sum1 += entropy0[i];
        sum2 += entropy1[i];
    }

#pragma omp parallel for
    for (unsigned i = 0; i < size; i++)
    {
        entropy0[i] /= sum1;
        entropy1[i] /= sum2;
    }

    print_vector(entropy0);
    print_vector(entropy1);

    float ent1 = 0, ent2 = 0;
#pragma omp parallel for reduction(+ \
                                   : ent1, ent2)
    for (unsigned i = 0; i < size; i++)
    {
        ent1 += entropy0[i] * log(entropy0[i]);
        ent2 += entropy1[i] * log(entropy1[i]);
    }

    ent1 = -ent1;
    ent2 = -ent2;

    printf("Change in entropy: %g\n", ent2 - ent1);

    return 0;
}
