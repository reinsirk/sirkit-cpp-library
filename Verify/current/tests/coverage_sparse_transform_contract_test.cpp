// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_COVERAGE_ZETA_SOURCE
#define SIRKIT_COVERAGE_ZETA_SOURCE "../../../Math/Zeta/Sparse_Transform.cpp"
#endif
#include SIRKIT_COVERAGE_ZETA_SOURCE

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

using Values = std::vector<std::pair<int, std::int64_t>>;

void check_transform(Values values, const sirkit::IntegerSieve& sieve) {
    auto original = values;
    std::ranges::sort(original);
    auto expected = original;
    for (auto& [divisor, sum] : expected) {
        sum = 0;
        for (const auto& [key, value] : original) {
            if (key % divisor == 0) sum += value;
        }
    }
    sirkit::sparse_multiple_zeta_transform(values, sieve);
    assert(values == expected);
    auto sorted = values;
    sirkit::sparse_multiple_mobius_transform(sorted, sieve);
    assert(sorted == original);
    std::ranges::reverse(values);
    sirkit::sparse_multiple_mobius_transform(values, sieve);
    assert(values == original);
}

#ifndef SKIP_RUNTIME_CHECKS
template <class Error, class Call>
void rejects(Call call) {
    try {
        call();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

template <class Error>
void rejects_transform(const Values& input, const sirkit::IntegerSieve& sieve) {
    rejects<Error>([&] {
        auto values = input;
        sirkit::sparse_multiple_zeta_transform(values, sieve);
    });
    rejects<Error>([&] {
        auto values = input;
        sirkit::sparse_multiple_mobius_transform(values, sieve);
    });
}
#endif

} // namespace

int main() {
    const sirkit::IntegerSieve sieve(10'000);
    Values dense;
    for (int key = 1; key <= 65; ++key) dense.emplace_back(key, 17 - key);
    check_transform(dense, sieve);
    check_transform({{4, -3}, {2, 8}, {1, 5}, {8, 9}, {16, 2}}, sieve);
    check_transform({{6, 4}, {2, -1}, {1, 2}, {3, 9}}, sieve);
    check_transform({{899, 7}, {29, -5}, {31, 8}, {1, -6}}, sieve);
    check_transform({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {7, 5}, {14, 6}, {28, 7}}, sieve);
    check_transform({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {29, 7}}, sieve);
    check_transform({}, sieve);

    // 16個のキーに対する表で初期位置が重なる素数を並べ、二分探索まで通す。
    Values colliding{{1, -15}};
    for (const int prime : {181, 359, 503, 613, 647, 757, 1223, 1367, 1511, 1723, 1867,
                            2011, 2333, 2477, 2621}) {
        colliding.emplace_back(prime, prime % 11 - 6);
    }
    std::ranges::reverse(colliding);
    check_transform(colliding, sieve);

#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([] { (void)sirkit::IntegerSieve(-1); });
    rejects<std::out_of_range>([&] { (void)sieve.is_prime(-1); });
    rejects<std::invalid_argument>([&] { (void)sieve.prime_factorization(0); });
    auto source = sirkit::IntegerSieve(16);
    const auto moved = std::move(source);
    assert(moved.max_value() == 16);
    rejects<std::out_of_range>([&] { (void)source.prime_factorization(2); });
    rejects_transform<std::out_of_range>({{1, 8}}, source);
    rejects_transform<std::out_of_range>({{1, 2}, {3, 8}}, source);
    rejects_transform<std::out_of_range>({{1, 8}, {2, 7}, {3, 6}}, sirkit::IntegerSieve(2));
    rejects_transform<std::invalid_argument>({{1, 8}, {2, 7}, {2, 6}}, sieve);
    rejects_transform<std::invalid_argument>({{1, 8}, {4, 7}}, sieve);
    rejects_transform<std::invalid_argument>({{1, 8}, {101, 7}, {101, 6}}, sieve);
    rejects_transform<std::invalid_argument>({{1, 8}, {512, 7}}, sieve);
    rejects_transform<std::invalid_argument>({{1, 8}, {97 * 101, 7}}, sieve);
#endif
}
