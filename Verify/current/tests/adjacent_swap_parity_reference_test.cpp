// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Adjacent_Swap_Parity.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Sequence = std::vector<int>;
using State = std::pair<Sequence, bool>;

sirkit::AdjacentSwapParityDistances MinByBfs(const Sequence &from, const Sequence &to)
{
    std::map<State, std::uint64_t> distances;
    std::queue<State> pending;
    distances.emplace(State{from, false}, 0);
    pending.emplace(from, false);
    while (!pending.empty())
    {
        const auto [values, odd] = pending.front();
        pending.pop();
        const auto moves = distances.at({values, odd}) + 1;
        for (std::size_t i = 1; i < values.size(); ++i)
        {
            auto next = values;
            std::swap(next[i - 1], next[i]);
            if (distances.emplace(State{next, !odd}, moves).second)
            {
                pending.emplace(std::move(next), !odd);
            }
        }
    }
    const auto even = distances.find({to, false});
    const auto odd = distances.find({to, true});
    return {even == distances.end() ? std::nullopt : std::optional{even->second},
            odd == distances.end() ? std::nullopt : std::optional{odd->second}};
}

void Verify(const Sequence &from, const Sequence &to)
{
    const auto answer = sirkit::adjacent_swap_distances_by_parity(from, to);
    const auto brute = MinByBfs(from, to);
    assert(answer.even == brute.even && answer.odd == brute.odd);
}

struct SizedSequence
{
    std::size_t length;
    int value = 0;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return length;
    }

    [[nodiscard]] int operator[](std::size_t) const noexcept
    {
        return value;
    }
};
} // namespace

int main()
{
    Verify({}, {});
    Verify({}, {1});
    Verify({1}, {1});
    Verify({1, 5}, {1, 4});
    Verify({1, 4}, {1, 5});
    Verify({1, 2}, {2, 1});
    Verify({1, 1}, {1, 1});
    Verify({1, 2, 3, 1}, {1, 3, 2, 1});
    Verify({1, 2, 3, 4, 1, 2, 3, 4}, {4, 3, 2, 1, 4, 3, 2, 1});
    Verify({1, 2, 3, 4, 5, 6, 7}, {7, 6, 5, 4, 3, 2, 1});
    const std::array from{17, 25, 18, 27}, to{17, 28, 15, 37};
    const auto with_comparison = sirkit::adjacent_swap_distances_by_parity(
        from, to, [](const int &a, const int &b)
        {
            return a % 10 > b % 10;
        });
    assert(with_comparison.even == 6 && with_comparison.odd == 1);

    std::mt19937 random(51295);
    for (int trial = 0; trial < 510; ++trial)
    {
        Sequence a(random() % 8);
        for (auto &value : a)
        {
            value = static_cast<int>(random() % 5) - 2;
        }
        auto b = a;
        std::shuffle(b.begin(), b.end(), random);
        if (trial % 11 == 0 && !b.empty())
        {
            b.back() = -10;
        }
        Verify(a, b);
    }

    assert(sirkit::adjacent_swap_distances_by_parity(SizedSequence{1, 2}, SizedSequence{1, 2}).even == 0);
    assert(!sirkit::adjacent_swap_distances_by_parity(SizedSequence{1, 2}, SizedSequence{1, 1}).even);
    assert(!sirkit::adjacent_swap_distances_by_parity(SizedSequence{1, 2}, SizedSequence{1, 3}).odd);

    bool too_long = false;
    try
    {
        const SizedSequence huge{static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1};
        static_cast<void>(sirkit::adjacent_swap_distances_by_parity(huge, huge));
    }
    catch (const std::length_error &)
    {
        too_long = true;
    }
    assert(too_long);
}
