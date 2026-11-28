// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Adjacent_Swap_Distance.cpp"
#include "../../../Algorithm/Range_Inversions.cpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    const std::string_view empty;
    assert(sirkit::adjacent_swap_distance(empty, empty) == 0);
    assert(!sirkit::adjacent_swap_distance(empty, std::string_view("a")));
    assert(!sirkit::adjacent_swap_permutation(std::string_view("a"), std::string_view("b")));
    assert(!sirkit::adjacent_swap_permutation(std::string_view("b"), std::string_view("a")));

    std::mt19937 generator(192311);
    for (std::size_t trial = 0; trial < 100; ++trial)
    {
        std::string from(1 + trial % 8, 'a');
        for (auto &value : from)
        {
            value += static_cast<char>(generator() % 3);
        }
        std::map<std::string, std::uint64_t> distances{{from, 0}};
        std::vector<std::string> queue{from};
        for (std::size_t head = 0; head < queue.size(); ++head)
        {
            auto next = queue[head];
            const auto distance = distances.at(next);
            assert(sirkit::adjacent_swap_distance(from, next) == distance);
            const auto order = sirkit::adjacent_swap_permutation(from, next);
            assert(order);
            std::string rearranged(from.size(), '\0');
            for (std::size_t i = 0; i < from.size(); ++i)
            {
                rearranged[(*order)[i]] = from[i];
                for (std::size_t j = 0; j < i; ++j)
                {
                    assert(from[i] != from[j] || (*order)[j] < (*order)[i]);
                }
            }
            assert(rearranged == next);
            for (std::size_t i = 1; i < next.size(); ++i)
            {
                std::swap(next[i], next[i - 1]);
                if (distances.try_emplace(next, distance + 1).second)
                {
                    queue.push_back(next);
                }
                std::swap(next[i], next[i - 1]);
            }
        }
    }

    const std::vector from{31, 12, 21, 2};
    const int to[]{42, 51, 22, 11};
    const auto compare = [](int left, int right) { return left % 10 > right % 10; };
    assert(sirkit::adjacent_swap_distance(from, to, compare) == 2);
}
