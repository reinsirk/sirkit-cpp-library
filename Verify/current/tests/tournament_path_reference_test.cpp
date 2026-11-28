// SPDX-License-Identifier: CC0-1.0
#include "Graph/Tournament_Path.cpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <stdexcept>
#include <vector>

int main()
{
    for (int n = 0; n <= 6; ++n)
    {
        const int pairs = n * (n - 1) / 2;
        for (unsigned mask = 0; mask < (1U << pairs); ++mask)
        {
            const auto edge = [mask](int u, int v)
            {
                const int high = std::max(u, v), low = std::min(u, v);
                const bool forward = ((mask >> (high * (high - 1) / 2 + low)) & 1U) != 0;
                return (u < v) == forward;
            };
            int queries = 0;
            auto path = sirkit::tournament_path(n, [&](int u, int v)
                                                {
                                                    assert(u != v && 0 <= u && u < n && 0 <= v && v < n);
                                                    ++queries;
                                                    return edge(u, v);
                                                });
            assert(path.size() == static_cast<std::size_t>(n));
            for (int i = 1; i < n; ++i)
            {
                assert(edge(path[i - 1], path[i]));
            }
            std::sort(path.begin(), path.end());
            for (int i = 0; i < n; ++i)
            {
                assert(path[i] == i);
            }
            if (n > 0)
            {
                const int levels = std::bit_width(static_cast<unsigned>(n - 1));
                assert(queries <= n * levels - (1 << levels) + 1);
            }
        }
    }

    // 右のパスから始めて交互に並べると、各併合で片側が尽きるのは最後の比較になる。
    constexpr int kMaxVertices = 500;
    const auto interleave = [](const auto &self, int left, int right) -> std::vector<int>
    {
        if (right - left == 1)
        {
            return {left};
        }
        const int middle = (left + right) / 2;
        const auto lower = self(self, left, middle);
        const auto upper = self(self, middle, right);
        std::vector<int> merged(right - left);
        for (int i = 0; i < right - left; ++i)
        {
            merged[i] = i % 2 == 0 ? upper[i / 2] : lower[i / 2];
        }
        return merged;
    };
    const auto order = interleave(interleave, 0, kMaxVertices);
    std::vector<int> rank(kMaxVertices);
    for (int i = 0; i < kMaxVertices; ++i)
    {
        rank[order[i]] = i;
    }
    int queries = 0;
    const auto worst = sirkit::tournament_path(kMaxVertices, [&](int u, int v)
                                               {
                                                   ++queries;
                                                   return rank[u] < rank[v];
                                               });
    assert(worst == order);
    assert(queries == 3989);
    try
    {
        (void)sirkit::tournament_path(-1, [](int, int)
                                      {
                                          return false;
                                      });
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
}
