// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_BINOMIAL_PRIMES_SOURCE
#define SIRKIT_TEST_BINOMIAL_PRIMES_SOURCE "../../../Math/Combination/ModInt_Binomial.cpp"
#endif
#include SIRKIT_TEST_BINOMIAL_PRIMES_SOURCE

#include <atcoder/modint.hpp>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

int main() {
    using Mint = atcoder::dynamic_modint<149>;
    for (const int prime : {61, 469762049, 754974721, 1000000009, 1004535809, 1224736769, 2013265921}) {
        Mint::set_mod(prime);
        sirkit::BinomialMod integer(prime, 4, {17, 64});
        sirkit::ModIntBinomial<Mint> mint(4, {17, 64});
        assert(integer.is_prime_modulus());
        std::array<std::uint64_t, 41> row{};
        row[0] = 1;
        for (int n = 0; n <= 40; ++n) {
            for (int k = 0; k <= n; ++k) {
                assert(integer.COM(n, k) == row[k]);
                assert(static_cast<std::uint64_t>(mint.COM(n, k).val()) == row[k]);
            }
            if (n != 40) for (int k = n + 1; k >= 1; --k) row[k] = (row[k] + row[k - 1]) % prime;
        }
        assert(integer.COM(-1, 0) == 0 && integer.COM(4, -1) == 0 && integer.COM(4, 5) == 0);
        assert(mint.COM(-1, 0).val() == 0 && mint.COM(4, -1).val() == 0 && mint.COM(4, 5).val() == 0);
    }
    constexpr std::uint32_t tail = 65537u * 521;
    for (int first_factor : {2, 3, 5, 7}) {
        const std::uint32_t mod = first_factor * tail;
        Mint::set_mod(static_cast<int>(mod));
        sirkit::ModIntBinomial<Mint> mint(0, {0, 24});
        sirkit::BinomialMod integer(mod, 0, {0, 24});
        assert(!integer.is_prime_modulus());
        constexpr std::uint64_t n = 99;
        assert(integer.COM(n, 2) == n * (n - 1) / 2 % mod);
        assert(mint.COM(n, 2).val() == n * (n - 1) / 2 % mod);
    }
    constexpr std::uint32_t kPrime = 10007;
    Mint::set_mod(kPrime);
    sirkit::BinomialMod full_integer(kPrime, 3, {kPrime - 1, 0});
    sirkit::ModIntBinomial<Mint> full_mint(3, {kPrime - 1, 0});
    full_integer.prepare(kPrime - 1);
    full_mint.prepare(kPrime - 1);
    assert(full_integer.COM(kPrime - 1, 2) == 1);
    assert(full_mint.COM(kPrime - 1, 2).val() == 1);
    for (int start : {2, 3}) {
        sirkit::BinomialMod integer(kPrime, start, {kPrime - 1, 24});
        sirkit::ModIntBinomial<Mint> mint(start, {kPrime - 1, 24});
        for (const std::uint64_t n : {4900u, 4901u, kPrime - 1}) {
            integer.prepare(n);
            mint.prepare(n);
            const auto expected = n * (n - 1) / 2 % kPrime;
            assert(integer.COM(n, 2) == expected);
            assert(mint.COM(n, 2).val() == expected);
        }
    }
    // int64の上端に近い桁を持つ素数冪の照会。n≡-1 (mod 81) なので C(n,2)≡1。
    constexpr std::int64_t n_max = std::numeric_limits<std::int64_t>::max();
    constexpr std::int64_t n = n_max / 81 * 81 - 1;
    sirkit::BinomialMod large_index(81, 0, {0, 24});
    assert(large_index.COM(n, 2) == 1);
    // 実際の係数が法-1となる合成法。CRTの復元時にも法-1に戻る。
    constexpr std::int64_t kCrtIndex = 65536;
    constexpr std::int64_t kCrtExpected = kCrtIndex * (kCrtIndex - 1) / 2;
    Mint::set_mod(static_cast<int>(kCrtExpected + 1));
    sirkit::ModIntBinomial<Mint> crt_mint(0, {0, 24});
    sirkit::BinomialMod crt(kCrtExpected + 1, 0, {0, 24});
    assert(crt_mint.COM(kCrtIndex, 2).val() == kCrtExpected);
    assert(crt.COM(kCrtIndex, 2) == kCrtExpected);
    const sirkit::BinomialMod with_options(11, sirkit::BinomialOptions{10, 24});
    assert(with_options.mod() == 11);
}
