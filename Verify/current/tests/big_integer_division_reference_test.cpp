// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_BIG_INTEGER_DEFINED
#include "../../../Math/Big_Integer.cpp"
#endif
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>

namespace
{
template<unsigned Radix>
void check_magnitudes(const sirkit::BigInteger<Radix> &divisor, const sirkit::BigInteger<Radix> &expected_quotient,
                      const sirkit::BigInteger<Radix> &expected_remainder)
{
    const auto a = expected_quotient * divisor + expected_remainder;
    const auto [q, r] = a.divmod(divisor);
    assert(q == expected_quotient && r == expected_remainder);
    const auto [signed_q, signed_r] = (-a).divmod(-divisor);
    assert(signed_q == expected_quotient && signed_r == -expected_remainder);
}

template<unsigned Radix>
void check()
{
    using Number = sirkit::BigInteger<Radix>;
    constexpr std::int64_t kBase = Radix * Radix * Radix * Radix;
    std::mt19937_64 random(4347818323 + Radix);
    for (int case_id = 0; case_id < 1400; ++case_id)
    {
        auto a = static_cast<std::int64_t>(random() >> 1);
        auto b = static_cast<std::int64_t>(random() >> (2 + random() % 62)) + 1;
        a = case_id % 2 == 0 ? -a : a;
        b = case_id % 5 == 0 ? -b : b;
        const auto [q, r] = Number(a).divmod(Number(b));
        assert(q == Number(a / b) && r == Number(a % b));
    }

    const auto kLow = std::numeric_limits<std::int64_t>::min();
    const auto [low_q, low_r] = Number(kLow).divmod(Number(-1));
    assert(low_q == -Number(kLow) && low_r == Number(0));
    assert(Number(0) / Number(17) == Number(0));
    assert(Number(-10) % Number(7) == Number(-3));
    auto compound = Number(123);
    assert((compound /= Number(-8)) == Number(-15));
    assert((compound %= Number(4)) == Number(-3));
    compound /= compound;
    assert(compound == Number(1));
    compound %= compound;
    assert(compound == Number(0));

    bool threw = false;
    try
    {
        (void)Number(1).divmod(Number(0));
    }
    catch (const std::domain_error &)
    {
        threw = true;
    }
    assert(threw);

    // 筆算の商を上位2桁で減らす場合と、全桁の減算後に除数を戻す場合。
    const auto divisor = Number(kBase / 2) * kBase * kBase + kBase + 1;
    check_magnitudes(divisor, Number(kBase), divisor - kBase);
    check_magnitudes(divisor, Number(kBase), divisor - 1);
    check_magnitudes(Number(kBase + 13), Number(kBase * kBase - 1), Number(kBase));

    // Burnikel–Zieglerのブロック上限、過大な推定商、0の下位ブロック。
    const auto block = Number(std::string("1") + std::string(256, '0'));
    const auto high = Number(std::string(1, Radix == 10 ? '5' : '8') + std::string(255, '0'));
    const auto balanced = high * block + (block - 1);
    check_magnitudes(balanced, block - 1, Number(0));
    check_magnitudes(balanced, block - 1, balanced - 1);
    check_magnitudes(high * block, block - 5, Number(13));

    const auto digits = [&](std::size_t length) {
        constexpr char kAlphabet[] = "0123456789ABCDEF";
        std::string text(length, '0');
        text.front() = kAlphabet[1 + random() % (Radix - 1)];
        for (std::size_t i = 1; i < length; ++i)
        {
            text[i] = kAlphabet[random() % Radix];
        }
        return Number(text);
    };
    for (std::size_t length : {253, 256, 257, 295, 511, 512, 513, 785, 1256})
    {
        const auto b = digits(length);
        for (std::size_t q_length : {1, 56, 65, 137, 582, 1909})
        {
            const auto q = digits(q_length);
            check_magnitudes(b, q, b - 1);
            check_magnitudes(b, q, Number(0));
        }
    }
}
} // namespace

int main()
{
    check<10>();
    check<16>();
}
