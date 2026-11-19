// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../../../Math/Linear_Recurrence.cpp"

namespace {

template <class Number, class Residue>
void check_modular_arithmetic(Residue residue) {
    const auto mod = static_cast<std::uint64_t>(Number::modulus);
    const std::vector<Number> defaults(2);
    for (const auto& number : defaults) assert(residue(number) == 0);
    constexpr std::uint64_t kLeft[]{0, 1, 7, 998'244'352};
    constexpr std::uint64_t kRight[]{1, 3, 998'244'352};
    for (const auto a : kLeft) {
        for (const auto b : kRight) {
            const Number x{a}, y{b};
            assert(residue(x + y) == (a + b) % mod);
            assert(residue(x - y) == (a + mod - b) % mod);
            assert(residue(x * y) == a * b % mod);
            assert(residue(x / y) * b % mod == a);
        }
    }
}

constexpr bool trial_prime(std::uint32_t n) {
    if (n < 2) return false;
    for (std::uint32_t divisor = 2; divisor < n; ++divisor)
        if (n % divisor == 0) return false;
    return true;
}

constexpr std::uint32_t repeated_power(std::uint32_t base, std::uint32_t exponent) {
    std::uint32_t result = 1;
    for (; exponent != 0; --exponent) result = result * base % 17;
    return result;
}

static_assert([]<std::size_t... Index>(std::index_sequence<Index...>) {
    return ((sirkit::detail::ntt_prime(Index) == trial_prime(Index)) && ...);
}(std::make_index_sequence<101>{}));

static_assert([]<std::size_t... Index>(std::index_sequence<Index...>) {
    return ((sirkit::detail::ntt_power(Index / 12, Index % 12, 17) ==
             repeated_power(Index / 12, Index % 12)) && ...);
}(std::make_index_sequence<144>{}));

void check_number_theory_and_size() {
    check_modular_arithmetic<sirkit::detail::RecurrenceMod998>([](const auto& x) { return x.value; });
#if defined(__clang__)
    check_modular_arithmetic<sirkit::detail::RecurrenceMontgomery998>([](const auto& x) { return x.get(); });
#endif
    assert(sirkit::detail::recurrence_product_size<std::uint32_t>(0) == 0);
    assert(sirkit::detail::recurrence_product_size<std::uint32_t>(19) == 37);
    try {
        (void)sirkit::detail::recurrence_product_size<std::uint32_t>(std::numeric_limits<std::size_t>::max());
        assert(false);
    } catch (const std::length_error&) {
    }
    const auto capacity = std::vector<std::uint32_t>{}.max_size();
    assert(sirkit::detail::recurrence_size_plus_one(capacity - 1, capacity, "capacity") == capacity);
    try {
        (void)sirkit::detail::recurrence_size_plus_one(capacity, capacity, "capacity");
        assert(false);
    } catch (const std::length_error&) {
    }
    try {
        (void)sirkit::detail::convolution_result_size(
            std::numeric_limits<std::size_t>::max(), 2, "NTT: convolution size overflow");
        assert(false);
    } catch (const std::length_error&) {
    }
}

}  // namespace

int main() {
    check_number_theory_and_size();
    using Residues = std::vector<std::uint32_t>;
    constexpr auto kMod = sirkit::NTT<>::modulus();

    // 法を超える係数を含む有限列の最短漸化式は、一次の等比列から求められる。
    assert(sirkit::BerlekampMassey(Residues{}).empty());
    assert(sirkit::BerlekampMassey(Residues{1, 3, 9, 27, kMod + 81}) == Residues({3}));
    // 0,1,0,1,... の偶数項では分子の係数を間引くとゼロになる。
    const Residues alternating{0, 1};
    assert(sirkit::LinearRecurrenceTerm(alternating, alternating, 2) == 0);
    assert(sirkit::LinearRecurrenceTerm(alternating, alternating, 3) == 1);

    sirkit::NTT<> ntt;
    ntt.prepare(0);
    assert(ntt.roots_capacity() == 0);
    try {
        sirkit::NTT<17, 3>{}.prepare(32);
        assert(false);
    } catch (const std::length_error&) {
    }
    // 定数多項式の全周波数成分は定数項に等しい。
    Residues impulse(16);
    impulse.front() = 5;
    auto frequencies = impulse;
    ntt.transform(frequencies);
    assert(frequencies == Residues(16, 5));
    ntt.transform(frequencies, true);
    assert(frequencies == impulse);

    // 全係数が1の多項式の積を、次数ごとの組数と照合する。
    const Residues smaller(33, 1);
    const Residues larger(2048, 1);
    const auto product = ntt.convolution_normalized(smaller, larger);
    assert(product.size() == smaller.size() + larger.size() - 1);
    for (std::size_t i = 0; i < product.size(); ++i) {
        assert(product[i] == std::min({i + 1, smaller.size(), product.size() - i}));
    }

#ifndef SKIP_RUNTIME_CHECKS
    impulse.back() = kMod;
    bool rejected = false;
    try {
        ntt.transform(impulse);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);

    rejected = false;
    try {
        (void)sirkit::LinearRecurrencePrefix(Residues{1}, Residues{}, 5);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);
#endif
}
