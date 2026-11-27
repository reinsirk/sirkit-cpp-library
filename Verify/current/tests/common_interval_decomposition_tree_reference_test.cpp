// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_COMMON_INTERVAL_DECOMPOSITION_DEFINED
#include "../../../Algorithm/Common_Interval_Decomposition.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Kind = sirkit::CommonIntervalKind;
using Range = std::pair<int, int>;

std::vector<Range> find_strong(const std::vector<int> &permutation)
{
    std::vector<Range> connected, strong;
    const int n = static_cast<int>(permutation.size());
    for (int l = 0; l < n; ++l)
    {
        int low = permutation[l], high = low;
        for (int r = l; r < n; ++r)
        {
            low = std::min(low, permutation[r]);
            high = std::max(high, permutation[r]);
            if (high - low == r - l)
            {
                connected.emplace_back(l, r + 1);
            }
        }
    }
    for (const auto &[l, r] : connected)
    {
        if (std::ranges::none_of(connected, [&](Range interval)
                                 {
                                     const auto [a, b] = interval;
                                     return (l < a && a < r && r < b) || (a < l && l < b && b < r);
                                 }))
        {
            strong.emplace_back(l, r);
        }
    }
    return strong;
}

void compare(const std::vector<int> &permutation)
{
    const auto answer = sirkit::common_interval_decomposition(permutation);
    const auto reference = find_strong(permutation);
    assert(answer.size() == reference.size());
    std::map<Range, int> indexes;
    for (std::size_t i = 0; i < answer.size(); ++i)
    {
        const auto &node = answer[i];
        assert(0 <= node.left && node.left < node.right && node.right <= static_cast<int>(permutation.size()));
        assert(node.parent >= -1 && node.parent < static_cast<int>(i));
        const auto [low, high] = std::minmax_element(permutation.begin() + node.left,
                                                     permutation.begin() + node.right);
        assert(node.value_begin == *low && node.value_end == *high + 1);
        const auto [position, inserted] = indexes.emplace(Range{node.left, node.right}, i);
        assert(inserted);
    }
    std::vector<std::vector<int>> children(answer.size());
    for (const auto &range : reference)
    {
        const auto i = indexes.at(range);
        const auto [l, r] = range;
        const int n = static_cast<int>(permutation.size());
        int parent = -1, length = n + 1;
        for (const auto &candidate : reference)
        {
            const auto [a, b] = candidate;
            if (candidate != range && a <= l && r <= b && b - a < length)
            {
                length = b - a;
                parent = indexes.at(candidate);
            }
        }
        assert(answer[i].parent == parent);
        if (parent >= 0)
        {
            children[parent].push_back(i);
        }
    }
    for (std::size_t i = 0; i < answer.size(); ++i)
    {
        const auto &direct = children[i];
        if (direct.empty())
        {
            assert(answer[i].kind == Kind::leaf);
            continue;
        }
        bool linear = true;
        for (std::size_t x = 0; x < direct.size(); ++x)
        {
            int low = answer[direct[x]].value_begin, high = answer[direct[x]].value_end;
            for (std::size_t y = x; y < direct.size(); ++y)
            {
                low = std::min(low, answer[direct[y]].value_begin);
                high = std::max(high, answer[direct[y]].value_end);
                linear &= high - low == answer[direct[y]].right - answer[direct[x]].left;
            }
        }
        const auto kind = linear ? (answer[direct.front()].value_begin < answer[direct.back()].value_begin
                                        ? Kind::increasing
                                        : Kind::decreasing)
                                 : Kind::prime;
        assert(answer[i].kind == kind);
    }
}

template <class Exception, class Call>
void expect_exception(Call call)
{
    bool failed = false;
    try
    {
        call();
    }
    catch (const Exception &)
    {
        failed = true;
    }
    assert(failed);
}
} // namespace

int main()
{
    for (int n = 0; n <= 8; ++n)
    {
        std::vector<int> permutation(n);
        std::iota(permutation.begin(), permutation.end(), 0);
        do
        {
            compare(permutation);
        } while (std::ranges::next_permutation(permutation).found);
    }
    expect_exception<std::length_error>([]
                                        {
                                            (void)sirkit::detail::CommonIntervalSize(std::numeric_limits<std::size_t>::max());
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &invalid : {std::vector<int>{-1}, std::vector<int>{1}, std::vector<int>{0, 0}})
    {
        expect_exception<std::invalid_argument>([&]
                                                {
                                                    (void)sirkit::common_interval_decomposition(invalid);
                                                });
    }
#endif
    std::mt19937 random(10811);
    for (int i = 0; i < 800; ++i)
    {
        std::vector<int> permutation(random() % 46);
        std::iota(permutation.begin(), permutation.end(), 0);
        std::shuffle(permutation.begin(), permutation.end(), random);
        compare(permutation);
    }

    // 長さN-1の深さを持つ、昇順節と降順節が交互に現れる木。
    constexpr int kMaxN = 500'000;
    std::vector<int> permutation(kMaxN);
    for (int i = 0; i < kMaxN; ++i)
    {
        permutation[i] = i % 2 == 0 ? i / 2 : kMaxN - 1 - i / 2;
    }
    const auto deep = sirkit::common_interval_decomposition(permutation);
    assert(deep.size() == 2 * kMaxN - 1);
    assert(deep.front().parent == -1);
    for (std::size_t i = 0; i < deep.size(); ++i)
    {
        const auto &node = deep[i];
        const bool leaf = node.left + 1 == node.right;
        const auto expected = leaf ? Kind::leaf : (node.left % 2 == 0 ? Kind::increasing : Kind::decreasing);
        assert(node.kind == expected);
        if (!leaf)
        {
            assert(node.right == kMaxN);
        }
        if (node.parent >= 0)
        {
            assert(node.parent < static_cast<int>(i));
            assert(deep[node.parent].left == std::min(node.left - !leaf, kMaxN - 2));
        }
    }
}
