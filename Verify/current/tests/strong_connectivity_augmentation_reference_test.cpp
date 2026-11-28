// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Strong_Connectivity_Augmentation.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Arc = std::pair<int, int>;

std::vector<std::vector<bool>> closure(int n, const std::vector<Arc> &arcs)
{
    std::vector reach(n, std::vector<bool>(n));
    for (int v = 0; v < n; ++v)
    {
        reach[v][v] = true;
    }
    for (const auto &[u, v] : arcs)
    {
        reach[u][v] = true;
    }
    for (int limit = 0; limit < n; ++limit)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                reach[u][v] = reach[u][v] || (reach[u][limit] && reach[limit][v]);
            }
        }
    }
    return reach;
}

std::size_t expected_count(int n, const std::vector<Arc> &arcs)
{
    const auto reach = closure(n, arcs);
    std::vector<int> id(n);
    int components = 0;
    for (int v = 0; v < n; ++v)
    {
        id[v] = v;
        for (int u = 0; u < v; ++u)
        {
            if (reach[u][v] && reach[v][u])
            {
                id[v] = u;
                break;
            }
        }
        components += id[v] == v;
    }
    if (components <= 1)
    {
        return 0;
    }
    std::vector<bool> has_in(n), has_out(n);
    for (const auto &[u, v] : arcs)
    {
        if (id[u] != id[v])
        {
            has_out[id[u]] = true;
            has_in[id[v]] = true;
        }
    }
    std::size_t sources = 0, sinks = 0;
    for (int v = 0; v < n; ++v)
    {
        if (id[v] == v)
        {
            sources += !has_in[v];
            sinks += !has_out[v];
        }
    }
    return std::max(sources, sinks);
}

void check(int n, const std::vector<Arc> &arcs)
{
    const auto added = sirkit::strong_connectivity_augmentation(n, arcs);
    assert(added.size() == expected_count(n, arcs));
    std::set<Arc> unique(arcs.begin(), arcs.end());
    for (const auto &[u, v] : added)
    {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        assert(unique.emplace(u, v).second);
    }
    std::vector<Arc> augmented = arcs;
    augmented.insert(augmented.end(), added.begin(), added.end());
    const auto reachable = closure(n, augmented);
    for (const auto &row : reachable)
    {
        assert(std::ranges::all_of(row, [](bool value)
                                   {
                                       return value;
                                   }));
    }
}

template <class Error, class Action>
void expect_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check(0, {});
    check(1, {{0, 0}});
    check(4, {});
    check(5, {{0, 4}, {1, 4}, {2, 4}, {3, 4}});
    check(5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}});
    check(7, {{0, 3}, {0, 2}, {2, 5}, {3, 4}, {4, 6}, {1, 4}});
    check(6, {{0, 1}, {1, 0}, {0, 2}, {1, 2}, {0, 2}, {4, 2}, {2, 3}, {4, 4}});
    check(6, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 0}});
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::strong_connectivity_augmentation(-1, {});
                                    });
    for (const auto &[u, v] : {Arc{-1, 0}, Arc{2, 0}, Arc{0, -1}, Arc{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array bad{Arc{u, v}};
                                            (void)sirkit::strong_connectivity_augmentation(2, bad);
                                        });
    }

    std::vector<Arc> all;
    for (int u = 0; u < 4; ++u)
    {
        for (int v = 0; v < 4; ++v)
        {
            if (u != v)
            {
                all.emplace_back(u, v);
            }
        }
    }
    for (std::size_t mask = 0; mask < (std::size_t{1} << all.size()); ++mask)
    {
        std::vector<Arc> arcs;
        for (std::size_t i = 0; i < all.size(); ++i)
        {
            if ((mask >> i) & 1)
            {
                arcs.push_back(all[i]);
            }
        }
        check(4, arcs);
    }
    std::mt19937 random(366728299);
    for (int trial = 0; trial < 720; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 24);
        const int m = static_cast<int>(random() % (n * 4 + 1));
        std::vector<Arc> arcs;
        for (int i = 0; i < m; ++i)
        {
            arcs.emplace_back(static_cast<int>(random() % n), static_cast<int>(random() % n));
        }
        check(n, arcs);
    }
}
