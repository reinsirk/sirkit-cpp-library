// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main() {
    std::int64_t n{}; verify_in >> n;
    int bound = 1;
    while (std::int64_t{bound} * bound < n) ++bound;
    IntegerSieve sieve(bound);
    int nim_sum = 0;
    for (const auto& factor : sieve.prime_factorization(n)) nim_sum ^= factor.second;
    verify_out << (nim_sum ? "Alice\n" : "Bob\n");
}
