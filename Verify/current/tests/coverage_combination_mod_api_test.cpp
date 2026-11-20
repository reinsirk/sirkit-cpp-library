// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Combination_Mod.cpp"
#include <atcoder/modint>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {

using Index = std::int64_t;
using Uint = std::uint64_t;

// 分母を整数のまま約分する。法に対する逆元や因数分解を使わない。
std::uint32_t reference(Index n, Index k, std::uint32_t modulus) {
    if (n < 0 || k < 0 || k > n) return 0;
    const auto terms = static_cast<std::size_t>(std::min(k, n - k));
    std::array<Uint, 64> numerator{};
    assert(terms <= numerator.size());
    for (std::size_t i = 0; i < terms; ++i) {
        numerator[i] = static_cast<Uint>(n) - terms + i + 1;
    }
    for (Uint divisor = 2; divisor <= terms; ++divisor) {
        Uint remaining = divisor;
        for (std::size_t i = 0; i < terms && remaining > 1; ++i) {
            const auto common = std::gcd(numerator[i], remaining);
            numerator[i] /= common;
            remaining /= common;
        }
        assert(remaining == 1);
    }
    Uint result = 1 % modulus;
    for (std::size_t i = 0; i < terms; ++i) {
        result = result * (numerator[i] % modulus) % modulus;
    }
    return static_cast<std::uint32_t>(result);
}

template <class Error, class Function>
void rejects(Function function) {
    try {
        function();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

void integer_composite_paths() {
    constexpr std::array<std::uint32_t, 18> moduli{
        1, 4, 8, 9, 27, 125, 720, 2147483648u, 4000000000u, 4294967295u,
        2u * 998244353, 3u * 998244353, 7u * 167772161,
        2u * 65521 * 32749, 3u * 4099 * 65537, 7u * 17497 * 16381, 287, 3215031751u,
    };
    constexpr Index top = std::numeric_limits<Index>::max();
    constexpr std::array<Index, 9> rows{3, 8, 17, 50, 99, 144, 4294967297, top - 1, top};
    for (const auto modulus : moduli) {
        for (const std::uint32_t table : {0u, 7u, 64u}) {
            sirkit::BinomialMod binomial(modulus, 0, {table, 1024});
            assert(binomial.mod() == modulus);
            assert(!binomial.is_prime_modulus());
            for (const auto n : rows) {
                for (const Index k : {0, 1, 2, 3, 4, 6, 9, 17, 24}) {
                    const auto expected = reference(n, k, modulus);
                    assert(binomial.COM(n, k) == expected);
                    assert(binomial.COM(n, n - k) == expected);
                }
            }
            assert(binomial.H(0, 0) == 1 % modulus);
            assert(binomial.H(0, 12) == 0);
            assert(binomial.H(-1, 0) == 0);
            assert(binomial.H(11, -1) == 0);
            assert(binomial.H(30, 11) == reference(40, 11, modulus));
        }
    }

    // 準備済みの添字の偶奇を変えながら、素数冪用の表を伸ばす。
    for (const std::uint32_t modulus : {64u, 81u}) {
        sirkit::BinomialMod binomial(modulus, 1, {modulus - 1, 0});
        for (const Index bound : {3, 8, 17, 40, 81}) {
            binomial.prepare(bound);
            assert(binomial.COM(bound, 3) == reference(bound, 3, modulus));
        }
    }
}

template <int Modulus>
void large_tables(Index first, Index next) {
    using Mint = atcoder::static_modint<Modulus>;
    const sirkit::BinomialOptions options{10000, 4096};
    sirkit::BinomialMod integer(Modulus, first, options);
    sirkit::ModIntBinomial<Mint> wrapped(first, options);
    auto check = [&](Index n) {
        for (const Index k : {0, 1, 2, 3, 7, 18, 33}) {
            const auto expected = reference(n, k, Modulus);
            assert(integer.COM(n, k) == expected);
            assert(wrapped.COM(n, k).val() == static_cast<int>(expected));
        }
    };
    for (const Index n : {first - 1, first}) check(n);
    integer.prepare(next);
    wrapped.prepare(next);
    for (const Index n : {first, next - 1, next, next + Modulus}) check(n);
}

template <int Modulus>
void mint_digits() {
    using Mint = atcoder::static_modint<Modulus>;
    constexpr std::array<Index, 6> rows{2, 8, 26, 35, 70, 177};
    for (const std::uint32_t table : {0u, static_cast<std::uint32_t>(Modulus - 1)}) {
        sirkit::ModIntBinomial<Mint> wrapped(table, {table, 120});
        for (const auto n : rows) {
            for (Index k = 0; k <= std::min<Index>(n, 31); ++k) {
                const auto expected = static_cast<int>(reference(n, k, Modulus));
                assert(wrapped.COM(n, k).val() == expected);
                assert(wrapped.Lucas(n, k).val() == expected);
            }
        }
    }
}

void checked_failures() {
    constexpr Index top = std::numeric_limits<Index>::max();
    using Mint = atcoder::dynamic_modint<23>;
    Mint::set_mod(101);
    sirkit::ModIntBinomial<Mint> wrapped(0, {0, 0});
    sirkit::BinomialMod integer(101, 0, {0, 0});

    rejects<std::length_error>([&] { (void)integer.COM(10, 3); });
    rejects<std::length_error>([&] { (void)wrapped.COM(10, 3); });
    rejects<std::length_error>([&] { integer.prepare(2); });
    rejects<std::length_error>([&] { wrapped.prepare(2); });
    rejects<std::overflow_error>([&] { (void)integer.H(top, 2); });
    rejects<std::overflow_error>([&] { (void)wrapped.H(top, 2); });
    assert(integer.COM(101, 50) == 0);
    assert(wrapped.COM(101, 50).val() == 0);
    assert(wrapped.H(-1, 0).val() == 0);
    assert(wrapped.H(11, -1).val() == 0);
    assert(integer.COM(11, 1) == 11);
    assert(wrapped.COM(11, 1).val() == 11);

#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([] { (void)sirkit::BinomialMod(0, 0); });
    rejects<std::invalid_argument>([] { (void)sirkit::BinomialMod(4294967296, 0); });
    rejects<std::invalid_argument>([&] { integer.prepare(-1); });
    rejects<std::invalid_argument>([&] { wrapped.prepare(-1); });
    Mint::set_mod(72);
    rejects<std::logic_error>([&] { (void)wrapped.COM(2, 1); });
    rejects<std::logic_error>([&] { (void)wrapped.H(2, 1); });
    rejects<std::logic_error>([&] { wrapped.prepare(0); });
    sirkit::BinomialMod composite(72, 0);
    sirkit::ModIntBinomial<Mint> wrapped_composite(0);
    rejects<std::domain_error>([&] { (void)composite.Lucas(2, 1); });
    rejects<std::domain_error>([&] { (void)wrapped_composite.Lucas(2, 1); });
    assert(wrapped_composite.COM(20, 7).val() == static_cast<int>(reference(20, 7, 72)));
    Mint::set_mod(101);
    assert(wrapped.COM(11, 1).val() == 11);
#endif
}

} // namespace

int main() {
    integer_composite_paths();
    large_tables<998244353>(4097, 8197);
    large_tables<5003>(4097, 5002);
    large_tables<5003>(5002, 5002);
    mint_digits<2>();
    mint_digits<3>();
    mint_digits<37>();
    checked_failures();
}
