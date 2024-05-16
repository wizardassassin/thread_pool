#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "thread_pool.hpp"
#include "witness_calculations.hpp"

void test_code() {
    std::vector<int64_t> vect(91, 0);
    Witness::witness_count(vect, 91);
    std::set<int64_t> false_witnesses({1, 9, 10, 12, 16, 17, 22, 29, 38, 53, 62, 69, 74, 75, 79, 81, 82, 90});
    for (int64_t i = 0; i < 91; i++) {
        assert(vect[i] == 0 || vect[i] == 1);
        if (vect[i] == 0) assert(false_witnesses.find(vect[i]) == false_witnesses.end());
        if (vect[i] == 1) assert(false_witnesses.find(vect[i]) != false_witnesses.end());
    }

    auto primes = Witness::primeSieve(100);
    std::set<int64_t> the_primes(
        {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97});
    assert(primes == the_primes);
}

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;
    test_code();

    const int64_t max_val = 100000;
    auto primes = Witness::primeSieve(max_val);
    std::vector<int64_t> vect(max_val, 0);
    for (int64_t i = 3; i < max_val; i += 2) {
        if (primes.find(i) != primes.end()) continue;
        Witness::witness_count(vect, i);
    }

    std::vector<std::pair<int64_t, int64_t>> vect2(max_val);
    for (int64_t i = 0; i < max_val; i++) {
        vect2[i] = std::pair(i, vect[i]);
    }
    std::sort(vect2.begin(), vect2.end(), [](auto &a, auto &b) { return b.second < a.second; });

    std::ofstream file("witness_output.txt");
    for (int64_t i = 0; i < max_val; i++) {
        file << vect2[i].first << " " << vect2[i].second << "\n";
    }

    return 0;
}
