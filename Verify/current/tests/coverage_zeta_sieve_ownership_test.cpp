// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_ZETA_SIEVE_TEST_SOURCE
#define SIRKIT_ZETA_SIEVE_TEST_SOURCE "../../../Math/Zeta/Sparse_Transform.cpp"
#endif
#include SIRKIT_ZETA_SIEVE_TEST_SOURCE

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

std::map<int, int> trial_factors(int value) {
    std::map<int, int> result;
    for (int prime = 2; prime <= value / prime; ++prime) {
        while (value % prime == 0) {
            ++result[prime];
            value /= prime;
        }
    }
    if (value > 1) ++result[value];
    return result;
}

struct Mod17 {
    int value = 0;
    explicit Mod17(int input = 0) : value(input % 17) {}
    friend Mod17 operator*(Mod17 left, Mod17 right) {
        return Mod17(left.value * right.value);
    }
    Mod17 pow(std::int64_t exponent) const {
        Mod17 result(1), factor(*this);
        while (exponent != 0) {
            if (exponent % 2 != 0) result = result * factor;
            exponent /= 2;
            factor = factor * factor;
        }
        return result;
    }
};

void check_power_table(const sirkit::IntegerSieve& sieve, int limit) {
    for (const std::int64_t exponent : {0, 1, 9}) {
        const auto powers = sieve.power_table<Mod17>(exponent, limit);
        assert(powers.size() == static_cast<std::size_t>(limit + 1));
        for (int base = 0; base <= limit; ++base) {
            int expected = 1;
            for (std::int64_t i = 0; i < exponent; ++i) expected = expected * base % 17;
            assert(powers[static_cast<std::size_t>(base)].value == expected);
        }
    }
}

void check_sieve(const sirkit::IntegerSieve& sieve, int limit) {
    assert(sieve.max_value() == limit);
    const auto mobius = sieve.mobius_table();
    const auto totient = sieve.totient_table();
    assert(mobius.size() == static_cast<std::size_t>(limit + 1));
    assert(totient.size() == mobius.size());
    if (limit < 0) return;
    assert(mobius[0] == 0 && totient[0] == 0);
    std::vector<int> primes;
    for (int value = 1; value <= limit; ++value) {
        const auto factors = trial_factors(value);
        const bool prime = factors.contains(value);
        if (prime) primes.push_back(value);
        assert(sieve.is_prime(value) == prime);
        assert(sieve.prime_factorization(value) == factors);
        int mu = 1, phi = 0;
        for (auto [p, exponent] : factors) {
            assert(p >= 2);
            mu = exponent == 1 ? -mu : 0;
        }
        for (int j = 1; j <= value; ++j) phi += std::gcd(j, value) == 1;
        assert(mobius[value] == mu && totient[value] == phi);
    }
    assert(sieve.prime_list == primes);
}

#ifndef SKIP_RUNTIME_CHECKS
template<class Error, class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const Error&) {
        return;
    }
    assert(false);
}
#endif

} // namespace

int main() {
    const sirkit::IntegerSieve tiny(4);
    assert(tiny.prime_factorization(11) == trial_factors(11));
    sirkit::IntegerSieve original(64), other(4);
    other = original;
    const auto& self_copy = other;
    other = self_copy;
    sirkit::IntegerSieve moving(std::move(other));
    check_sieve(other, -1);
    other = std::move(moving);
    check_sieve(moving, -1);
    auto& self_move = other;
    other = std::move(self_move);
    sirkit::IntegerSieve small(0);
    other.swap(small);
    check_sieve(other, 0);
    check_sieve(small, 64);
    check_sieve(original, 64);
    check_power_table(other, 0);
    check_power_table(original, 64);
    // 残った大きな素因子と、表の範囲に戻る残余を両方照合する。
    for (int value : {67, 2 * 67, 13 * 13, 2 * 2 * 3 * 43, 53 * 73, 64 * 64}) {
        assert(original.prime_factorization(value) == trial_factors(value));
    }
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::out_of_range>([&] { (void)original.is_prime(-1); });
    rejects<std::out_of_range>([&] { (void)original.is_prime(65u); });
    rejects<std::invalid_argument>([&] { (void)original.prime_factorization(0); });
    rejects<std::out_of_range>([&] { (void)other.prime_factorization(2); });
    rejects<std::out_of_range>([&] { (void)original.prime_factorization(64 * 64 + 1); });
    rejects<std::invalid_argument>([&] { (void)original.power_table<Mod17>(-1, 0); });
    rejects<std::out_of_range>([&] { (void)original.power_table<Mod17>(1, -1); });
    rejects<std::out_of_range>([&] { (void)original.power_table<Mod17>(1, 65); });
#endif
}
