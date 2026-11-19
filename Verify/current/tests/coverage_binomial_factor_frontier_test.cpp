// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Combination/Binomial_Mod.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

namespace {
constexpr int kColumns = 9;

// 階乗や逆元を使わず、Pascalの漸化式で小さいkの列を照合する。
void check_rows(sirkit::BinomialMod& calculator, int size) {
    const auto modulus = calculator.mod();
    std::array<std::uint64_t, kColumns> row{};
    row[0] = 1;
    for (int n = 0; n <= size; ++n) {
        for (int k = 0; k < kColumns && k <= n; ++k) {
            assert(calculator.COM(n, k) == row[k]);
            assert(calculator.COM(n, n - k) == row[k]);
        }
        for (int k = kColumns - 1; k > 0; --k) row[k] = (row[k] + row[k - 1]) % modulus;
    }
}
} // namespace

int main() {
    constexpr std::int64_t known_primes[] = {167772161, 469762049, 754974721, 1000000009, 1004535809,
                                             1224736769, 2013265921};
    for (auto prime : known_primes) {
        sirkit::BinomialMod calculator(prime, 0, {18, 64});
        assert(calculator.is_prime_modulus());
        check_rows(calculator, 36);
    }

    // 小さな因子を除いても2^25を超える合成数が残る。
    constexpr std::int64_t composite_tail = 6007 * 6011;
    for (int first_factor : {2, 3, 5, 7}) {
        sirkit::BinomialMod calculator(first_factor * composite_tail, 0, {18, 64});
        assert(!calculator.is_prime_modulus());
        check_rows(calculator, 36);
    }
    for (int target : {4097, 4098, 4100, 4901}) {
        sirkit::BinomialMod calculator(10007, 4095, {10006, 64});
        calculator.prepare(target);
        check_rows(calculator, 40);
        calculator.prepare(10006);
        check_rows(calculator, target + 1);
    }
    sirkit::BinomialMod odd_tail(10007, 3, {10006, 64});
    odd_tail.prepare(4901);
    check_rows(odd_tail, 4901);
    constexpr std::int64_t n = 92680;
    constexpr std::int64_t expected = n * (n - 1) / 2;
    sirkit::BinomialMod crt(expected + 1, 0, {0, 32});
    assert(crt.COM(n, 2) == expected);
    const sirkit::BinomialMod with_options(11, sirkit::BinomialOptions{10, 32});
    assert(with_options.mod() == 11);
}
