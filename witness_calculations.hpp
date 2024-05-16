#pragma once

#include <cmath>
#include <set>
#include <vector>

// https://www.youtube.com/watch?v=_MscGSN5J6o
// https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test
namespace Witness {
std::set<int64_t> primeSieve(int64_t max_val) {
    std::set<int64_t> primeSet;
    std::vector<bool> primeBit(max_val, false);
    int64_t i = 2;
    int64_t i_end = std::sqrt(max_val);
    while (i < i_end) {
        if (!primeBit[i]) {
            primeSet.insert(i);
            for (int64_t j = i * i; j < max_val; j += i) primeBit[j] = true;
        }
        i++;
    }
    while (i < max_val) {
        if (!primeBit[i]) primeSet.insert(i);
        i++;
    }
    return primeSet;
}

// https://www.youtube.com/watch?v=cbGB__V8MNk
// https://en.wikipedia.org/wiki/Exponentiation_by_squaring
int64_t powerMod(int64_t x, int64_t n, int64_t mod_val) {
    // assume n > 0
    int64_t y = 1;
    while (n > 1) {
        if (n % 2 == 1) {
            y = (x * y) % mod_val;
            n--;
        }
        x = (x * x) % mod_val;
        n /= 2;
    }
    return (x * y) % mod_val;
}

bool is_false_witness(int64_t a, int64_t s, int64_t d, int64_t n) {
    int64_t y = powerMod(a, d, n);
    if (y == 1 || y == n - 1) return true;
    int64_t r = 1;
    while (r < s) {
        y = (y * y) % n;
        if (y == n - 1) return true;
        r++;
    }
    return false;
}

int64_t remove_twos(int64_t& n) {
    int64_t s = 0;
    while (n % 2 == 0) {
        s++;
        n /= 2;
    }
    return s;
}

void witness_count(std::vector<int64_t>& vect, int64_t n) {
    // assume no out of bounds errors
    int64_t d = n - 1;
    int64_t s = remove_twos(d);
    for (int64_t i = 1; i < n; i++)
        if (is_false_witness(i, s, d, n)) vect[i]++;
}
}  // namespace Witness
