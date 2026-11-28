// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Tree_Subset_Medians.cpp
// 参考（部分集合からの距離和を最小にする頂点を数えるVerify）: https://yukicoder.me/problems/no/3834
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_subset_medians.md
//
// API一覧
// TreeSubsetMedians(n, edges, eligible={}): 0始まりの無向木。eligibleはn要素の0/非0マスク、空なら全頂点を選べる。
// size()/eligible_count(): 木の頂点数/選んでよい頂点数e。O(1)。nはINT_MAX以下、空木も可。
// cut_histogram()[s]: 各辺を切ったとき、二側のうち少ない方に選べる頂点がs個ある辺の数。O(1)でconst参照。
// median_count_sum(k, choose): 選べる頂点からk個選ぶ全場合について、選んだ頂点への距離和が最小の頂点数を足す。
// choose(n,k)は組合せ数を返す関数。返り値型で加算、乗算、整数からの変換を行う。k>eなら0、k=0ならn。
// 構築O(1+n)時間・領域。median_count_sumはchooseの計算をO(1)とすると偶数kでO(1+e-k)、奇数kでO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_SUBSET_MEDIANS
#define SIRKIT_TREE_SUBSET_MEDIANS

class TreeSubsetMedians
{
public:
    TreeSubsetMedians(std::size_t vertices, std::span<const std::pair<int, int>> edges,
                      std::span<const std::uint8_t> eligible = {})
        : vertices_(vertices)
    {
        if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("TreeSubsetMedians: too many vertices");
        }
        if (edges.size() != (vertices == 0 ? 0 : vertices - 1) || (!eligible.empty() && eligible.size() != vertices))
        {
            throw std::invalid_argument("TreeSubsetMedians: edge count or eligibility size mismatch");
        }
        if (vertices == 0)
        {
            return;
        }
        std::vector<std::vector<int>> adjacency(vertices);
        for (const auto &[a, b] : edges)
        {
            if (static_cast<std::size_t>(a) >= vertices || static_cast<std::size_t>(b) >= vertices)
            {
                throw std::out_of_range("TreeSubsetMedians: endpoint out of range");
            }
            adjacency[a].push_back(b);
            adjacency[b].push_back(a);
        }
        std::vector<int> parent(vertices, -1), order;
        order.reserve(vertices);
        parent[0] = 0;
        order.push_back(0);
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const auto at = order[i];
            for (const auto to : adjacency[at])
            {
                if (parent[to] == -1)
                {
                    parent[to] = at;
                    order.push_back(to);
                }
            }
        }
        if (order.size() != vertices)
        {
            throw std::invalid_argument("TreeSubsetMedians: disconnected tree");
        }
        std::vector<std::size_t> subtree(vertices);
        for (std::size_t i = 0; i < vertices; ++i)
        {
            subtree[i] = eligible.empty() || eligible[i] != 0;
        }
        eligible_count_ = std::accumulate(subtree.begin(), subtree.end(), std::size_t{0});
        histogram_.resize(eligible_count_ / 2 + 1);
        for (std::size_t i = vertices; i > 1; --i)
        {
            const auto child = order[i - 1];
            const auto selected = subtree[child];
            ++histogram_[std::min(selected, eligible_count_ - selected)];
            subtree[parent[child]] += selected;
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return vertices_;
    }

    [[nodiscard]] std::size_t eligible_count() const noexcept
    {
        return eligible_count_;
    }

    [[nodiscard]] const std::vector<std::size_t> &cut_histogram() const noexcept
    {
        return histogram_;
    }

    template <class Choose>
    [[nodiscard]] auto median_count_sum(std::size_t selected, Choose &&choose) const
    {
        using Count = std::remove_cvref_t<decltype(choose(eligible_count_, selected))>;
        if (selected > eligible_count_)
        {
            return Count{};
        }
        if (selected == 0)
        {
            return Count(vertices_);
        }
        auto total = choose(eligible_count_, selected);
        if (selected % 2 != 0)
        {
            return total;
        }
        const auto half = selected / 2;
        // 最小値を取る頂点はパスをなし、両側にk/2個ずつ選ぶ辺がちょうどそのパスの辺になる。
        for (auto cut = half; cut < histogram_.size(); ++cut)
        {
            if (histogram_[cut] != 0)
            {
                total += choose(cut, half) * choose(eligible_count_ - cut, half) * Count(histogram_[cut]);
            }
        }
        return total;
    }

private:
    std::size_t vertices_, eligible_count_ = 0;
    std::vector<std::size_t> histogram_{0};
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
