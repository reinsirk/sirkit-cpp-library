// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace
{
struct HashOperations
{
    using Vertex = std::uint64_t;
    using Point = std::uint64_t;
    struct Path
    {
        std::uint64_t mul;
        std::uint64_t add;
    };
    static constexpr std::uint64_t kMod = 998244353;

    Path vertex(Vertex value, const Point *branches) const noexcept
    {
        return {branches ? *branches : 1, value};
    }

    Point project(const Path &path) const noexcept
    {
        return path.add;
    }

    Point rake(Point left, Point right) const noexcept
    {
        return left * right % kMod;
    }

    Path compress(const Path &upper, const Path &lower) const noexcept
    {
        return {upper.mul * lower.mul % kMod, (upper.mul * lower.add + upper.add) % kMod};
    }
};
}

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<std::pair<int, int>> edges;
    edges.reserve(n - 1);
    for (int v = 1; v < n; ++v)
    {
        int parent{};
        verify_in >> parent;
        edges.emplace_back(parent - 1, v);
    }
    std::vector<std::uint64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::StaticTopTree<HashOperations> tree(values, edges);
    for (int i = 0; i < q; ++i)
    {
        int v{};
        std::uint64_t value{};
        verify_in >> v >> value;
        tree.set(v - 1, value);
        verify_out << tree.all_prod().add << '\n';
    }
}
