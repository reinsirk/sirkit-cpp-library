// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_STATIC_RANGE_LIS_DEFINED
#include "../../../Algorithm/Static_Range_LIS.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
template <class Symbol>
void compare(const std::vector<Symbol> &sequence)
{
    const sirkit::StaticRangeLIS lis(sequence);
    for (std::size_t left = 0; left <= sequence.size(); ++left)
    {
        std::vector<Symbol> tails;
        assert(lis.length(left, left) == 0);
        for (std::size_t right = left; right < sequence.size(); ++right)
        {
            const auto where = std::ranges::lower_bound(tails, sequence[right]);
            if (where == tails.end())
            {
                tails.push_back(sequence[right]);
            }
            else
            {
                *where = sequence[right];
            }
            assert(lis.length(left, right + 1) == tails.size());
        }
    }
}

#ifndef SKIP_RUNTIME_CHECKS
void expect_invalid(const sirkit::StaticRangeLIS &lis, std::size_t left, std::size_t right)
{
    bool threw = false;
    try
    {
        (void)lis.length(left, right);
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }
    assert(threw);
}
#endif
} // namespace

int main()
{
    for (std::size_t size = 0; size <= 7; ++size)
    {
        std::vector<int> permutation(size);
        std::iota(permutation.begin(), permutation.end(), 0);
        do
        {
            compare(permutation);
        } while (std::ranges::next_permutation(permutation).found);
    }

    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    compare(std::vector<std::int64_t>{minimum, maximum, minimum, 0, -1, maximum, -1, 0});
    compare(std::vector<std::uint64_t>{std::numeric_limits<std::uint64_t>::max(), 0, 1, 1, 0, 2});

    std::mt19937 random(181382);
    for (int trial = 0; trial < 800; ++trial)
    {
        std::vector<int> sequence(random() % 65);
        const int symbols = trial % 2 == 0 ? 5 : 1000;
        for (auto &value : sequence)
        {
            value = static_cast<int>(random() % symbols) - symbols / 2;
        }
        compare(sequence);
    }

#ifndef SKIP_RUNTIME_CHECKS
    const sirkit::StaticRangeLIS lis(std::array{1, 2, 3});
    expect_invalid(lis, 2, 1);
    expect_invalid(lis, 0, 4);
#endif

    // 各ブロック内は減少、ブロック間は増加。区間に触れるブロック数がLIS長。
    constexpr std::size_t kMaxSize = 100'000, kBlock = 25;
    std::vector<std::size_t> sequence(kMaxSize);
    for (std::size_t i = 0; i < sequence.size(); ++i)
    {
        sequence[i] = i / kBlock * kBlock + (kBlock - 1 - i % kBlock);
    }
    const sirkit::StaticRangeLIS large(sequence);
    for (int query = 0; query < 100'000; ++query)
    {
        const auto l = random() % (kMaxSize + 1), r = l + random() % (kMaxSize + 1 - l);
        const auto answer = r == l ? 0 : (r - 1) / kBlock - l / kBlock + 1;
        assert(large.length(l, r) == answer);
    }
}
