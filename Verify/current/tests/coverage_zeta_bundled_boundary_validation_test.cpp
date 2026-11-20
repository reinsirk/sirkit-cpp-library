// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Zeta_Mebius.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

struct Power {
    std::int64_t value;
    Power() : value(0) {}
    explicit Power(int v) : value(v) {}
    friend Power operator*(Power a, Power b) {
        a.value *= b.value;
        return a;
    }
    Power pow(std::int64_t exponent) const {
        Power answer(1);
        for (std::int64_t i = 0; i < exponent; ++i) answer = answer * *this;
        return answer;
    }
};

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
    const sirkit::IntegerSieve sieve(10'000);
    const auto powers = sieve.power_table<Power>(3, 65);
    for (std::int64_t i = 0; i < 66; ++i) assert(powers[i].value == i * i * i);
    const auto one = sieve.power_table<Power>(2, 0);
    assert(one.size() == 1 && one.front().value == 0);
    const sirkit::IntegerSieve unit_sieve(1);
    std::vector<std::pair<int, int>> singleton{{1, 5}};
    sirkit::sparse_multiple_zeta_transform(singleton, unit_sieve);
    sirkit::sparse_multiple_mobius_transform(singleton, unit_sieve);
    assert((singleton == std::vector<std::pair<int, int>>{{1, 5}}));

    std::vector<std::pair<int, int>> input{{1, 3}, {97, 4}, {101, 5}, {97 * 101, 6}};
    const auto saved = input;
    const std::vector<std::pair<int, int>> zeta{{1, 18}, {97, 10}, {101, 11}, {97 * 101, 6}};
    sirkit::sparse_multiple_zeta_transform(input, sieve);
    assert(input == zeta);
    sirkit::sparse_multiple_mobius_transform(input, sieve);
    assert(input == saved);
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::length_error>([] {
        std::vector<int> storage;
        (void)sirkit::detail::IntegerTransform::prime_view_for(
            static_cast<std::size_t>(std::numeric_limits<int>::max()) + 2, storage);
    });
    rejects<std::invalid_argument>([&] { (void)sieve.power_table<Power>(-1, 10); });
    rejects<std::out_of_range>([&] { (void)sieve.power_table<Power>(2, -1); });
    rejects<std::out_of_range>([&] { (void)sieve.power_table<Power>(2, sieve.max_value() + 1); });
    rejects<std::out_of_range>([&] { (void)sieve.is_prime(-1); });
    rejects<std::invalid_argument>([&] { (void)sieve.prime_factorization(0); });
    rejects<std::out_of_range>([] {
        auto storage = sirkit::IntegerSieve(8);
        const auto moved = std::move(storage);
        assert(moved.max_value() == 8);
        (void)storage.prime_factorization(2);
    });
    rejects<std::out_of_range>([] {
        auto storage = sirkit::IntegerSieve(8);
        const auto moved = std::move(storage);
        assert(moved.max_value() == 8);
        (void)sirkit::gcd_square(std::vector<int>{0, 1}, storage);
    });
    const std::vector<int> left{1, 2}, right{3, 4, 5, 6};
    rejects<std::invalid_argument>([&] { (void)sirkit::bitwise_and_convolution(left, right); });
    rejects<std::invalid_argument>([&] { (void)sirkit::bitwise_or_convolution(left, right); });
    rejects<std::out_of_range>([] {
        std::vector<std::pair<int, int>> dense{{1, 7}, {2, 8}, {3, 9}};
        sirkit::sparse_multiple_zeta_transform(dense, sirkit::IntegerSieve(2));
    });
    // 疎な表で、重複および直接位置表・二分探索の両方の欠損を検査する。
    for (const auto& invalid : {std::vector{1, 101, 101}, std::vector{1, 1024}, std::vector{1, 97 * 101}}) {
        for (bool inverse : {false, true}) {
            std::vector<std::pair<int, int>> values;
            for (int key : invalid) values.emplace_back(key, 1);
            rejects<std::invalid_argument>([&] {
                if (inverse) sirkit::sparse_multiple_mobius_transform(values, sieve);
                else sirkit::sparse_multiple_zeta_transform(values, sieve);
            });
        }
    }
#endif
}
