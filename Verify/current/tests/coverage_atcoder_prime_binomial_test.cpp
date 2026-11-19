// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Combination_AtCoderMod.cpp"
#include <atcoder/modint.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <random>
#include <source_location>
#include <stdexcept>
#include <utility>
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

Int exact_small_k(Int n, Int k) {
    Int result = 1;
    for (Int i = 1; i <= k; ++i) result = result * (n - i + 1) / i;
    return result;
}

Int lucas_small_prime(Int n, Int k, int p) {
    Int result = 1;
    while (n || k) {
        const Int nd = n % p;
        const Int kd = k % p;
        if (kd > nd) return 0;
        result = result * exact_small_k(nd, kd) % p;
        n /= p;
        k /= p;
    }
    return result;
}

template<int Prime>
void check_small_prime() {
    using Mint = atcoder::static_modint<Prime>;
    for (int bound : {0, 1, Prime - 1}) {
        sirkit::PrimeBinomial<Mint> comb(bound, 40);
        std::vector<Int> row(301);
        row[0] = 1;
        for (Index n = 0; n <= 300; ++n) {
            for (Index k = 0; k <= n; ++k) {
                require(static_cast<Int>(comb.COM(n, k).val()) == row[k]);
                require(static_cast<Int>(comb.Lucas(n, n - k).val()) == row[k]);
            }
            if (n < 300) {
                for (Index k = n + 1; k > 0; --k) row[k] = (row[k] + row[k - 1]) % Prime;
            }
        }
        std::mt19937_64 rng(11818);
        for (int i = 0; i < 500; ++i) {
            const auto n = rng() >> 1;
            const auto k = rng() % (n + 1);
            require(static_cast<Int>(comb.Lucas(n, k).val()) == lucas_small_prime(n, k, Prime));
        }
        auto copy = comb;
        auto moved = std::move(copy);
        copy.prepare(Prime - 1);
        require(static_cast<Int>(copy.COM(300, 31).val()) == row[31]);
        require(static_cast<Int>(moved.COM(300, 31).val()) == row[31]);
    }
}

template<int Prime>
void check_large_tables(Index initial, Index first_target) {
    using Mint = atcoder::static_modint<Prime>;
    sirkit::PrimeBinomial<Mint> comb(initial);
    std::mt19937 rng(701);
    for (const auto target : {first_target, static_cast<Index>(Prime - 1)}) {
        comb.prepare(target);
        comb.prepare(target - 1);
        for (int i = 0; i < 150; ++i) {
            const Int n = i < 10 ? target - i : rng() % (target - 3) + 4;
            for (Int k = 0; k <= 4; ++k) {
                const auto expected = exact_small_k(n, k) % Prime;
                require(static_cast<Int>(comb.COM(n, k).val()) == expected);
                require(static_cast<Int>(comb.Lucas(n, n - k).val()) == expected);
            }
        }
    }
}

// val()と単項マイナスを持たないMintで汎用経路と表拡張の失敗を確認する。
struct SimpleMint {
    using Residue = atcoder::static_modint<10007>;
    inline static int creates_before_failure = -1;

    Residue value;

    SimpleMint() {
        if (creates_before_failure == 0) throw std::bad_alloc();
        if (creates_before_failure > 0) --creates_before_failure;
    }
    SimpleMint(Index value) : value(value) {}
    explicit SimpleMint(Residue value) : value(value) {}
    static constexpr int mod() { return Residue::mod(); }
    static SimpleMint raw(int value) { return SimpleMint(Residue::raw(value)); }
    SimpleMint inv() const { return SimpleMint(value.inv()); }
    SimpleMint& operator*=(const SimpleMint& other) {
        value *= other.value;
        return *this;
    }
    friend SimpleMint operator*(SimpleMint left, const SimpleMint& right) { return left *= right; }
};

void check_table_recovery() {
    sirkit::PrimeBinomial<SimpleMint> comb(0);
    SimpleMint::creates_before_failure = 64;
    rejects<std::bad_alloc>([&] { comb.prepare(64); });
    SimpleMint::creates_before_failure = -1;
    require(comb.COM(64, 4).value.val() == static_cast<int>(exact_small_k(64, 4) % SimpleMint::mod()));
    comb.prepare(SimpleMint::mod() - 1);
    require(comb.COM(10005, 3).value.val() == static_cast<int>(exact_small_k(10005, 3) % SimpleMint::mod()));
}

void check_errors() {
    using Mint = atcoder::static_modint<97>;
    constexpr auto top = std::numeric_limits<Index>::max();
    sirkit::PrimeBinomial<Mint> comb(0, 5);
    require(comb.COM(-1, 0).val() == 0);
    require(comb.COM(1, -1).val() == 0);
    require(comb.COM(1, 2).val() == 0);
    require(comb.COM(80, 0).val() == 1);
    require(comb.COM(80, 79).val() == 80);
    require(comb.COM(80, 4).val() == static_cast<int>(exact_small_k(80, 4) % 97));
    require(comb.COM(97 * 97 + 2 * 97 + 96, 3 * 97 + 20).val() == 0);
    require(comb.Lucas(2 * 97 + 3, 97).val() == 2);
    require(comb.Lucas(2 * 97 + 3, 97 + 4).val() == 0);
    rejects<std::length_error>([&] { (void)comb.COM(80, 6); });
    rejects<std::length_error>([&] { (void)comb.COM(16 * 97 + 16, 3 * 97 + 3); });
    require(comb.H(-1, 0).val() == 0);
    require(comb.H(1, -1).val() == 0);
    require(comb.H(0, 0).val() == 1);
    require(comb.H(0, 1).val() == 0);
    require(comb.H(7, 3).val() == exact_small_k(9, 3) % 97);
    require(comb.H(top, 1).val() == top % 97);
    rejects<std::overflow_error>([&] { (void)comb.H(top, 2); });
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([&] { comb.prepare(-1); });
    require(!sirkit::detail::is_prime_u32(0));
    require(!sirkit::detail::is_prime_u32(1));
    using Dynamic = atcoder::dynamic_modint<833>;
    for (const int modulus : {1, 4, 9, 25, 49, 121, 169, 437, 321503175}) {
        Dynamic::set_mod(modulus);
        rejects<std::domain_error>([] { sirkit::PrimeBinomial<Dynamic> invalid(0); });
    }
    for (const int prime : {2, 3, 5, 7, 11, 13, 61, 97, 167772161, 469762049, 754974721,
                            998244353, 1000000007, 1000000009, 1004535809, 1224736769, 2013265921, 2147483647}) {
        Dynamic::set_mod(prime);
        sirkit::PrimeBinomial<Dynamic> dynamic(13);
        require(dynamic.COM(13, 3).val() == static_cast<int>(exact_small_k(13, 3) % prime));
    }
    Dynamic::set_mod(97);
    const sirkit::PrimeBinomial<Dynamic> stale(8);
    Dynamic::set_mod(89);
    rejects<std::logic_error>([&] { (void)stale.COM(8, 2); });
    rejects<std::logic_error>([&] { (void)stale.H(1, 0); });
    Dynamic::set_mod(97);
    require(stale.COM(8, 2).val() == 28);
#endif
}
} // namespace

int main() {
    const auto capacity = std::numeric_limits<std::size_t>::max();
    require(sirkit::detail::prime_binomial_table_bound(capacity - 1, capacity) == capacity - 1);
    rejects<std::length_error>([&] {
        (void)sirkit::detail::prime_binomial_table_bound(capacity, capacity);
    });
    check_small_prime<2>();
    check_small_prime<3>();
    check_small_prime<11>();
    check_large_tables<10007>(4, 4098);
    check_large_tables<65537>(3, 4099);
    check_table_recovery();
    check_errors();
}
