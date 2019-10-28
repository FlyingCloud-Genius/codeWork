#include <omp.h>

void BigCalculationOne()
{
}

void BigCalculationTwo()
{
}

void ApplyBoundaryConditions()
{
}

int main(void)
{
#pragma omp parallel
    {
        BigCalculationOne();
#pragma omp single
        {
            ApplyBoundaryConditions();
        }
        BigCalculationTwo();
    }
    return 0;
}
