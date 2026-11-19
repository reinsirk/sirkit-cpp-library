// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_FORCE_STANDARD_ARITHMETIC
#define SIRKIT_FORCE_STANDARD_ARITHMETIC
#endif
#include "../../../Math/Combination/Prime_Binomial.cpp"

#include <atcoder/modint.hpp>
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace {
using Mint = atcoder::dynamic_modint<205>;

bool trial(std::uint32_t number) {
    if (number < 2) return false;
    for (std::uint32_t divisor = 2; divisor <= number / divisor; ++divisor) {
        if (number % divisor == 0) return false;
    }
    return true;
}

void check_modulus(int mod) {
    const bool prime = trial(mod);
    Mint::set_mod(mod);
    try {
        sirkit::PrimeBinomial<Mint> choose(0, 6);
        assert(prime);
        // C(p+3,2) = (p+3)(p+2)/2。p=2も整数として先に割る。
        const auto n = std::int64_t{mod} + 3;
        assert(choose.COM(n, 2).val() == (n * (n - 1) / 2) % mod);
        assert(choose.H(-1, 3).val() == 0);
        assert(choose.H(3, -1).val() == 0);
    } catch (const std::domain_error&) {
        assert(!prime);
    }
}
}

int main() {
#ifndef SKIP_RUNTIME_CHECKS
    for (std::uint32_t n = 0; n <= 2048; ++n)
        assert(sirkit::detail::is_prime_u32(n) == trial(n));
    for (int mod = 1; mod <= 2048; ++mod) check_modulus(mod);
    for (int mod : {1000000007, 1000000009, 1004535809, 1224736769, 2013265921,
                    1373653, 25326001, 2147483646, 2147483647}) {
        check_modulus(mod);
    }
    for (const std::uint32_t n : {3215031751u, 4294967291u, 4294967293u, 4294967295u})
        assert(sirkit::detail::is_prime_u32(n) == trial(n));
#else
    for (int mod : {2, 61, 97, 1000000007, 2147483647}) check_modulus(mod);
#endif
}
