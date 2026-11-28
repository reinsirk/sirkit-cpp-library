// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Tree_Permutation_Distance.cpp
// 参考（重心ごとの包除）: https://img.atcoder.jp/abc082/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_permutation_distance.md
//
// API一覧
// TreePermutationEdge{from,to,weight=1}: 0始まり正重み無向木の辺。
// TreePermutationDistance(n,edges): Σ dist(v,p[v]) を最大にする順列問題を準備。O(n)時間・領域。
// maximum(): 最大の総距離。uint64_tの上限を超えた場合はnullopt。O(1)。
// permutation(): 最大値を達成する順列pを1つ返す。O(n)。
// optimal_count(modulus=1000000007): 最大値を達成する順列数をmodulusで割った余り。modulus>0。
//   包除原理でO(n^2)時間・O(n)追加領域。空木の順列は1通り。
//   各辺を通る割当は小さい側の頂点数の2倍が上限。重心の各枝の中だけで完結する割当を除けば同時に達成する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_PERMUTATION_DISTANCE_DEFINED
#define SIRKIT_TREE_PERMUTATION_DISTANCE_DEFINED

struct TreePermutationEdge
{
    int from;
    int to;
    std::uint64_t weight = 1;
};

class TreePermutationDistance
{
public:
    TreePermutationDistance(int n, std::span<const TreePermutationEdge> edges)
    {
        if (n < 0 || edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1))
        {
            throw std::length_error("TreePermutationDistance: invalid tree size");
        }
        if (n == 0)
        {
            return;
        }
        using Arc = std::pair<int, std::uint64_t>;
        std::vector<std::vector<Arc>> graph(n);
        for (const auto &[u, v, weight] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (static_cast<std::size_t>(u) >= graph.size() || static_cast<std::size_t>(v) >= graph.size())
            {
                throw std::out_of_range("TreePermutationDistance: invalid vertex");
            }
#endif
            if (weight == 0)
            {
                throw std::invalid_argument("TreePermutationDistance: edge weight must be positive");
            }
            graph[u].emplace_back(v, weight);
            graph[v].emplace_back(u, weight);
        }
        constexpr int kUnseen = -2;
        std::vector<int> parent(n, kUnseen), traversal{0};
        std::vector<std::uint64_t> parent_weight(n);
        parent[0] = -1;
        for (std::size_t i = 0; i < traversal.size(); ++i)
        {
            const int u = traversal[i];
            for (const auto &[v, weight] : graph[u])
            {
                if (v == parent[u])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (parent[v] != kUnseen)
                {
                    throw std::invalid_argument("TreePermutationDistance: cycle");
                }
#endif
                parent[v] = u;
                parent_weight[v] = weight;
                traversal.push_back(v);
            }
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (traversal.size() != graph.size())
        {
            throw std::invalid_argument("TreePermutationDistance: disconnected");
        }
#endif
        std::vector<int> subtree(n, 1);
        int centroid = -1;
        for (const int v : std::views::reverse(traversal))
        {
            const int size = subtree[v];
            if (centroid == -1 && size > n / 2)
            {
                centroid = v;
            }
            if (parent[v] >= 0)
            {
                subtree[parent[v]] += size;
                const auto crossings = std::uint64_t{2} * std::min(size, n - size);
                if (maximum_)
                {
                    if (parent_weight[v] > (std::numeric_limits<std::uint64_t>::max() - *maximum_) / crossings)
                    {
                        maximum_.reset();
                    }
                    else
                    {
                        *maximum_ += crossings * parent_weight[v];
                    }
                }
            }
        }

        vertices_.reserve(n);
        vertices_.push_back(centroid);
        std::vector<std::pair<int, int>> pending;
        for (const auto &[first, weight] : graph[centroid])
        {
            const auto begin = vertices_.size();
            pending.emplace_back(first, centroid);
            while (!pending.empty())
            {
                const auto [v, previous] = pending.back();
                pending.pop_back();
                vertices_.push_back(v);
                for (const auto &[u, edge_weight] : graph[v])
                {
                    if (u != previous)
                    {
                        pending.emplace_back(u, v);
                    }
                }
            }
            branches_.push_back(vertices_.size() - begin);
        }
    }

    [[nodiscard]] std::optional<std::uint64_t> maximum() const
    {
        return maximum_;
    }

    [[nodiscard]] std::vector<int> permutation() const
    {
        const auto n = vertices_.size();
        std::vector<int> result(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            result[vertices_[i]] = vertices_[(i + n / 2) % n];
        }
        return result;
    }

    [[nodiscard]] std::uint32_t optimal_count(std::uint32_t modulus = 1'000'000'007) const
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("TreePermutationDistance: zero modulus");
        }
        const auto add = [modulus](std::uint32_t u, std::uint32_t v)
        {
            const auto sum = std::uint64_t{u} + v;
            return static_cast<std::uint32_t>(sum >= modulus ? sum - modulus : sum);
        };
        const auto multiply = [modulus](std::uint32_t u, std::uint32_t v)
        {
            return static_cast<std::uint32_t>(std::uint64_t{u} * v % modulus);
        };

        const auto n = vertices_.size();
        std::vector<std::uint32_t> factorial(n + 1);
        factorial[0] = 1U % modulus;
        for (std::size_t i = 1; i <= n; ++i)
        {
            factorial[i] = multiply(factorial[i - 1], static_cast<std::uint32_t>(i % modulus));
        }
        for (const auto size : branches_)
        {
            if (size * 2 == n)
            {
                return multiply(factorial[size], factorial[size]);
            }
        }

        std::vector<std::uint32_t> ways{factorial[0]};
        for (const auto size : branches_)
        {
            std::vector<std::uint32_t> choose(size + 1);
            choose[0] = factorial[0];
            for (std::size_t row = 1; row <= size; ++row)
            {
                for (std::size_t j = row; j != 0; --j)
                {
                    choose[j] = add(choose[j], choose[j - 1]);
                }
            }
            std::vector<std::uint32_t> rook(size + 1);
            for (std::size_t j = 0; j <= size; ++j)
            {
                rook[j] = multiply(multiply(choose[j], choose[j]), factorial[j]);
            }

            std::vector<std::uint32_t> next(ways.size() + size);
            for (std::size_t i = 0; i < ways.size(); ++i)
            {
                for (std::size_t j = 0; j <= size; ++j)
                {
                    next[i + j] = add(next[i + j], multiply(ways[i], rook[j]));
                }
            }
            ways.swap(next);
        }
        std::uint32_t answer = 0;
        for (std::size_t i = 0; i < ways.size(); ++i)
        {
            auto value = multiply(ways[i], factorial[n - i]);
            if ((i & 1U) != 0 && value != 0)
            {
                value = modulus - value;
            }
            answer = add(answer, value);
        }
        return answer;
    }

private:
    std::optional<std::uint64_t> maximum_ = 0;
    std::vector<int> vertices_;
    std::vector<std::size_t> branches_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
