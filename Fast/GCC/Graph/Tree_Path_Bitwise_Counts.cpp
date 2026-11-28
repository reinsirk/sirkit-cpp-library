// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Path_Bitwise_Counts.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_path_bitwise_counts.md
//
// API一覧
// TreePathBitwiseEdge{from,to,weight}: 0始まりの無向辺と uint64_t の重み。
// tree_path_bitwise_counts(n,edges): 森の同じ成分に属する相異なる頂点の順序なしペアを集計する。
//   TreePathBitwiseCounts の and_pairs[b]/or_pairs[b]/xor_pairs[b] = パスの演算結果の bit b が1の組数。
//   0<=b<64。各組数は uint64_t の正確値。重みに使うビット数 B で O((B+1)(n+m)) 時間、O(n+m) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_PATH_BITWISE_COUNTS_DEFINED
#define SIRKIT_TREE_PATH_BITWISE_COUNTS_DEFINED

struct TreePathBitwiseEdge
{
    int from;
    int to;
    std::uint64_t weight;
};

struct TreePathBitwiseCounts
{
    std::array<std::uint64_t, 64> and_pairs{};
    std::array<std::uint64_t, 64> or_pairs{};
    std::array<std::uint64_t, 64> xor_pairs{};
};

inline TreePathBitwiseCounts tree_path_bitwise_counts(int n, std::span<const TreePathBitwiseEdge> edges)
{
    struct Arc
    {
        int vertex;
        std::uint64_t weight;
    };
    std::vector<std::vector<Arc>> graph(n);
    std::uint64_t used_bits = 0;
    for (const auto &[from, to, weight] : edges)
    {
        graph[from].push_back({to, weight});
        graph[to].push_back({from, weight});
        used_bits |= weight;
    }

    std::vector<int> parent(n, -1), order;
    order.reserve(static_cast<std::size_t>(n));
    std::vector<std::uint64_t> weight_to_parent(n), prefix_xor(n);
    std::vector<std::pair<std::size_t, std::size_t>> components;
    std::uint64_t total_pairs = 0;
    for (int root = 0; root < n; ++root)
    {
        if (parent[root] != -1)
        {
            continue;
        }
        parent[root] = root;
        const auto first = order.size();
        order.push_back(root);
        for (std::size_t i = first; i < order.size(); ++i)
        {
            const int u = order[i];
            for (const auto &[v, weight] : graph[u])
            {
                if (v == parent[u])
                {
                    continue;
                }
                parent[v] = u;
                weight_to_parent[v] = weight;
                prefix_xor[v] = prefix_xor[u] ^ weight;
                order.push_back(v);
            }
        }
        const std::uint64_t count = order.size() - first;
        total_pairs += count * (count - 1) / 2;
        components.emplace_back(first, order.size());
    }

    TreePathBitwiseCounts result;
    std::vector<std::array<std::uint64_t, 2>> joined(n);
    for (; used_bits != 0; used_bits &= used_bits - 1)
    {
        const auto bit = std::countr_zero(used_bits);
        const auto mask = std::uint64_t{1} << bit;
        for (auto &sizes : joined)
        {
            sizes = {1, 1};
        }
        std::array<std::uint64_t, 2> filtered_pairs{};
        // 同じビットの辺だけを残した森で連結なペアを数える。
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            const int u = *it;
            const int p = parent[u];
            if (p == u)
            {
                continue;
            }
            const int color = (weight_to_parent[u] & mask) != 0;
            filtered_pairs[color] += joined[p][color] * joined[u][color];
            joined[p][color] += joined[u][color];
        }
        result.and_pairs[bit] = filtered_pairs[1];
        result.or_pairs[bit] = total_pairs - filtered_pairs[0];

        // パス XOR は、同じ成分の根から両端までの XOR 同士で求まる。
        for (const auto &[first, last] : components)
        {
            std::uint64_t ones = 0;
            for (std::size_t i = first; i < last; ++i)
            {
                ones += (prefix_xor[order[i]] & mask) != 0;
            }
            result.xor_pairs[bit] += ones * (last - first - ones);
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
