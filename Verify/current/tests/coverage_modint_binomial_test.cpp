// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Combination/ModInt_Binomial.cpp"
#include <atcoder/modint.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <source_location>
#include <stdexcept>
#include <vector>

namespace {
using Index = std::int64_t;
using Int = std::uint64_t;

void require(bool condition, std::source_location where = std::source_location::current()) {
    if (condition) return;
    std::cerr << where.file_name() << ':' << where.line() << '\n';
    std::abort();
}

template<class Exception, class Operation>
void rejects(Operation operation, std::source_location where = std::source_location::current()) {
    try {
        operation();
    } catch (const Exception&) {
        return;
    }
    require(false, where);
}

using Pascal = std::vector<std::vector<Int>>;

Pascal pascal(int bound, Int mod) {
    Pascal table(bound + 1, std::vector<Int>(bound + 1));
    table[0][0] = 1 % mod;
    for (int n = 1; n <= bound; ++n) {
        table[n][0] = 1 % mod;
        for (int k = 1; k <= n; ++k) {
            table[n][k] = (table[n - 1][k - 1] + table[n - 1][k]) % mod;
        }
    }
    return table;
}

Int choose_small_k(Int n, Int k) {
    Int result = 1;
    for (Int i = 1; i <= k; ++i) result = result * (n - i + 1) / i;
    return result;
}

Int lucas_pascal(Int n, Int k, Int prime, const Pascal& table) {
    Int answer = 1;
    while (n || k) {
        answer = answer * table[n % prime][k % prime] % prime;
        n /= prime;
        k /= prime;
    }
    return answer;
}

using Dynamic = atcoder::dynamic_modint<832>;

void check_composite_paths() {
    // 2の冪、奇素数冪、複数素因子の法を、表あり・部分表・積だけで比較する。
    constexpr int moduli[] = {
        1, 4, 8, 9, 16, 25, 27, 32, 49, 81, 121, 125, 128, 169, 256, 343, 625,
        6, 12, 18, 72, 108, 210, 900, 1728, 72072, 1073741824, 1162261467, 2147483647,
    };
    for (const auto mod : moduli) {
        Dynamic::set_mod(mod);
        const auto reference = pascal(92, mod);
        for (std::uint32_t limit : {0u, 7u, 92u}) {
            const sirkit::BinomialOptions options{limit, 10000};
            sirkit::ModIntBinomial<Dynamic> mint(0, options);
            sirkit::BinomialMod integer(mod, 0, options);
            require(integer.mod() == static_cast<Int>(mod));
            require(integer.is_prime_modulus() == (mod == 2147483647));
            mint.prepare(limit / 2);
            integer.prepare(limit / 2);
            for (int n = 0; n <= 92; ++n) {
                for (int k = 0; k <= n; ++k) {
                    require(static_cast<Int>(mint.COM(n, k).val()) == reference[n][k]);
                    require(integer.COM(n, k) == reference[n][k]);
                    if (n + k <= 92 && n > 0 && k > 0) {
                        require(static_cast<Int>(mint.H(n, k).val()) == reference[n + k - 1][k]);
                    }
                }
            }
            require(mint.COM(-1, 0).val() == 0);
            require(mint.COM(1, -1).val() == 0);
            require(mint.COM(1, 2).val() == 0);
            require(mint.H(-1, 0).val() == 0);
            require(mint.H(1, -1).val() == 0);
            require(mint.H(0, 1).val() == 0);
            require(mint.H(0, 0).val() == 1 % mod);
            require(integer.H(0, 0) == static_cast<Int>(1 % mod));
            require(integer.H(0, 1) == 0);
            require(integer.H(-1, 0) == 0);
#ifndef SKIP_RUNTIME_CHECKS
            if (!integer.is_prime_modulus()) {
                rejects<std::domain_error>([&] { (void)mint.Lucas(8, 2); });
                rejects<std::domain_error>([&] { (void)integer.Lucas(8, 2); });
            }
#endif
        }
    }

    constexpr Int mod = std::numeric_limits<std::uint32_t>::max();
    sirkit::BinomialMod full(mod, 200, {200, 1000});
    for (Int n : {64, 121, 511, 1024, 2048}) {
        for (Int k = 0; k <= 4; ++k) {
            require(full.COM(n, k) == choose_small_k(n, k) % mod);
        }
    }
}

template<int Mod>
void check_lucas() {
    using Mint = atcoder::static_modint<Mod>;
    const auto digits = pascal(Mod - 1, Mod);
    for (const std::uint32_t limit : {0u, static_cast<std::uint32_t>(Mod - 1)}) {
        sirkit::ModIntBinomial<Mint> comb(limit, {limit, 10000});
        std::mt19937_64 generator(11817 + Mod);
        for (int trial = 0; trial < 1200; ++trial) {
            const auto n = generator() >> 1;
            const auto k = trial % 2 ? generator() % (n + 1) : std::min<Int>(n, 2 + trial % 20);
            const auto expected = lucas_pascal(n, k, Mod, digits);
            require(static_cast<Int>(comb.COM(n, k).val()) == expected);
            require(static_cast<Int>(comb.Lucas(n, n - k).val()) == expected);
        }
        for (Index n = 1; n <= 6 * Mod; ++n) {
            for (Index k = 0; k <= n; ++k) {
                require(static_cast<Int>(comb.COM(n, k).val()) == lucas_pascal(n, k, Mod, digits));
            }
        }
    }
    sirkit::ModIntBinomial<Mint> defaults(sirkit::BinomialOptions{Mod - 1, 0});
    defaults.prepare();
    require(static_cast<Int>(defaults.COM(2 * Mod + 1, Mod).val()) == lucas_pascal(2 * Mod + 1, Mod, Mod, digits));
}

template<int Mod>
void check_growth(Index initial, Index intermediate) {
    using Mint = atcoder::static_modint<Mod>;
    sirkit::ModIntBinomial<Mint> comb(initial, {Mod - 1, 12});
    std::mt19937 generator(711);
    for (const auto bound : {intermediate, static_cast<Index>(Mod - 1)}) {
        comb.prepare(bound);
        comb.prepare(bound - 1);
        for (int trial = 0; trial < 200; ++trial) {
            const Int n = trial < 15 ? static_cast<Int>(bound - trial) : generator() % bound + 4;
            for (Int k = 0; k <= 4; ++k) {
                const auto expected = choose_small_k(n, k) % Mod;
                require(static_cast<Int>(comb.COM(n, k).val()) == expected);
                require(static_cast<Int>(comb.Lucas(n, n - k).val()) == expected);
            }
        }
    }
}

void check_errors() {
    using Mint = atcoder::static_modint<97>;
    constexpr auto maximum = std::numeric_limits<Index>::max();
    sirkit::ModIntBinomial<Mint> comb(0, {0, 3});
    const auto reference = pascal(96, 97);
    require(static_cast<Int>(comb.COM(90, 2).val()) == reference[90][2]);
    require(static_cast<Int>(comb.COM(90, 3).val()) == reference[90][3]);
    require(static_cast<Int>(comb.COM(5 * 97 + 5, 97 + 3).val()) == 50);
    require(comb.COM(97 * 97 + 2 * 97 + 96, 3 * 97 + 20).val() == 0);
    rejects<std::length_error>([&] { (void)comb.COM(90, 4); });
    rejects<std::length_error>([&] { (void)comb.Lucas(5 * 97 + 5, 2 * 97 + 2); });
    rejects<std::length_error>([&] { comb.prepare(1); });
    rejects<std::overflow_error>([&] { (void)comb.H(maximum, 2); });
    require(comb.H(maximum, 1).val() == maximum % 97);
    require(comb.H(1, maximum).val() == 1);

    Dynamic::set_mod(64);
    sirkit::ModIntBinomial<Dynamic> composite(0, {0, 0});
    rejects<std::length_error>([&] { (void)composite.COM(17, 8); });
    rejects<std::length_error>([&] { composite.prepare(1); });
    require(composite.COM(17, 1).val() == 17);
    rejects<std::overflow_error>([&] { (void)composite.H(maximum, 2); });
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([&] { comb.prepare(-1); });
    rejects<std::invalid_argument>([] { sirkit::ModIntBinomial<Mint> invalid(-1); });
    rejects<std::invalid_argument>([] { sirkit::BinomialMod invalid(0, 0); });
    rejects<std::invalid_argument>([] { sirkit::BinomialMod invalid(Index{1} << 32, 0); });
    rejects<std::invalid_argument>([] { sirkit::BinomialMod invalid(2, -1); });
    Dynamic::set_mod(97);
    sirkit::ModIntBinomial<Dynamic> old(10);
    Dynamic::set_mod(89);
    rejects<std::logic_error>([&] { (void)old.COM(5, 1); });
    rejects<std::logic_error>([&] { (void)old.H(1, 0); });
    rejects<std::logic_error>([&] { old.prepare(0); });
    Dynamic::set_mod(97);
    require(old.COM(5, 2).val() == 10);
#endif
}
} // namespace

int main() {
    check_composite_paths();
    check_lucas<2>();
    check_lucas<3>();
    check_lucas<97>();
    check_growth<10007>(4, 4098);
    check_growth<65537>(3, 4099);
    check_errors();
}
