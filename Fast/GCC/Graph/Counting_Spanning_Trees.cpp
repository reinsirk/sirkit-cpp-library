// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Counting_Spanning_Trees.cpp
// 参考（行列木定理）: https://doi.org/10.1137/0603033
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/counting_spanning_trees.md
//
// API一覧
// SpanningTreeCountEdge<Mint>: 辺{from,to,weight}。weightの既定値は1。
// count_undirected_spanning_trees<Mint>(n,edges): 無向全域木の重みの積和。
// count_directed_spanning_trees<Mint>(n,root,edges): 根から全頂点に到達する有向全域木の重みの積和。
// 頂点数は1以上、番号は0始まり。多重辺は区別し、自己辺は数えない。1頂点の空の木は1通り。
// Mintは素数mod型等の正確な体。係数演算をO(1)として、O(n³+m)時間・O(n²)領域。
// count_eulerian_circuits<Mint>(n,edges): 巡回シフトを同一視して有向オイラー閉路を数える。
// 自己辺と平行辺は番号で区別し、孤立頂点は無視する。辺0本では1、存在しなければ0。
// 辺に接する頂点数kとしてO(n+m+k³)時間・O(n+m+k²)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNTING_SPANNING_TREES_DEFINED
#define SIRKIT_COUNTING_SPANNING_TREES_DEFINED

template <class Mint>
struct SpanningTreeCountEdge
{
    int from = 0;
    int to = 0;
    Mint weight{1};
};

namespace detail
{
template <class Mint>
Mint spanning_tree_minor_determinant(std::vector<Mint> &matrix, std::size_t side)
{
    const Mint zero{0}, one{1};
    Mint determinant = one;
    for (std::size_t column = 0; column < side; ++column)
    {
        std::size_t pivot = column;
        while (pivot < side && matrix[pivot * side + column] == zero)
        {
            ++pivot;
        }
        if (pivot == side)
        {
            return zero;
        }
        auto *pivot_row = matrix.data() + column * side;
        if (pivot != column)
        {
            std::swap_ranges(pivot_row + column, pivot_row + side, matrix.data() + pivot * side + column);
            determinant = zero - determinant;
        }
        const auto diagonal = pivot_row[column];
        determinant *= diagonal;
        const auto inverse = one / diagonal;
        for (std::size_t r = column + 1; r < side; ++r)
        {
            auto *row = matrix.data() + r * side;
            if (row[column] == zero)
            {
                continue;
            }
            const auto factor = row[column] * inverse;
            for (std::size_t c = column + 1; c < side; ++c)
            {
                row[c] -= factor * pivot_row[c];
            }
        }
    }
    return determinant;
}

template <class Mint>
Mint count_spanning_trees_minor(
    int n, int root, std::span<const SpanningTreeCountEdge<Mint>> edges, bool directed)
{
    static_assert(!std::is_arithmetic_v<Mint>, "spanning tree counts require an exact field type");
    if (n <= 0)
    {
        throw std::invalid_argument("count_spanning_trees: nonpositive vertex count");
    }
    const auto side = static_cast<std::size_t>(n - 1);
    constexpr auto kMaxEntries = static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(Mint);
    if (side != 0 && side > kMaxEntries / side)
    {
        throw std::length_error("count_spanning_trees: matrix too large");
    }
    std::vector<Mint> matrix(side * side, Mint{0});
    const auto index = [root](int vertex)
    {
        return static_cast<std::size_t>(vertex - static_cast<int>(vertex > root));
    };
    const auto add_arc = [&](int from, int to, const Mint &weight)
    {
        if (to != root)
        {
            const auto row = index(to);
            matrix[row * side + row] += weight;
            if (from != root)
            {
                matrix[row * side + index(from)] -= weight;
            }
        }
    };
    for (const auto &[from, to, weight] : edges)
    {
        if (from == to)
        {
            continue;
        }
        // 着点の入次数を対角に置き、根から外へ向かう木を数える。
        add_arc(from, to, weight);
        if (!directed)
        {
            add_arc(to, from, weight);
        }
    }
    return spanning_tree_minor_determinant(matrix, side);
}
} // namespace detail

template <class Mint>
Mint count_undirected_spanning_trees(int n, std::span<const SpanningTreeCountEdge<Mint>> edges)
{
    return detail::count_spanning_trees_minor(n, 0, edges, false);
}

template <class Mint>
Mint count_directed_spanning_trees(int n, int root, std::span<const SpanningTreeCountEdge<Mint>> edges)
{
    return detail::count_spanning_trees_minor(n, root, edges, true);
}

template <class Mint>
Mint count_eulerian_circuits(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::invalid_argument("count_eulerian_circuits: negative vertex count");
    }
    if (edges.empty())
    {
        return Mint{1};
    }
    std::vector<std::size_t> out(n), in(n);
    for (const auto &[from, to] : edges)
    {
        ++out[from];
        ++in[to];
    }
    int active = 0;
    std::vector<int> id(n, -1);
    Mint arrangements{1};
    for (int i = 0; i < n; ++i)
    {
        if (out[i] != in[i])
        {
            return Mint{0};
        }
        if (out[i] != 0)
        {
            id[i] = active++;
            for (std::size_t k = 1; k < out[i]; ++k)
            {
                arrangements *= Mint{k};
            }
        }
    }
    std::vector<SpanningTreeCountEdge<Mint>> reversed;
    reversed.reserve(edges.size());
    for (const auto &[from, to] : edges)
    {
        reversed.push_back({id[to], id[from]});
    }
    // BEST定理の根向き全域木は、逆向きにしたグラフの根から外向きの全域木として数える。
    return arrangements * count_directed_spanning_trees<Mint>(active, id[edges.front().first], reversed);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
