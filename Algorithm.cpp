#include "Algorithm.h"
namespace ARLib {
namespace detail {
    constexpr inline size_t max_int32 = 2147483647;
    size_t legendre(size_t a, size_t m) {
        return static_cast<size_t>(pow(static_cast<double>(a), static_cast<double>((m - 1ull) >> 1ull))) % m;
    }
    bool is_sprp(size_t n, size_t b = 2) {
        size_t d = n - 1;
        size_t s = 0;
        while ((d & 1) == 0) {
            s += 1;
            d >>= 1;
        }

        size_t x = static_cast<size_t>(pow(static_cast<double>(b), static_cast<double>(d))) % n;
        if (x == 1 || x == n - 1) return true;

        for (size_t r = 1; r < s; r++) {
            x = (x * x) % n;
            if (x == 1)
                return false;
            else if (x == n - 1)
                return true;
        }
        return false;
    }
    bool is_lucas_prp(size_t n, size_t D) {
        size_t Q = (1 - D) >> 2;

        // n + 1 = 2 * *r * s where s is odd
        size_t s = n + 1;
        size_t r = 0;
        while ((s & 1) == 0) {
            r += 1;
            s >>= 1;
        }

        // calculate the bit reversal of(odd) s
        // e.g. 19 (10011) <=> 25 (11001)
        size_t t = 0;
        while (s > 0) {
            if (s & 1) {
                t += 1;
                s -= 1;
            } else {
                t <<= 1;
                s >>= 1;
            }
        }

        // use the same bit reversal process to calculate the sth Lucas number
        // keep track of q = Q * *n as we go
        size_t U = 0;
        size_t V = 2;
        size_t q = 1;
        // mod_inv(2, n)
        size_t inv_2 = (n + 1) >> 1;
        while (t > 0) {
            if ((t & 1) == 1) {
                // U, V of n + 1
                U = ((U + V) * inv_2) % n;
                V = ((D * U + V) * inv_2) % n;
                q = (q * Q) % n;
                t -= 1;
            } else {
                // U, V of n * 2
                U = (U * V) % n;
                V = (V * V - 2 * q) % n;
                q = (q * q) % n;
                t >>= 1;
            }
        }
        // double s until we have the 2 * *r * sth Lucas number
        while (r > 0) {
            U = (U * V) % n;
            V = (V * V - 2 * q) % n;
            q = (q * q) % n;
            r -= 1;
        }

        // primality check
        // if n is prime, n divides the n+1st Lucas number, given the assumptions
        return U == 0;
    }
    // primes less than 212
    constexpr uint8_t small_primes[] = { 2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,
                                         59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131,
                                         137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211 };

    // pre - calced sieve of eratosthenes for n = 2, 3, 5, 7
    constexpr uint8_t indices[] = { 1,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,  61,  67,
                                    71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 121, 127, 131, 137, 139,
                                    143, 149, 151, 157, 163, 167, 169, 173, 179, 181, 187, 191, 193, 197, 199, 209 };

    // distances between sieve values
    constexpr uint8_t offsets[] = { 10, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2,  4,
                                    2,  4, 8, 6, 4, 6, 2, 4, 6, 2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2, 4, 2, 10, 2 };
    bool is_prime(size_t n) {
        if (n < 212) {
            for (auto sm : small_primes)
                if (n == sm) return true;
            return false;
        }

        for (auto p : small_primes)
            if ((n % p) == 0) return false;

        // if n is a 32-bit integer, perform full trial division
        if (n <= max_int32) {
            size_t i = 211;
            while ((i * i) < n) {
                for (auto o : offsets) {
                    i += o;
                    if ((n % i) == 0) return false;
                }
            }
            return true;
        }

        // Baillie - PSW
        // this is technically a probabalistic test, but there are no known pseudoprimes
        if (!is_sprp(n)) return false;
        size_t a   = 5;
        intptr_t s = 2;
        while (legendre(a, n) != n - 1) {
            s = -s;
            a = static_cast<size_t>(s) - a;
            return is_lucas_prp(n, a);
        }
        unreachable
    }
    size_t generate_next_prime(size_t n) {
        if (n < 2) return 2;

        // first odd larger than n
        n = (n + 1) | 1;
        if (n < 212) {
            while (true) {
                for (auto sm : small_primes)
                    if (n == sm) return n;
                n += 2;
            }
        }

        // find our position in the sieve rotation via binary search
        size_t x = n % 210ull;
        size_t s = 0;
        size_t e = 47;
        size_t m = 24;
        while (m != e) {
            if (indices[m] < x) {
                s = m;
                m = (s + e + 1) >> 1;
            } else {
                e = m;
                m = (s + e) >> 1;
            }
        }
        size_t l = n + (indices[m] - x);
        // adjust offsets
        while (true) {
            for (size_t i = m; i < sizeof_array(offsets); i++) {
                if (is_prime(l)) return l;
                l += offsets[i];
            }
            for (size_t i = 0; i < m; i++) {
                if (is_prime(l)) return l;
                l += offsets[i];
            }
        }
    }
}    // namespace detail
size_t prime_generator(size_t n) {
    constexpr size_t max_prime_size_t_ = 18446744073709551557llu;
    if (n < max_prime_size_t_) return detail::generate_next_prime(n);
    return max_prime_size_t_;
}
}    // namespace ARLib
