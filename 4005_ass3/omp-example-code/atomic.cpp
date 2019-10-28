#include <omp.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;

#include "randomMT.h"

float force(const float x)
{
    return -2. * atan(x) / (x * x + 1.);
}

int main(void)
{
    const int n = 64;
    vector<float> x(n);

/* Generate random points on the unit interval */
#pragma omp parallel
    {
        const long tid = omp_get_thread_num();
        randMT r(4357U + unsigned(tid));
/* Thread safe generation of random numbers */
#pragma omp for
        for (int i = 0; i < n; ++i)
        {
            x[i] = r.rand();
        }
    }

    /* Compute interaction forces between particles.
     Atomic is used. */
    vector<float> f(n);
#pragma omp parallel for
    for (int i = 0; i < n; ++i)
    {
        f[i] = 0.;
    }

#pragma omp parallel for
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
        {
            const float x_ = x[i] - x[j];
            const float f_ = force(x_);
#pragma omp atomic
            f[i] += f_;
#pragma omp atomic
            f[j] -= f_;
        }

    // Test
    {
        vector<float> f0(n, 0.);

        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
            {
                const float x_ = x[i] - x[j];
                const float f_ = force(x_);
                f0[i] += f_;
                f0[j] -= f_;
            }

        for (int i = 0; i < n; ++i)
        {
            assert(abs(f0[i] - f[i]) < 1e-4);
        }

        cout << "First test passed.\n";
    }

// Without the use of atomic
#pragma omp parallel for
    for (int i = 0; i < n; ++i)
    {
        f[i] = 0.;
    }

#pragma omp parallel for
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
        {
            const float x_ = x[i] - x[j];
            const float f_ = force(x_);
            f[i] += f_;
            f[j] -= f_;
        }

    // Test
    {
        cout << "Calculation without atomic.\n";
        cout << "Second test is expected to fail.\n";
        vector<float> f0(n, 0.);

        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
            {
                const float x_ = x[i] - x[j];
                const float f_ = force(x_);
                f0[i] += f_;
                f0[j] -= f_;
            }

        int error_printout_max = 5;
        int n_error = 0;
        for (int i = 0; i < n; ++i)
            if (!(abs(f0[i] - f[i]) < 1e-4) && n_error < error_printout_max)
            {
                printf("f omp, f exact, error: %g %g %g\n", f0[i], f[i], abs(f0[i] - f[i]));
                ++n_error;
            }
            else if (!(abs(f0[i] - f[i]) < 1e-4) && n_error == error_printout_max)
            {
                printf("... (skipping remaining errors)\n");
                ++n_error;
            }
    }

    return 0;
}
