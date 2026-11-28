// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Complete_Binary_Tree.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

namespace
{
unsigned climb(unsigned vertex, unsigned steps)
{
    for (unsigned i = 0; i < steps; ++i)
    {
        vertex /= 2;
    }
    return vertex;
}

void check(unsigned n)
{
    const sirkit::CompleteBinaryTree tree(n);
    assert(tree.size() == n);
    std::vector<std::vector<unsigned>> adjacent(n + 1);
    for (unsigned u = 2; u <= n; ++u)
    {
        adjacent[u].push_back(u / 2);
        adjacent[u / 2].push_back(u);
    }
    for (unsigned start = 1; start <= n; ++start)
    {
        std::vector<unsigned> pending{start}, previous(n + 1);
        previous[start] = start;
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            for (const unsigned next : adjacent[pending[i]])
            {
                if (previous[next] == 0)
                {
                    previous[next] = pending[i];
                    pending.push_back(next);
                }
            }
        }
        std::vector<unsigned> counts(n + 1);
        for (unsigned v = 1; v <= n; ++v)
        {
            std::vector<unsigned> path{v};
            while (path.back() != start)
            {
                path.push_back(previous[path.back()]);
            }
            std::ranges::reverse(path);
            const auto edges = path.size() - 1;
            ++counts[edges];
            assert(tree.distance(start, v) == static_cast<int>(edges));
            for (std::size_t k = 0; k < path.size(); ++k)
            {
                assert(tree.kth_on_path(start, v, k) == path[k]);
            }
            assert(tree.kth_on_path(start, v, path.size()) == 0);
            assert(tree.lca(start, v) == *std::ranges::min_element(path));
        }

        unsigned depth = 0;
        while (climb(start, depth) != 1)
        {
            ++depth;
        }
        assert(tree.depth(start) == static_cast<int>(depth));
        for (unsigned k = 0; k <= n; ++k)
        {
            assert(tree.count_at_distance(start, k) == counts[k]);
            assert(tree.kth_ancestor(start, k) == climb(start, k));
            std::vector<unsigned> descendants;
            for (unsigned v = 1; v <= n; ++v)
            {
                if (climb(v, k) == start)
                {
                    descendants.push_back(v);
                }
            }
            const auto actual = tree.descendant_range(start, k);
            assert(actual.has_value() == !descendants.empty());
            if (actual)
            {
                assert(actual->first == descendants.front());
                assert(actual->second == descendants.back());
                assert(actual->second - actual->first + 1 == descendants.size());
            }
        }
    }
}
} // namespace

int main()
{
    for (unsigned n = 0; n <= 48; ++n)
    {
        check(n);
    }
    constexpr auto kHalf = std::uint64_t{1} << 63;
    constexpr auto kMaximum = std::numeric_limits<std::uint64_t>::max();
    const sirkit::CompleteBinaryTree full(kMaximum), partial(kHalf);
    assert((full.descendant_range(1, 63) == std::pair{kHalf, kMaximum}));
    assert(full.count_at_distance(1, 63) == kHalf);
    assert(full.count_at_distance(kHalf, 126) == kHalf / 2);
    assert(full.count_at_distance(kHalf, 64) > 0);
    assert(full.kth_ancestor(kMaximum, 63) == 1 && full.kth_ancestor(kMaximum, 64) == 0);
    assert(full.lca(kHalf, kMaximum) == 1 && full.distance(kHalf, kMaximum) == 126);
    assert(full.kth_on_path(kHalf, kMaximum, 126) == kMaximum);
    assert(!full.descendant_range(1, 64) && !full.descendant_range(2, 63));
    assert((partial.descendant_range(1, 63) == std::pair{kHalf, kHalf}));
    assert(!partial.descendant_range(1, kMaximum));
    assert(partial.count_at_distance(kHalf, 126) == 0);
    assert(partial.count_at_distance(kHalf, 125) == kHalf / 4);

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    for (const auto invalid : std::array<std::uint64_t, 2>{0, kHalf + 1})
    {
        try
        {
            (void)partial.depth(invalid);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    assert(rejected == 2);
#endif
}
