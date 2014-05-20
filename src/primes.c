#include "lib/primes.h"

int primes_is_prime(unsigned long x)
{
    unsigned long i, q;
    for (i = 3; 1; i += 2)
    {
        q = x / i;
        if (q < i)
            return 1;
        if (x == q * i)
            return 0;
    }
    
    return 1;
}

unsigned long primes_next_prime(unsigned long x)
{
    if (x <= 2)
        return 2;
    if (!(x & 1))
        ++x;
    for (; !primes_is_prime(x); x += 2);
    
    return x;
}
