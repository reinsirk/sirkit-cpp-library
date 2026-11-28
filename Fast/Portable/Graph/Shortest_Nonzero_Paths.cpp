// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Shortest_Nonzero_Paths.cpp
// 参考: Iwata–Yamaguchi, Finding a Shortest Non-zero Path in Group-Labeled Graphs, https://arxiv.org/abs/1906.04062
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_nonzero_paths.md
//
// API一覧
// GroupPathEdge<Label>{from,to,length,label}: 0始まりの無向辺。長さはuint32_t。from→toの群ラベルを渡す。
// shortest_nonzero_paths(n,edges,source,identity,combine,inverse):
//   群ラベルの積が単位元でない単純パスについて、始点から各頂点への最短長を返す。
//   逆向きの辺には逆元を使う。多重辺可。自己辺は使わない。
//   vector<optional<uint64_t>>を返す。該当するパスのない頂点と始点自身はnullopt。
//   群演算と比較がO(1)なら、O(n+m log(m+2)) 時間、O(n+m) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORTEST_NONZERO_PATHS_DEFINED
#define SIRKIT_SHORTEST_NONZERO_PATHS_DEFINED

template <typename Label>
struct GroupPathEdge
{
    int from;
    int to;
    std::uint32_t length;
    Label label;
};

template <typename Label, typename Combine, typename Inverse>
std::vector<std::optional<std::uint64_t>> shortest_nonzero_paths(
    int n, std::span<const GroupPathEdge<std::type_identity_t<Label>>> edges, int source, const Label &identity,
    Combine &&combine, Inverse &&inverse)
{
    struct Arc
    {
        int end;
        std::size_t edge;
    };
    std::vector<std::vector<Arc>> graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &edge = edges[id];
        if (edge.from != edge.to)
        {
            graph[edge.from].push_back({edge.to, id});
            graph[edge.to].push_back({edge.from, id});
        }
    }

    constexpr auto kUnreachable = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> distance(n, kUnreachable);
    std::vector<int> previous(n, -1), depth(n);
    std::vector<Label> labels(n, identity);
    {
        using Step = std::pair<std::uint64_t, int>;
        std::priority_queue<Step, std::vector<Step>, std::greater<Step>> pending;
        distance[source] = 0;
        pending.emplace(0, source);
        while (!pending.empty())
        {
            const auto [current, v] = pending.top();
            pending.pop();
            if (current != distance[v])
            {
                continue;
            }
            for (const auto &[w, id] : graph[v])
            {
                const auto &edge = edges[id];
                const auto candidate = current + edge.length;
                if (candidate < distance[w])
                {
                    distance[w] = candidate;
                    previous[w] = v;
                    depth[w] = depth[v] + 1;
                    if (v == edge.from)
                    {
                        labels[w] = std::invoke(combine, labels[v], edge.label);
                    }
                    else
                    {
                        labels[w] = std::invoke(combine, labels[v], std::invoke(inverse, edge.label));
                    }
                    pending.emplace(candidate, w);
                }
            }
        }
    }

    std::vector<std::optional<std::uint64_t>> result(n);
    for (int v = 0; v < n; ++v)
    {
        if (!(labels[v] == identity))
        {
            result[v] = distance[v];
        }
    }
    using Blossom = std::pair<std::uint64_t, std::size_t>;
    std::priority_queue<Blossom, std::vector<Blossom>, std::greater<Blossom>> pending;
    std::vector<bool> consistent(edges.size());
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &edge = edges[id];
        if (edge.from == edge.to || distance[edge.from] == kUnreachable)
        {
            continue;
        }
        consistent[id] = std::invoke(combine, labels[edge.from], edge.label) == labels[edge.to];
        if (!consistent[id])
        {
            pending.emplace(distance[edge.from] + distance[edge.to] + edge.length, id);
        }
    }

    // 縮約された成分の代表は、最短路木で最も浅い頂点。
    std::vector<int> representative(n), joined;
    std::iota(representative.begin(), representative.end(), 0);
    auto root = [&](int v)
    {
        while (representative[v] != v)
        {
            representative[v] = representative[representative[v]];
            v = representative[v];
        }
        return v;
    };
    while (!pending.empty())
    {
        const auto [height, id] = pending.top();
        pending.pop();
        auto from = root(edges[id].from);
        auto to = root(edges[id].to);
        while (from != to)
        {
            if (depth[from] < depth[to])
            {
                std::swap(from, to);
            }
            joined.push_back(from);
            from = root(previous[from]);
        }
        for (int v : joined)
        {
            representative[v] = from;
            const auto alternative = height - distance[v];
            if (!result[v])
            {
                result[v] = alternative;
            }
            for (const auto &[w, next] : graph[v])
            {
                if (consistent[next])
                {
                    pending.emplace(alternative + distance[w] + edges[next].length, next);
                }
            }
        }
        joined.clear();
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
