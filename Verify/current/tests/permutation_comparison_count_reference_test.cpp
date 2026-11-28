// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Permutation_Comparison_Count.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
template <std::uint32_t Prime>
std::uint32_t CountByRank(std::string_view comparisons)
{
    std::vector<std::uint32_t> counts{1};
    for (const char sign : comparisons)
    {
        const auto total = std::accumulate(counts.begin(), counts.end(), std::uint64_t{0}) % Prime;
        std::uint64_t lower = 0;
        std::vector<std::uint32_t> next(counts.size() + 1);
        for (std::size_t rank = 0; rank < next.size(); ++rank)
        {
            auto ways = total;
            if (sign == '<')
            {
                ways = lower;
            }
            else if (sign == '>')
            {
                ways = (total + Prime - lower) % Prime;
            }
            next[rank] = static_cast<std::uint32_t>(ways);
            if (rank < counts.size())
            {
                lower = (lower + counts[rank]) % Prime;
            }
        }
        counts.swap(next);
    }
    return static_cast<std::uint32_t>(std::accumulate(counts.begin(), counts.end(), std::uint64_t{0}) % Prime);
}

std::uint32_t CountByEnumeration(std::string_view comparisons)
{
    std::vector<int> order(comparisons.size() + 1);
    std::iota(order.begin(), order.end(), 0);
    std::uint32_t count = 0;
    do
    {
        bool matches = true;
        for (std::size_t i = 0; i < comparisons.size(); ++i)
        {
            const char sign = order[i] < order[i + 1] ? '<' : '>';
            if (comparisons[i] != '?' && comparisons[i] != sign)
            {
                matches = false;
                break;
            }
        }
        count += static_cast<std::uint32_t>(matches);
    } while (std::next_permutation(order.begin(), order.end()));
    return count;
}
} // namespace

int main()
{
    using sirkit::count_permutations_by_comparisons;
    std::mt19937 random(650723);
    assert(count_permutations_by_comparisons("") == 1);
    assert(count_permutations_by_comparisons<2>("") == 1);
    assert(count_permutations_by_comparisons("<<<<") == 1);
    assert(count_permutations_by_comparisons(">>>>") == 1);
    assert(count_permutations_by_comparisons("????") == 120);
    assert(count_permutations_by_comparisons("><>") == 5);
    assert(count_permutations_by_comparisons("><>>") == 9);
    assert(count_permutations_by_comparisons("><>>?<") == 189);

    constexpr std::string_view comparisons = "<>?";
    for (int trial = 0; trial < 190; ++trial)
    {
        std::string pattern(random() % 8, '?');
        for (auto &sign : pattern)
        {
            sign = comparisons[random() % comparisons.size()];
        }
        const auto count = CountByEnumeration(pattern);
        assert(CountByRank<998244353>(pattern) == count);
        assert(count_permutations_by_comparisons(pattern) == count);
    }

    for (int trial = 0; trial < 140; ++trial)
    {
        std::string pattern(105 + random() % 220, '?');
        const auto choices = trial % 2 == 0 ? 2 : comparisons.size();
        for (auto &sign : pattern)
        {
            sign = comparisons[random() % choices];
        }
        assert(count_permutations_by_comparisons(pattern) == CountByRank<998244353>(pattern));
        assert(count_permutations_by_comparisons<1'000'000'007>(pattern) == CountByRank<1'000'000'007>(pattern));
        assert(count_permutations_by_comparisons<4'294'967'291>(pattern) == CountByRank<4'294'967'291>(pattern));
    }

    for (int trial = 0; trial < 512; ++trial)
    {
        std::string pattern(11, '?');
        for (auto &sign : pattern)
        {
            sign = comparisons[random() % 2];
        }
        assert(count_permutations_by_comparisons<13>(pattern) == CountByRank<13>(pattern));
    }

    bool invalid_sign = false;
    try
    {
        static_cast<void>(count_permutations_by_comparisons("<!>"));
    }
    catch (const std::invalid_argument &)
    {
        invalid_sign = true;
    }
    assert(invalid_sign);

    bool invalid_length = false;
    try
    {
        static_cast<void>(count_permutations_by_comparisons<7>("<<<<<<"));
    }
    catch (const std::length_error &)
    {
        invalid_length = true;
    }
    assert(invalid_length);
}
