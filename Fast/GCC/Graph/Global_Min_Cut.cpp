// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Global_Min_Cut.cpp
// 参考: Stoer & Wagner, A Simple Min-Cut Algorithm, https://doi.org/10.1145/263867.263872
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/global_min_cut.md
//
// API一覧
// GlobalMinCutEdge {u,v,weight=1}: 無向辺と非負のuint64_t重み。
// GlobalMinCutResult {weight,side}: 最小カットの重みと片側の頂点一覧（順不同）。
// global_min_cut(n,edges): 全頂点を二つの空でない集合に分けるときの最小カット。n<2ではnullopt。
// 平行辺は加算し、自己辺は無視する。自己辺以外の全重みの合計はuint64_tに収める。
// Stoer–Wagner法。n頂点m辺でO(n³+m)時間・O(n²)領域。入力は変更しない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GLOBAL_MIN_CUT_DEFINED
#define SIRKIT_GLOBAL_MIN_CUT_DEFINED
struct GlobalMinCutEdge
{
    int u = 0;
    int v = 0;
    std::uint64_t weight = 1;
};

struct GlobalMinCutResult
{
    std::uint64_t weight = 0;
    std::vector<int> side;
};

namespace detail
{
inline std::size_t GlobalCutMatrixSize(std::size_t n, std::size_t max_cells)
{
    if (n != 0 && n > max_cells / n)
    {
        throw std::length_error("global_min_cut: graph exceeds matrix capacity");
    }
    return n * n;
}
}

inline std::optional<GlobalMinCutResult> global_min_cut(int n, std::span<const GlobalMinCutEdge> edges)
{
    std::vector<std::uint64_t> matrix;
    matrix.resize(detail::GlobalCutMatrixSize(n, matrix.max_size()));
    const auto row = [&](int vertex)
    {
        return matrix.data() + static_cast<std::size_t>(vertex) * n;
    };
    std::uint64_t total = 0;
    for (const auto &[u, v, weight] : edges)
    {
        if (u == v)
        {
            continue;
        }
        if (weight > std::numeric_limits<std::uint64_t>::max() - total)
        {
            throw std::overflow_error("global_min_cut: total edge weight does not fit");
        }
        total += weight;
        row(u)[v] += weight;
        row(v)[u] += weight;
    }
    if (n < 2)
    {
        return std::nullopt;
    }

    std::vector<int> active(n), tail(n), next(n, -1);
    std::iota(active.begin(), active.end(), 0);
    std::iota(tail.begin(), tail.end(), 0);
    std::vector<std::uint64_t> adjacent_weight(n);
    std::optional<GlobalMinCutResult> best;
    while (active.size() >= 2)
    {
        std::fill(adjacent_weight.begin(), adjacent_weight.end(), std::uint64_t{0});
        // すでに選んだ頂点への重み和が最大の頂点を選ぶ。
        for (std::size_t i = 0; i + 1 < active.size(); ++i)
        {
            auto max_index = i;
            for (auto j = i + 1; j < active.size(); ++j)
            {
                if (adjacent_weight[active[j]] > adjacent_weight[active[max_index]])
                {
                    max_index = j;
                }
            }
            std::swap(active[i], active[max_index]);
            const auto *from = row(active[i]);
            for (auto j = i + 1; j < active.size(); ++j)
            {
                const auto to = active[j];
                adjacent_weight[to] += from[to];
            }
        }
        const auto last = active.back();
        active.pop_back();
        const auto previous = active.back();
        const auto cut = adjacent_weight[last];
        if (!best || cut < best->weight)
        {
            best.emplace();
            best->weight = cut;
            for (int v = last; v >= 0; v = next[v])
            {
                best->side.push_back(v);
            }
            if (cut == 0)
            {
                return best;
            }
        }

        // 最後の2頂点を縮約し、元の頂点を連結リストで追跡する。
        const auto *removed = row(last);
        auto *merged = row(previous);
        for (std::size_t i = 0; i + 1 < active.size(); ++i)
        {
            const auto v = active[i];
            merged[v] += removed[v];
            row(v)[previous] = merged[v];
        }
        next[tail[previous]] = last;
        tail[previous] = tail[last];
    }
    return best;
}
#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
