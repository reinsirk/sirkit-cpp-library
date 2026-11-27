// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Summatory_Totient.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    assert(sirkit::summatory_totient(0) == 0);
    assert(sirkit::summatory_totient(10000, 1) == 0);
    assert(sirkit::summatory_totient(1, 2) == 1);
    std::uint64_t sum = 0;
    for (unsigned n = 1; n < 200; ++n)
    {
        for (unsigned i = 1; i <= n; ++i)
        {
            sum += std::gcd(i, n) == 1;
        }
        assert(sirkit::summatory_totient(n) == sum);
        assert(sirkit::summatory_totient(n, 12) == sum % 12);
    }

    constexpr unsigned kLimit = 1'000'000;
    std::vector<unsigned> phi(kLimit + 1);
    std::iota(phi.begin(), phi.end(), 0u);
    for (unsigned p = 2; p <= kLimit; ++p)
    {
        if (phi[p] == p)
        {
            for (unsigned i = p; i <= kLimit; i += p)
            {
                phi[i] -= phi[i] / p;
            }
        }
    }
    std::vector<std::uint64_t> prefix(kLimit + 1);
    for (unsigned i = 1; i <= kLimit; ++i)
    {
        prefix[i] = prefix[i - 1] + phi[i];
    }
    assert(prefix.back() == 303'963'552'392ULL);
    std::mt19937 random(753642);
    constexpr std::array<std::uint32_t, 4> kMods{2, 1000, 998244353, std::numeric_limits<std::uint32_t>::max()};
    for (unsigned i = 0; i < 350; ++i)
    {
        const unsigned n = random() % kLimit + 1;
        const std::uint32_t mod = kMods[i % kMods.size()];
        assert(sirkit::summatory_totient(n, mod) == prefix[n] % mod);
    }
    for (unsigned i = 80; i <= 100; ++i)
    {
        const auto n = i * i * i;
        const auto mod = kMods.back();
        assert(sirkit::summatory_totient(n, mod) == prefix[n] % mod);
        assert(sirkit::summatory_totient(n - 1, mod) == prefix[n - 1] % mod);
    }
    try
    {
        static_cast<void>(sirkit::summatory_totient(1, 0));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        static_cast<void>(sirkit::summatory_totient(std::numeric_limits<std::uint64_t>::max()));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
