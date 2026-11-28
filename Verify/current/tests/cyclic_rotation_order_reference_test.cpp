// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Cyclic_Rotation_Order.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
bool byte_less(std::string_view left, std::string_view right)
{
    return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end(), [](char x, char y)
    {
        return static_cast<unsigned char>(x) < static_cast<unsigned char>(y);
    });
}

std::vector<std::string> rotations(std::string_view text)
{
    std::vector<std::string> all;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        std::string rotated(text.substr(i));
        rotated.append(text.substr(0, i));
        all.push_back(rotated);
    }
    return all;
}

void check(std::span<const std::string_view> inputs)
{
    std::vector<std::string> joined;
    for (const auto text : inputs)
    {
        const auto row = rotations(text);
        joined.insert(joined.end(), row.begin(), row.end());
    }
    auto ordered = joined;
    std::sort(ordered.begin(), ordered.end(), byte_less);
    ordered.erase(std::unique(ordered.begin(), ordered.end()), ordered.end());
    std::vector<int> expected;
    for (const auto &word : joined)
    {
        expected.push_back(static_cast<int>(std::lower_bound(ordered.begin(), ordered.end(), word, byte_less) - ordered.begin()));
    }
    assert(sirkit::cyclic_rotation_ranks(inputs) == expected);
    for (const auto text : inputs)
    {
        const auto row = rotations(text);
        std::vector<int> order(row.size());
        std::iota(order.begin(), order.end(), 0);
        std::stable_sort(order.begin(), order.end(), [&](int x, int y)
        {
            return byte_less(row[static_cast<std::size_t>(x)], row[static_cast<std::size_t>(y)]);
        });
        assert(sirkit::cyclic_rotation_order(text) == order);
    }
    if (inputs.size() >= 2)
    {
        std::uint64_t pairs = 0;
        for (const auto &x : rotations(inputs[0]))
        {
            for (const auto &y : rotations(inputs[1]))
            {
                pairs += !byte_less(y, x);
            }
        }
        assert(sirkit::count_cyclic_rotation_pairs(inputs[0], inputs[1]) == pairs);
    }
}
}

int main()
{
    assert(sirkit::cyclic_rotation_ranks(std::span<const std::string_view>{}).empty());
    assert(sirkit::cyclic_rotation_ranks(std::string_view{}).empty());
    assert(sirkit::cyclic_rotation_order("").empty());
    assert(sirkit::count_cyclic_rotation_pairs("", "") == 0);
    check(std::array<std::string_view, 3>{"ababab", "bababa", "cacaca"});
    check(std::array<std::string_view, 2>{"aaaa", "aaaa"});
    check(std::array<std::string_view, 2>{"a", "b"});
    const std::string left{'\xff', '\x80', '\0'};
    const std::string right{'\0', '\x80', '\xff'};
    check(std::array<std::string_view, 2>{left, right});

    std::mt19937 gen(94314);
    for (int round = 0; round < 2000; ++round)
    {
        const std::size_t width = gen() % 14;
        const std::size_t count = gen() % 5 + 1;
        std::vector<std::string> words(count, std::string(width, 'a'));
        const int alphabet = static_cast<int>(gen() % 7 + 1);
        for (auto &word : words)
        {
            for (auto &letter : word)
            {
                letter = static_cast<char>('a' + static_cast<int>(gen() % static_cast<unsigned>(alphabet)));
            }
        }
        const std::vector<std::string_view> views(words.begin(), words.end());
        check(views);
    }

    bool unequal_rejected = false;
    try
    {
        static_cast<void>(sirkit::count_cyclic_rotation_pairs("", "a"));
    }
    catch (const std::invalid_argument &)
    {
        unequal_rejected = true;
    }
    assert(unequal_rejected);

    bool capacity_rejected = false;
    try
    {
        const auto width = static_cast<std::size_t>(std::numeric_limits<int>::max());
        static_cast<void>(sirkit::detail::checked_rotation_count(width, 1));
    }
    catch (const std::length_error &)
    {
        capacity_rejected = true;
    }
    assert(capacity_rejected);
}
