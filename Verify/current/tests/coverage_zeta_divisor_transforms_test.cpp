// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Zeta/Divisor_Multiple_Transform.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

int main() {
    const sirkit::IntegerSieve tiny(4);
    assert((tiny.prime_factorization(11) == std::map<int, int>{{11, 1}}));
    bool size_rejected = false;
    try {
        std::vector<int> storage;
        (void)sirkit::detail::IntegerTransform::prime_view_for(
            static_cast<std::size_t>(std::numeric_limits<int>::max()) + 2, storage);
    } catch (const std::length_error&) {
        size_rejected = true;
    }
    assert(size_rejected);

    for (std::size_t size : {0, 1, 3, 127, 128, 129, 257}) {
        std::vector<std::int64_t> input(size);
        for (std::size_t i = 0; i < size; ++i) input[i] = static_cast<std::int64_t>(i % 13) - 6;
        auto divisor = input, multiple = input;
        // 0番目は排除し、正の全添字を直接比較する。
        for (std::size_t i = 1; i < size; ++i) {
            divisor[i] = multiple[i] = 0;
            for (std::size_t j = 1; j < size; ++j) {
                if (i % j == 0) divisor[i] += input[j];
                if (j % i == 0) multiple[i] += input[j];
            }
        }
        const sirkit::IntegerSieve sieve(static_cast<int>(size));
        auto implicit_divisor = input, explicit_divisor = input;
        auto implicit_multiple = input, explicit_multiple = input;
        sirkit::divisor_zeta_transform(implicit_divisor);
        sirkit::divisor_zeta_transform(explicit_divisor, sieve);
        sirkit::multiple_zeta_transform(implicit_multiple);
        sirkit::multiple_zeta_transform(explicit_multiple, sieve);
        assert(implicit_divisor == divisor && explicit_divisor == divisor);
        assert(implicit_multiple == multiple && explicit_multiple == multiple);
        sirkit::divisor_mobius_transform(implicit_divisor);
        sirkit::divisor_mobius_transform(explicit_divisor, sieve);
        sirkit::multiple_mobius_transform(implicit_multiple);
        sirkit::multiple_mobius_transform(explicit_multiple, sieve);
        assert(implicit_divisor == input && explicit_divisor == input);
        assert(implicit_multiple == input && explicit_multiple == input);
    }
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<int> original{0, 1, 2, 3};
    auto kept = original;
    bool negative_sieve_rejected = false;
    try {
        (void)sirkit::IntegerSieve(-1);
    } catch (const std::invalid_argument&) {
        negative_sieve_rejected = true;
    }
    assert(negative_sieve_rejected);
    auto old_sieve = sirkit::IntegerSieve(10);
    auto moved_sieve = std::move(old_sieve);
    assert(moved_sieve.max_value() == 10);
    bool moved_rejected = false;
    try {
        sirkit::multiple_zeta_transform(kept, old_sieve);
    } catch (const std::out_of_range&) {
        moved_rejected = true;
    }
    assert(moved_rejected && kept == original);
    try {
        sirkit::divisor_zeta_transform(kept, sirkit::IntegerSieve(2));
    } catch (const std::out_of_range&) {
        assert(kept == original);
        return 0;
    }
    assert(false);
#endif
}
