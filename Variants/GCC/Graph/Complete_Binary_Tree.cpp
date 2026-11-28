// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Complete_Binary_Tree.cpp
// 参考（距離ごとの子孫の番号）: https://atcoder.jp/contests/abc321/editorial/7267
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/complete_binary_tree.md
//
// API一覧
// CompleteBinaryTree(n): 頂点番号1..n、親v/2の完全二分木。nはuint64_t。構築・領域O(1)。
// size()/depth(v): 頂点数、根からの深さ。
// kth_ancestor(v,k): k個上の祖先。根を越えれば0。
// lca(u,v)/distance(u,v): 最近共通祖先、2頂点間の辺数。
// kth_on_path(u,v,k): uを0番目としたu→v上の頂点。範囲外なら0。
// descendant_range(v,k): vからk段下の子孫の番号を両端込みの[first,last]で返す。空ならnullopt。
// count_at_distance(v,k): vから辺k本の距離にある頂点数。O(log(n+1))時間。
//   上記以外の照会はO(1)。k=0は自分自身を表す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMPLETE_BINARY_TREE_DEFINED
#define SIRKIT_COMPLETE_BINARY_TREE_DEFINED

class CompleteBinaryTree
{
public:
    explicit CompleteBinaryTree(std::uint64_t n) noexcept : n_(n)
    {
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] int depth(std::uint64_t vertex) const
    {
        check_vertex(vertex);
        return std::bit_width(vertex) - 1;
    }

    [[nodiscard]] std::uint64_t kth_ancestor(std::uint64_t vertex, std::uint64_t distance) const
    {
        check_vertex(vertex);
        return distance < kBits ? vertex >> distance : 0;
    }

    [[nodiscard]] std::uint64_t lca(std::uint64_t u, std::uint64_t v) const
    {
        const int difference = depth(u) - depth(v);
        u >>= std::max(0, difference);
        v >>= std::max(0, -difference);
        return u >> std::bit_width(u ^ v);
    }

    [[nodiscard]] int distance(std::uint64_t u, std::uint64_t v) const
    {
        const auto common = lca(u, v);
        return std::bit_width(u) + std::bit_width(v) - 2 * std::bit_width(common);
    }

    [[nodiscard]] std::uint64_t kth_on_path(std::uint64_t u, std::uint64_t v, std::uint64_t k) const
    {
        const auto common = lca(u, v);
        const std::uint64_t up = std::bit_width(u) - std::bit_width(common);
        const std::uint64_t down = std::bit_width(v) - std::bit_width(common);
        if (k <= up)
        {
            return u >> k;
        }
        return k <= up + down ? v >> (up + down - k) : 0;
    }

    [[nodiscard]] std::optional<std::pair<std::uint64_t, std::uint64_t>> descendant_range(
        std::uint64_t vertex, std::uint64_t steps) const
    {
        check_vertex(vertex);
        const auto count = count_descendants(vertex, steps);
        if (count == 0)
        {
            return std::nullopt;
        }
        const auto first = vertex << steps;
        return std::pair{first, first + (count - 1)};
    }

    [[nodiscard]] std::uint64_t count_at_distance(std::uint64_t vertex, std::uint64_t steps) const
    {
        check_vertex(vertex);
        const auto diameter_bound = 2 * (std::bit_width(n_) - 1);
        if (steps > static_cast<std::uint64_t>(diameter_bound))
        {
            return 0;
        }

        auto count = count_descendants(vertex, steps);
        // LCAを1段ずつ上げ、直前に通った枝の反対側だけを数える。
        while (vertex != 1 && steps != 0)
        {
            if (--steps == 0)
            {
                return count + 1;
            }
            count += count_descendants(vertex ^ 1, steps - 1);
            vertex >>= 1;
        }
        return count;
    }

private:
    static constexpr auto kBits = std::numeric_limits<std::uint64_t>::digits;

    void check_vertex(std::uint64_t vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex == 0 || vertex > n_)
        {
            throw std::out_of_range("complete binary tree: vertex out of range");
        }
#else
        (void)vertex;
#endif
    }

    [[nodiscard]] std::uint64_t count_descendants(std::uint64_t vertex, std::uint64_t steps) const noexcept
    {
        if (steps >= kBits || vertex > (n_ >> steps))
        {
            return 0;
        }
        return std::min(std::uint64_t{1} << steps, n_ - (vertex << steps) + 1);
    }

    std::uint64_t n_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
