// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Offline_Distinct_Count.cpp
// 参考（最終出現位置とFenwick Tree）: https://img.atcoder.jp/abc174/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_distinct_count.md
//
// API一覧
// DistinctCountQuery{left,right}: 0始まりの半開区間[left,right)。0<=left<=right<=N。
// offline_distinct_count(values,queries): 区間内の値の種類数をクエリ順に返す。値は符号付き64ビット全域。
// 空区間は0。O((N+Q) log(N+2))時間・O(N+Q)領域。検査有効時、無効な区間はstd::out_of_range。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_DISTINCT_COUNT_DEFINED
#define SIRKIT_OFFLINE_DISTINCT_COUNT_DEFINED

struct DistinctCountQuery
{
    std::size_t left;
    std::size_t right;
};

inline std::vector<std::size_t> offline_distinct_count(std::span<const std::int64_t> values,
                                                       std::span<const DistinctCountQuery> queries)
{
    std::size_t n = 0;
    for (const auto &[left, right] : queries)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > values.size())
        {
            throw std::out_of_range("offline_distinct_count: query range");
        }
#endif
        if (left != right)
        {
            n = std::max(n, right);
        }
    }
    const auto no_query = queries.size();
    std::vector<std::size_t> answer(no_query);
    if (n == 0)
    {
        return answer;
    }

    std::vector<std::size_t> head(n, no_query), next_query(no_query);
    for (std::size_t id = 0; id < no_query; ++id)
    {
        const auto &[left, right] = queries[id];
        if (left != right)
        {
            next_query[id] = std::exchange(head[right - 1], id);
        }
    }

    std::vector<std::size_t> previous(n, n);
    {
        // 同じ値を元の位置順に並べると、1つ前の出現が隣に来る。
        std::vector<std::pair<std::int64_t, std::size_t>> ordered(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            ordered[i] = {values[i], i};
        }
        std::sort(ordered.begin(), ordered.end());
        for (std::size_t i = 1; i < n; ++i)
        {
            if (ordered[i - 1].first == ordered[i].first)
            {
                previous[ordered[i].second] = ordered[i - 1].second;
            }
        }
    }

    // 各値の最新位置だけに1を置く、0始まりのFenwick Tree。
    std::vector<std::size_t> tree(n);
    std::size_t total = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        if (previous[i] == n)
        {
            ++total;
        }
        for (auto old = previous[i]; old < n; old |= old + 1)
        {
            --tree[old];
        }
        for (auto current = i; current < n; current |= current + 1)
        {
            ++tree[current];
        }
        for (auto id = head[i]; id != no_query; id = next_query[id])
        {
            std::size_t outside = 0;
            for (auto left = queries[id].left; left != 0; left &= left - 1)
            {
                outside += tree[left - 1];
            }
            answer[id] = total - outside;
        }
    }
    return answer;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
