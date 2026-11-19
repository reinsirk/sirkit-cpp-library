// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_ZETA_I386_SOURCE
#define SIRKIT_TEST_ZETA_I386_SOURCE "../../../Math/Zeta/Sieve.cpp"
#endif
#include SIRKIT_TEST_ZETA_I386_SOURCE

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

static_assert(sizeof(int) == 4 && sizeof(std::ptrdiff_t) == 4 && sizeof(std::size_t) == 4);

// 係数型を大きくし、小さい篩で係数表の容量境界を越える。
struct LargeMint {
    std::uint8_t value;
    std::byte storage[64 * 1024]{};

    explicit LargeMint(int residue = 0) : value(static_cast<std::uint8_t>(residue % 17)) {}
    LargeMint pow(std::int64_t exponent) const {
        int result = 1;
        while (exponent-- > 0) result = result * value % 17;
        return LargeMint(result);
    }
    friend LargeMint operator*(const LargeMint& a, const LargeMint& b) {
        return LargeMint(a.value * b.value);
    }
};

template<class Operation>
void rejects_capacity(std::string_view message, Operation operation) {
    try {
        operation();
    } catch (const std::length_error& error) {
        assert(error.what() == message);
        return;
    }
    assert(false);
}

} // namespace

int main() {
    rejects_capacity("IntegerSieve: table too large", [] {
        sirkit::IntegerSieve oversized(std::numeric_limits<int>::max());
    });
    const int limit = static_cast<int>(std::vector<LargeMint>{}.max_size());
    assert(limit > 1);
    const sirkit::IntegerSieve sieve(limit);
    rejects_capacity("IntegerSieve::power_table: table too large", [&] {
        (void)sieve.power_table<LargeMint>(1, limit);
    });
    assert(sieve.max_value() == limit && sieve.is_prime(37));
    const auto divisor_limit = std::vector<int>{}.max_size();
    assert(sirkit::detail::sieve_divisor_capacity(divisor_limit / 2, 2, divisor_limit) ==
           (divisor_limit / 2) * 2);
    rejects_capacity("IntegerSieve: divisor table too large", [&] {
        (void)sirkit::detail::sieve_divisor_capacity(divisor_limit / 2 + 1, 2, divisor_limit);
    });
#ifdef SIRKIT_INTERNAL_SPARSE_INTEGER_TRANSFORM_DEFINED
    const auto slot_limit = std::vector<std::size_t>{}.max_size();
    const auto sparse_count = slot_limit / 4 + 1;
    assert(sirkit::detail::sparse_lookup_capacity(4, slot_limit) == 8);
    assert(sirkit::detail::sparse_transfer_reserve_size(sparse_count, slot_limit) == sparse_count);
    assert(sirkit::detail::sparse_transfer_reserve_size(4, slot_limit) == 16);
    rejects_capacity("sparse integer transform: lookup capacity", [&] {
        (void)sirkit::detail::sparse_lookup_capacity(sparse_count, slot_limit);
    });
#endif
}
