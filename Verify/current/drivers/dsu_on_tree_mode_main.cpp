// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, given{}, palette{};
    std::uint32_t state{};
    verify_in >> n >> state >> given >> palette;
    auto advance = [&]
    {
        state = (state * 1103515245u + 12345u) & 0x7fffffffu;
    };
    std::vector<int> parent(n, -1), color(n);
    for (int v = 1; v < n; ++v)
    {
        if (v < given)
        {
            verify_in >> parent[v];
            --parent[v];
        }
        else
        {
            parent[v] = state % v;
            advance();
        }
    }
    for (int v = 0; v < n; ++v)
    {
        if (v < given)
        {
            verify_in >> color[v];
            --color[v];
        }
        else
        {
            color[v] = state % palette;
            advance();
        }
    }
    const sirkit::DsuOnTree tree(parent);
    std::vector<int> frequency(n), buckets(n + 1);
    int maximum = 0;
    auto add = [&](int v)
    {
        int &f = frequency[color[v]];
        if (f != 0)
        {
            --buckets[f];
        }
        ++buckets[++f];
        maximum = std::max(maximum, f);
    };
    auto remove = [&](int v)
    {
        int &f = frequency[color[v]];
        const int old = f--;
        --buckets[old];
        if (f != 0)
        {
            ++buckets[f];
        }
        if (old == maximum && buckets[old] == 0)
        {
            --maximum;
        }
    };
    constexpr std::uint64_t kMod = 998244353;
    std::uint64_t sum = 0;
    tree.run(add, remove, [&](int v)
    {
        const int vertex = v + 1;
        sum = (sum + static_cast<std::uint64_t>(maximum ^ vertex) * (buckets[maximum] ^ vertex)) % kMod;
    });
    verify_out << sum << '\n';
}
