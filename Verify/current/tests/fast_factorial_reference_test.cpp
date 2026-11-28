// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Fast_Factorial.cpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

std::uint32_t Multiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(static_cast<std::uint64_t>(a) * b % kMod);
}
}

int main()
{
    assert(sirkit::factorials_mod_998244353({}).empty());
    assert(sirkit::factorial_mod_998244353(0) == 1);
    assert(sirkit::factorial_mod_998244353(6) == 720);
    assert(sirkit::factorial_mod_998244353(kMod - 1) == kMod - 1);
    assert(sirkit::factorial_mod_998244353(kMod - 2) == 1);
    assert(sirkit::factorial_mod_998244353(kMod) == 0);
    assert(sirkit::factorial_mod_998244353(std::numeric_limits<std::uint64_t>::max()) == 0);

    constexpr std::uint32_t kMax = 250000;
    std::vector<std::uint32_t> expected(kMax + 1, 1);
    for (std::uint32_t i = 1; i <= kMax; ++i)
    {
        expected[i] = Multiply(expected[i - 1], i);
    }
    std::mt19937 random(25773);
    std::vector<std::uint64_t> queries{0, 1, 8191, 8192, 8193, kMax, kMax, kMod, kMod - 1, kMod - 1 - kMax};
    for (int i = 0; i < 550; ++i)
    {
        const auto value = random() % (kMax + 1);
        queries.push_back(value);
        queries.push_back(kMod - 1 - value);
    }
    std::shuffle(queries.begin(), queries.end(), random);
    const auto answer = sirkit::factorials_mod_998244353(queries);
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        if (queries[i] == kMod)
        {
            assert(answer[i] == 0);
        }
        else if (queries[i] <= kMax)
        {
            assert(answer[i] == expected[queries[i]]);
        }
        else
        {
            const auto reflected = kMod - 1 - queries[i];
            assert(Multiply(answer[i], expected[reflected]) == (reflected % 2 == 0 ? kMod - 1 : 1));
        }
    }
}
