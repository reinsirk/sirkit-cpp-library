// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Combination/Binomial_Prefix_Sum.cpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace
{
template <std::uint32_t Mod>
void CheckPascal(std::uint32_t limit)
{
    std::vector<std::vector<std::uint32_t>> sums;
    auto row = std::vector<std::uint32_t>(limit + 1);
    row[0] = 1;
    for (std::uint32_t n = 0; n <= limit; ++n)
    {
        for (std::uint32_t k = n; k > 0; --k)
        {
            row[k] = (row[k] + row[k - 1]) % Mod;
        }
        auto &prefix = sums.emplace_back(n + 1);
        prefix[0] = 1;
        for (std::uint32_t k = 1; k <= n; ++k)
        {
            prefix[k] = (prefix[k - 1] + row[k]) % Mod;
        }
    }
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries;
    for (std::uint32_t n = 0; n <= limit; ++n)
    {
        for (std::uint32_t k = 0; k <= n + 2; ++k)
        {
            queries.emplace_back(n, k);
        }
    }
    std::mt19937 random(82373);
    std::shuffle(queries.begin(), queries.end(), random);
    const auto answers = sirkit::binomial_prefix_sums<Mod>(queries);
    assert(answers.size() == queries.size());
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto [n, k] = queries[i];
        assert(answers[i] == sums[n][std::min(n, k)]);
    }
}
}

int main()
{
    assert(sirkit::binomial_prefix_sums({}).empty());
    CheckPascal<3>(2);
    CheckPascal<101>(100);
    CheckPascal<998244353>(180);
    const std::vector<std::pair<std::uint32_t, std::uint32_t>> odd_half{{8, 2}, {7, 1}};
    assert((sirkit::binomial_prefix_sums<11>(odd_half) == std::vector<std::uint32_t>{4, 8}));
    // 少数の問い合わせで幅の広いブロックに入り、nが前後に動く。
    const std::vector<std::pair<std::uint32_t, std::uint32_t>> sparse{{50, 1}, {49, 2}, {132, 0}, {120, 2}, {111, 1}, {120, 119}};
    const auto result = sirkit::binomial_prefix_sums(sparse);
    assert(result[0] == 51);
    assert(result[1] == 1226);
    assert(result[2] == 1);
    assert(result[3] == 7261);
    assert(result[4] == 112);
    std::uint64_t power = 1;
    for (int i = 0; i < 120; ++i)
    {
        power = (2 * power) % 998244353;
    }
    assert(result[5] == (power + 998244352) % 998244353);
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<std::pair<std::uint32_t, std::uint32_t>> invalid{{7, 0}};
    try
    {
        static_cast<void>(sirkit::binomial_prefix_sums<7>(invalid));
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
#endif
}
