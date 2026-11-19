// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Zeta/Gcd_Lcm_Convolution.cpp"
#include <atcoder/modint>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

using sirkit::IntegerSieve;

template <class Error, class Function>
void rejects(Function function) {
    try {
        function();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

template <class Value>
std::vector<Value> reference(const std::vector<Value>& lhs, const std::vector<Value>& rhs, bool gcd) {
    std::vector<Value> result(lhs.size());
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        for (std::size_t j = 0; j < rhs.size(); ++j) {
            const auto index = gcd ? std::gcd(i, j) : std::lcm(i, j);
            if (index >= result.size()) continue;
            if constexpr (std::is_unsigned_v<Value>) {
                result[index] = static_cast<Value>(std::uint64_t{result[index]} +
                    std::uint64_t{lhs[i]} * std::uint64_t{rhs[j]});
            } else {
                result[index] += lhs[i] * rhs[j];
            }
        }
    }
    return result;
}

template <class Value>
void convolutions(const IntegerSieve& sieve) {
    constexpr std::array<std::size_t, 9> lengths{0, 1, 2, 3, 7, 127, 128, 129, 257};
    for (const auto length : lengths) {
        for (int zeros = 0; zeros < 4; ++zeros) {
            std::vector<Value> lhs(length), rhs(length);
            for (std::size_t i = 0; i < length; ++i) {
                lhs[i] = static_cast<Value>(static_cast<int>(i % 13) - 7);
                rhs[i] = static_cast<Value>(static_cast<int>((i * 17) % 19) - 11);
            }
            if (length && (zeros & 1)) lhs[0] = 0;
            if (length && (zeros & 2)) rhs[0] = 0;

            const auto gcd = reference(lhs, rhs, true);
            const auto lcm = reference(lhs, rhs, false);
            assert(sirkit::gcd_convolution(lhs, rhs) == gcd);
            assert(sirkit::gcd_convolution(lhs, rhs, sieve) == gcd);
            assert(sirkit::lcm_convolution(lhs, rhs) == lcm);
            assert(sirkit::lcm_convolution(lhs, rhs, sieve) == lcm);

            const auto gcd_square = reference(lhs, lhs, true);
            const auto lcm_square = reference(lhs, lhs, false);
            assert(sirkit::gcd_square(lhs) == gcd_square);
            assert(sirkit::gcd_square(lhs, sieve) == gcd_square);
            assert(sirkit::lcm_square(lhs) == lcm_square);
            assert(sirkit::lcm_square(lhs, sieve) == lcm_square);
        }
    }
}

int totient(int n) {
    int answer = 0;
    for (int i = 1; i <= n; ++i) {
        if (std::gcd(i, n) == 1) ++answer;
    }
    return answer;
}

void sieve_tables() {
    using Mint = atcoder::modint998244353;
    for (int bound : {0, 1, 2, 8, 20, 111}) {
        const IntegerSieve sieve(bound);
        const auto mobius = sieve.mobius_table();
        const auto phi = sieve.totient_table();
        assert(mobius.size() == static_cast<std::size_t>(bound + 1));
        assert(phi.size() == mobius.size());
        assert(mobius[0] == 0);
        assert(phi[0] == 0);
        for (int n = 1; n <= bound; ++n) {
            int divisor_sum = 0;
            for (int divisor = 1; divisor <= n; ++divisor) {
                if (n % divisor == 0) divisor_sum += mobius[static_cast<std::size_t>(divisor)];
            }
            assert(divisor_sum == (n == 1 ? 1 : 0));
            assert(phi[static_cast<std::size_t>(n)] == totient(n));
        }
        for (int exp : {0, 1, 5, 19}) {
            const auto powers = sieve.power_table<Mint>(exp, bound);
            assert(powers.size() == mobius.size());
            for (int n = 0; n <= bound; ++n) {
                Mint expected = 1;
                for (int step = 0; step < exp; ++step) expected *= n;
                assert(powers[static_cast<std::size_t>(n)] == expected);
            }
        }
    }

    const IntegerSieve sieve(20);
    for (const int n : {1, 20, 36, 49, 323, 371, 397, 398, 400}) {
        std::vector<int> divisors;
        std::map<int, int> totients;
        for (int divisor = 1; divisor <= n; ++divisor) {
            if (n % divisor == 0) {
                divisors.push_back(divisor);
                totients.emplace(divisor, totient(divisor));
            }
        }
        auto enumerated = sieve.enumerate_divisors(n);
        std::sort(enumerated.begin(), enumerated.end());
        assert(enumerated == divisors);
        assert(sieve.sorted_divisors(n) == divisors);
        assert(sieve.divisor_totients(n) == totients);
    }
}

void ownership_and_validation() {
    const IntegerSieve sieve(257);
    IntegerSieve copy(sieve), assigned(0);
    assigned = copy;
    const auto* alias = &assigned;
    assigned = *alias;
    assert(assigned.least_prime_factor == sieve.least_prime_factor);
    assert(assigned.prime_list == sieve.prime_list);
    IntegerSieve moved(std::move(assigned));
    assert(assigned.max_value() == -1);
    const auto empty = std::vector<std::int64_t>{};
    assert(sirkit::gcd_square(empty, assigned).empty());
    assert(sirkit::lcm_convolution(empty, empty, assigned).empty());
    assigned.swap(copy);
    assert(copy.max_value() == -1);
    copy = std::move(moved);
    assert(moved.max_value() == -1);
    auto* move_alias = &copy;
    copy = std::move(*move_alias);
    assert(copy.least_prime_factor == sieve.least_prime_factor);
    assert(copy.prime_list == sieve.prime_list);

#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<int> small{1, 2, 3}, shorter{1, 2};
    const IntegerSieve limited(1);
    rejects<std::invalid_argument>([] { (void)IntegerSieve(-1); });
    rejects<std::out_of_range>([&] { (void)sieve.is_prime(-1); });
    rejects<std::out_of_range>([&] { (void)sieve.is_prime(258u); });
    rejects<std::invalid_argument>([&] { (void)sieve.prime_factorization(0); });
    rejects<std::out_of_range>([&] { (void)limited.prime_factorization(2); });
    rejects<std::out_of_range>([&] { (void)moved.prime_factorization(2); });
    rejects<std::invalid_argument>([&] { (void)sieve.power_table<atcoder::modint998244353>(-1, 5); });
    rejects<std::out_of_range>([&] { (void)sieve.power_table<atcoder::modint998244353>(1, -1); });
    rejects<std::out_of_range>([&] { (void)sieve.power_table<atcoder::modint998244353>(1, 258); });
    rejects<std::invalid_argument>([&] { (void)sirkit::gcd_convolution(small, shorter, sieve); });
    rejects<std::invalid_argument>([&] { (void)sirkit::lcm_convolution(small, shorter); });
    rejects<std::out_of_range>([&] { (void)sirkit::gcd_square(small, moved); });
    rejects<std::out_of_range>([&] { (void)sirkit::lcm_convolution(small, small, limited); });
    assert(sirkit::gcd_convolution(small, small, sieve) == reference(small, small, true));
#endif
}

} // namespace

int main() {
    rejects<std::length_error>([] {
        std::vector<int> storage;
        (void)sirkit::detail::IntegerTransform::prime_view_for(
            static_cast<std::size_t>(std::numeric_limits<int>::max()) + 2, storage);
    });
    const IntegerSieve sieve(300);
    assert(!sieve.is_prime(0));
    assert(!sieve.is_prime(1));
    assert(sieve.is_prime(2u));
    assert(sieve.is_prime(293));
    assert(!sieve.is_prime(289));
    convolutions<std::int64_t>(sieve);
    convolutions<std::uint16_t>(sieve);
    sieve_tables();
    ownership_and_validation();
}
