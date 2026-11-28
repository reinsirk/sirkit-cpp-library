// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Short_Simple_Path.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
bool can_finish(const std::vector<std::vector<int>> &graph, std::span<const int> ends,
                unsigned remaining, int vertex, std::vector<bool> &used)
{
    if (remaining == 0)
    {
        return std::ranges::find(ends, vertex) != ends.end();
    }
    for (const int next : graph[static_cast<std::size_t>(vertex)])
    {
        const auto v = static_cast<std::size_t>(next);
        if (!used[v])
        {
            used[v] = true;
            const bool found = can_finish(graph, ends, remaining - 1, next, used);
            used[v] = false;
            if (found)
            {
                return true;
            }
        }
    }
    return false;
}

void compare(const std::vector<std::vector<int>> &graph, unsigned length,
             const std::vector<int> &starts, const std::vector<int> &ends)
{
    std::vector<bool> used(graph.size());
    bool expected = false;
    for (const int start : starts)
    {
        used[static_cast<std::size_t>(start)] = true;
        expected |= can_finish(graph, ends, length - 1, start, used);
        used[static_cast<std::size_t>(start)] = false;
    }
    const auto found = sirkit::find_short_simple_path(graph, length, starts, ends);
    assert(found.has_value() == expected);
    if (found)
    {
        assert(found->size() == length);
        assert(std::ranges::find(starts, found->front()) != starts.end());
        assert(std::ranges::find(ends, found->back()) != ends.end());
        for (std::size_t i = 0; i < found->size(); ++i)
        {
            const auto v = static_cast<std::size_t>((*found)[i]);
            assert(v < graph.size() && !used[v]);
            used[v] = true;
            if (i + 1 < found->size())
            {
                assert(std::ranges::find(graph[v], (*found)[i + 1]) != graph[v].end());
            }
        }
    }
}
} // namespace

int main()
{
    using sirkit::find_short_simple_path;
    assert(!find_short_simple_path({}, 1, {}, {}));
    compare({{}}, 1, {0, 0}, {0, 0});
    compare({{}, {}}, 1, {1}, {0});
    compare({{1}, {}}, 2, {0}, {1});
    compare({{1}, {}}, 2, {0}, {});
    compare({{1}, {}}, 3, {0}, {1});
    compare({{0, 1, 1}, {0, 2}, {2}}, 3, {0, 0}, {2});

    for (const auto invalid_length : {0U, 9U})
    {
        bool rejected = false;
        try
        {
            static_cast<void>(find_short_simple_path({}, invalid_length, {}, {}));
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        assert(rejected);
    }
    bool rejected = false;
    try
    {
        sirkit::detail::ShortPathCheckParameters(sirkit::detail::kShortPathMod, 4);
    }
    catch (const std::length_error &)
    {
        rejected = true;
    }
    assert(rejected);

    const std::array<std::vector<int>, 1> graph{{{}}};
    const std::array<std::vector<int>, 1> invalid_graph{{{1}}};
    const std::array<int, 1> negative{-1}, valid{0}, past_end{1};
    for (const auto which : {0, 1, 2})
    {
        rejected = false;
        try
        {
            if (which == 0)
            {
                static_cast<void>(find_short_simple_path(invalid_graph, 1, valid, valid));
            }
            if (which == 1)
            {
                static_cast<void>(find_short_simple_path(graph, 1, negative, valid));
            }
            if (which == 2)
            {
                static_cast<void>(find_short_simple_path(graph, 1, valid, past_end));
            }
        }
        catch (const std::out_of_range &)
        {
            rejected = true;
        }
        assert(rejected);
    }

    std::mt19937 rng(82872);
    for (unsigned i = 0; i < 250; ++i)
    {
        const std::size_t n = 1 + rng() % 10;
        std::vector<std::vector<int>> directed(n);
        std::vector<int> starts, ends;
        for (std::size_t u = 0; u < n; ++u)
        {
            if (rng() % 4 == 0)
            {
                starts.push_back(static_cast<int>(u));
            }
            if (rng() % 3 == 0)
            {
                ends.push_back(static_cast<int>(u));
            }
            for (std::size_t v = 0; v < n; ++v)
            {
                if (rng() % 8 <= i % 8)
                {
                    directed[u].push_back(static_cast<int>(v));
                }
            }
        }
        for (unsigned k = 1; k <= 8; ++k)
        {
            compare(directed, k, starts, ends);
        }
    }
}
