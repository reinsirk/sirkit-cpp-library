// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<int> degrees;
    degrees.reserve(static_cast<std::size_t>(n));
    // 1..n-1をすべて含み、中央の次数だけが重複する連結グラフを作る。
    for (int degree = 1; degree < n; ++degree)
    {
        degrees.push_back(degree);
        if (degree == n / 2)
        {
            degrees.push_back(degree);
        }
    }
    auto edges = sirkit::realize_degree_sequence(degrees).value();
    for (auto &[u, v] : edges)
    {
        if (u > v)
        {
            std::swap(u, v);
        }
    }
    std::sort(edges.begin(), edges.end());
    verify_out << edges.size() << '\n';
    for (const auto &[u, v] : edges)
    {
        verify_out << u + 1 << ' ' << v + 1 << '\n';
    }
}
