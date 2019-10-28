#include <omp.h>
#include <cstdio>
#include <cassert>
#include <set>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;

bool LongCalculation(int n)
{
    bool flag = true;

    // Skip 1 and even numbers
    if (n == 1 || (n % 2 == 0 && n != 2))
    {
        return false;
    }

    for (int i = 3; i <= n / 2; i += 2)
    {
        if (n % i == 0)
        {
            flag = false; // Not a prime
            break;
        }
    }

    return flag; // Must be prime
}

void Consume(int p, set<int> &m)
{
    m.insert(p);
}

int main(void)
{
    const int n = 1 << 16;
    // Calculate all prime numbers smaller than n

    set<int> m;
#pragma omp parallel
    {
        const int id = omp_get_thread_num();
        const int n_threads = omp_get_num_threads();

        for (int i(id); i < n; i += n_threads)
        {
            // A different way to write a parallel for loop
            bool is_prime = LongCalculation(i);

// Comment the line below to get a bug
#pragma omp critical
            if (is_prime)
            {
                Consume(i, m); /* Save this prime */
            }
        }
    }

    printf("Number of prime numbers smaller than %d: %ld\n", n, m.size());

    // Check
    {
        set<int> m0;

        for (int i(0); i < n; ++i)
            if (LongCalculation(i) == 1)
            {
                Consume(i, m0);
            }

        {
            /* Read primes from a file to test m0
             Checking only the first 10,000 primes. */
            ifstream prime_file("10000.txt");
            auto it = m0.begin();
            int count = 0; /* Read only the first 10,000 primes */

            while (it != m0.end() && count < 10000)
            {
                int next_prime;
                prime_file >> next_prime;  // Read from file
                assert(*it == next_prime); // Test
                ++it;
                ++count;
            }

            prime_file.close();
        }

        // Checking that m == m0
        assert(equal(m0.begin(), m0.end(), m.begin()));

        printf("All tests passed.\n");
    }
    return 0;
}
