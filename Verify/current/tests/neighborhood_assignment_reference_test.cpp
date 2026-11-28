// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Neighborhood_Assignment.cpp"

#include <array>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Colors = sirkit::NeighborhoodAssignment<int>;

struct Label
{
    static inline bool refuse_copy = false;
    int id;
    explicit Label(int value) : id(value)
    {
    }
    Label(const Label &other) : id(other.id)
    {
        if (refuse_copy)
        {
            throw std::runtime_error("copy refused");
        }
    }
};

template <class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_labels()
{
    const std::array initial{Label(1), Label(2), Label(3), Label(4)};
    const std::array edges{Edge{0, 1}, Edge{0, 2}, Edge{0, 3}};
    sirkit::NeighborhoodAssignment<Label> data(initial, edges, 1);
    data.assign_neighbors(0, Label(20));
    assert(data.get(0).id == 1 && data.get(1).id == 20);
    Label::refuse_copy = true;
    must_throw<std::runtime_error>([&]
                                   {
                                       data.assign_neighbors(0, Label(7), true);
                                   });
    must_throw<std::runtime_error>([&]
                                   {
                                       data.assign(1, Label(3));
                                   });
    Label::refuse_copy = false;
    assert(data.get(0).id == 1 && data.get(1).id == 20);
    data.assign(1, Label(12));
    assert(data.get(1).id == 12 && data.get(2).id == 20);
    data.assign_neighbors(1, data.get(1), true);
    assert(data.get(0).id == 12 && data.get(1).id == 12 && data.get(2).id == 20);
}

void check_limits()
{
    assert(Colors({}, {}).size() == 0);
    const std::array values{5, 7};
    const std::array edges{Edge{0, 0}, Edge{0, 1}, Edge{0, 1}};
    Colors colors(values, edges, 2);
    colors.assign_neighbors(0, 9);
    assert(colors.get(0) == 9 && colors.get(1) == 9);
    colors.assign(0, 12);
    assert(colors.get(0) == 12 && colors.get(1) == 9);
    const auto kMax = std::numeric_limits<std::uint64_t>::max();
    assert(sirkit::detail::next_neighborhood_assignment_stamp(kMax - 1) == kMax);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(sirkit::detail::next_neighborhood_assignment_stamp(kMax));
                                    });
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::detail::neighborhood_assignment_size(std::size_t{1} + std::numeric_limits<int>::max()));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    for (int invalid : {-1, 2})
    {
        const std::array bad{Edge{0, invalid}};
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(Colors(values, bad));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(colors.get(invalid));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          colors.assign_neighbors(invalid, 42);
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          colors.assign(invalid, 42);
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(219);
    for (int trial = 0; trial < 200; ++trial)
    {
        const int n = 1 + static_cast<int>(rng() % 32);
        const int m = static_cast<int>(rng() % (4 * n + 1));
        std::vector<Edge> edges;
        std::vector<std::vector<int>> graph(n);
        std::vector<int> expected(n);
        std::iota(expected.begin(), expected.end(), 0);
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(rng() % n);
            const int v = static_cast<int>(rng() % n);
            edges.emplace_back(u, v);
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        Colors colors(expected, edges, trial % 4 == 0 ? 0U : 1U + rng() % 21U);
        assert(colors.size() == n);
        for (int i = 0; i < 220; ++i)
        {
            const int v = static_cast<int>(rng() % n);
            const int value = static_cast<int>(rng() % 4000) - 2000;
            if (i % 3 == 0)
            {
                colors.assign(v, value);
                expected[v] = value;
            }
            else
            {
                const bool include_self = rng() % 2 == 0;
                const auto x = i % 5 == 0 ? colors.get(v) : value;
                const auto original = i % 5 == 0 ? expected[v] : value;
                colors.assign_neighbors(v, x, include_self);
                if (include_self)
                {
                    expected[v] = original;
                }
                for (const int to : graph[v])
                {
                    expected[to] = original;
                }
            }
            for (int v = 0; v < n; ++v)
            {
                assert(colors.get(v) == expected[v]);
            }
        }
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::detail::next_neighborhood_assignment_stamp(std::numeric_limits<std::uint64_t>::max()));
        return 1;
    }
    check_limits();
    check_labels();
    check_random();
}
