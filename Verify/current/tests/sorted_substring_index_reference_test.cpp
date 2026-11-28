// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Sorted_Substring_Index.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{
template <class Exception, class Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    sirkit::SortedSubstringIndex<> empty;
    assert(empty.empty() && empty.size() == 0);
    assert(empty.is_nondecreasing(0, 0) && empty.is_sorted_slice(0, 0));
    assert(sirkit::SortedSubstringIndex<>("").empty());

    sirkit::SortedSubstringIndex<> letters("zacefz");
    assert(letters.get(0) == 'z' && letters.is_sorted_slice(1, 6));
    letters.set(0, 'b');
    assert(letters.is_nondecreasing(1, 6) && !letters.is_sorted_slice(1, 6));
    letters.set(0, 'z');
    letters.set(5, 'd');
    assert(letters.is_nondecreasing(1, 5) && !letters.is_sorted_slice(1, 5));
    const auto copy = letters;
    letters.set(5, 'z');
    assert(letters.is_sorted_slice(1, 5) && !copy.is_sorted_slice(1, 5));
    const auto moved = std::move(letters);
    assert(letters.empty() && letters.is_sorted_slice(0, 0));
    letters = moved;
    assert(letters.is_sorted_slice(1, 5));

    const std::string bytes{'\x7f', '\x80', '\x81'};
    sirkit::SortedSubstringIndex<3, 127> extended(bytes);
    assert(extended.is_nondecreasing(0, 3) && extended.is_sorted_slice(0, 3));
    extended.set(1, '\x7f');
    assert(extended.is_sorted_slice(0, 3));
    extended.set(2, '\x81');
    assert(extended.get(2) == '\x81');
    sirkit::SortedSubstringIndex<2> one("a");
    one.set(0, 'b');
    assert(one.is_sorted_slice(0, 1));

    std::mt19937 gen(277638);
    for (int round = 0; round < 180; ++round)
    {
        const auto n = static_cast<int>(gen() % 61 + 1);
        std::string text(static_cast<std::size_t>(n), 'a');
        const auto random_byte = [&]()
        {
            return static_cast<char>('a' + gen() % 8);
        };
        for (auto &byte : text)
        {
            byte = random_byte();
        }
        sirkit::SortedSubstringIndex<> index(text);
        assert(!index.empty() && index.size() == n);
        for (int query = 0; query < 120; ++query)
        {
            const auto position = static_cast<int>(gen() % static_cast<unsigned>(n));
            const auto byte = random_byte();
            index.set(position, byte);
            text[static_cast<std::size_t>(position)] = byte;
            assert(index.get(position) == byte);

            auto left = static_cast<int>(gen() % static_cast<unsigned>(n + 1));
            auto right = static_cast<int>(gen() % static_cast<unsigned>(n + 1));
            if (left > right)
            {
                std::swap(left, right);
            }
            auto sorted = text;
            std::ranges::sort(sorted);
            const auto part = std::string_view(text).substr(static_cast<std::size_t>(left), static_cast<std::size_t>(right - left));
            assert(index.is_nondecreasing(left, right) == std::ranges::is_sorted(part));
            assert(index.is_sorted_slice(left, right) == (sorted.find(part) != std::string::npos));
        }
    }

    expect_exception<std::length_error>([]()
    {
        sirkit::detail::CheckSortedSubstringLength(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&]() { static_cast<void>(letters.get(-1)); });
    expect_exception<std::out_of_range>([&]() { letters.set(6, 'a'); });
    expect_exception<std::out_of_range>([&]() { letters.set(0, '#'); });
    expect_exception<std::out_of_range>([&]() { letters.set(0, '{'); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(letters.is_sorted_slice(-1, 1)); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(letters.is_sorted_slice(2, 1)); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(letters.is_nondecreasing(0, 7)); });
    assert(letters.get(0) == 'z');
#endif
}
