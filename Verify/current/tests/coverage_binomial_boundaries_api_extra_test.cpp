// SPDX-License-Identifier: CC0-1.0
#include <atcoder/modint.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#ifdef SIRKIT_TEST_PRIME_BINOMIAL_BOUNDARIES
#include "../../../Math/Combination/Prime_Binomial.cpp"
#else
#include "../../../Math/Combination/ModInt_Binomial.cpp"
#endif

namespace {
using Index = std::int64_t;
using Modulus = std::uint32_t;
using Mint = atcoder::dynamic_modint<158>;
constexpr int kSmallK = 6;

// 割り算を使わず、パスカルの漸化式で各行の先頭を求める。
auto pascal_prefix(int bound, Modulus mod) {
    std::vector<std::array<Modulus, kSmallK + 1>> rows(bound + 1);
    rows[0][0] = 1;
    for (int n = 1; n <= bound; ++n) {
        rows[n][0] = 1;
        for (int k = 1; k <= std::min(n, kSmallK); ++k) {
            rows[n][k] = (std::uint64_t{rows[n - 1][k - 1]} + rows[n - 1][k]) % mod;
        }
    }
    return rows;
}

template<class Calculator, class Unwrap>
void check_growth(Calculator& calculator, Unwrap unwrap, Modulus mod) {
    const auto expected = pascal_prefix(mod - 1, mod);
    // 4要素ずつ埋める範囲、2要素ずつ埋める範囲、法-1までの拡張を通す。
    for (int bound : {4101, 9000, static_cast<int>(mod) - 1}) {
        calculator.prepare(bound);
        for (int n : {bound - 1, bound, 31}) {
            for (int k = 0; k <= kSmallK; ++k) {
                assert(unwrap(calculator.COM(n, k)) == expected[n][k]);
                assert(unwrap(calculator.Lucas(n, n - k)) == expected[n][k]);
            }
        }
    }
    assert(unwrap(calculator.H(7, 4)) == expected[10][4]);
    assert(unwrap(calculator.H(0, 0)) == 1u);
    bool overflow = false;
    try {
        (void)calculator.H(std::numeric_limits<Index>::max(), 2);
    } catch (const std::overflow_error&) {
        overflow = true;
    }
    assert(overflow);
}
}

int main() {
    constexpr int kPrime = 20011;
    const auto unwrap_mint = [](Mint value) { return static_cast<Modulus>(value.val()); };
    Mint::set_mod(kPrime);

#ifdef SIRKIT_TEST_PRIME_BINOMIAL_BOUNDARIES
    const auto capacity = std::numeric_limits<std::size_t>::max();
    assert(sirkit::detail::prime_binomial_table_bound(capacity - 1, capacity) == capacity - 1);
    bool capacity_rejected = false;
    try {
        (void)sirkit::detail::prime_binomial_table_bound(capacity, capacity);
    } catch (const std::length_error&) {
        capacity_rejected = true;
    }
    assert(capacity_rejected);

    sirkit::PrimeBinomial<Mint> calculator(2, 100);
    check_growth(calculator, unwrap_mint, kPrime);
    sirkit::PrimeBinomial<Mint> even_start(3, 0);
    even_start.prepare(kPrime - 1);
    assert(even_start.COM(kPrime - 1, 2).val() == 1);
    for (int prime : {167772161, 469762049, 754974721}) {
        Mint::set_mod(prime);
        sirkit::PrimeBinomial<Mint> large_prime(0, 4);
        assert(large_prime.COM(Index{prime} + 4, 2).val() == 6);
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (int composite : {169, 1763, 1373653}) {
        Mint::set_mod(composite);
        bool rejected = false;
        try {
            (void)sirkit::PrimeBinomial<Mint>(0);
        } catch (const std::domain_error&) {
            rejected = true;
        }
        assert(rejected);
    }
#endif
#else
    sirkit::ModIntBinomial<Mint> calculator(2, {kPrime - 1, 100});
    check_growth(calculator, unwrap_mint, kPrime);
    sirkit::BinomialMod integer(kPrime, 2, {kPrime - 1, 100});
    check_growth(integer, [](Modulus value) { return value; }, kPrime);

    // 大きな素因子を持つ偶数・奇数・小素数で割れない合成法を照合する。
    constexpr Modulus kLargePrime = 167772161;
    for (Modulus mod : {2 * kLargePrime, 3 * kLargePrime, 5 * kLargePrime, 7 * kLargePrime,
                        469762049u, 754974721u, 998244353u, 531441u}) {
        const auto expected = pascal_prefix(173, mod);
        for (Modulus limit : {0u, 400u}) {
            sirkit::BinomialMod composite(mod, limit, {limit, 100});
            for (int n : {51, 62, 94, 121, 173}) {
                for (int k = 2; k <= kSmallK; ++k) {
                    assert(composite.COM(n, k) == expected[n][k]);
                }
            }
        }
    }

    // 3桁すべてへ非自明な係数を配置して完全な階乗表を使用する。
    for (Modulus prime : {19u, 23u, 29u, 31u, 37u}) {
        const auto expected = pascal_prefix(prime - 1, prime);
        sirkit::BinomialMod lucas(prime, sirkit::BinomialOptions{prime - 1, 0});
        const Index square = Index{prime} * prime;
        const Index n = 7 * square + 9 * prime + 11;
        const Index k = 2 * square + 4 * prime + 5;
        const auto result = (std::uint64_t{expected[7][2]} * expected[9][4] * expected[11][5]) % prime;
        assert(lucas.Lucas(n, k) == result);
        assert(lucas.COM(n, n - k) == result);
        assert(lucas.COM(n, k + 5 * prime + prime) == 0);
    }
#endif
}
