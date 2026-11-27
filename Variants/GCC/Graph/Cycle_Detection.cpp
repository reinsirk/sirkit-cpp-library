// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Cycle_Detection.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cycle_detection.md
//
// API一覧
// find_cycle(n,edges,directed=true): 有向/無向多重グラフの単純閉路を1つ返す。なければnullopt。
// Cycle{vertices,edge_ids}: i番目の辺はvertices[i]から次の頂点へ進む。辺IDは入力位置。
// 頂点番号は[0,n)。自己辺と平行辺を扱う。O(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CYCLE_DETECTION_DEFINED
#define SIRKIT_CYCLE_DETECTION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t cycle_arc_count(std::size_t edges, std::size_t directions, std::size_t limit)
{
    if (edges > limit / directions)
    {
        throw std::length_error("find_cycle: too many edges");
    }
    return edges * directions;
}
} // namespace detail

struct Cycle
{
    std::vector<int> vertices;
    std::vector<std::size_t> edge_ids;
};

inline std::optional<Cycle> find_cycle(int n, std::span<const std::pair<int, int>> edges, bool directed = true)
{
    if (n < 0)
    {
        throw std::length_error("find_cycle: negative vertex count");
    }
    struct Arc
    {
        int to;
        std::size_t id;
    };
    const auto directions = directed ? 1U : 2U;
    const auto arc_count = detail::cycle_arc_count(edges.size(), directions, std::vector<Arc>{}.max_size());

    std::vector<std::size_t> offset(static_cast<std::size_t>(n) + 1);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("find_cycle: vertex out of range");
        }
#endif
        ++offset[u];
        if (!directed)
        {
            ++offset[v];
        }
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());
    std::vector<Arc> adjacency(arc_count);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        adjacency[--offset[u]] = {v, id};
        if (!directed)
        {
            adjacency[--offset[v]] = {u, id};
        }
    }

    struct Frame
    {
        int vertex;
        std::size_t next;
        std::size_t incoming;
    };
    constexpr int kUnseen = -1;
    constexpr int kFinished = -2;
    std::vector<int> position(n, kUnseen);
    std::vector<Frame> stack;
    for (int start = 0; start < n; ++start)
    {
        if (position[start] != kUnseen)
        {
            continue;
        }
        position[start] = 0;
        stack.push_back({start, offset[start], edges.size()});
        while (!stack.empty())
        {
            auto &frame = stack.back();
            if (frame.next == offset[frame.vertex + 1])
            {
                position[frame.vertex] = kFinished;
                stack.pop_back();
                continue;
            }
            const auto [to, id] = adjacency[frame.next++];
            if (!directed && id == frame.incoming)
            {
                continue;
            }
            if (position[to] == kUnseen)
            {
                position[to] = static_cast<int>(stack.size());
                stack.push_back({to, offset[to], id});
            }
            else if (position[to] >= 0)
            {
                const auto first = static_cast<std::size_t>(position[to]);
                Cycle result;
                result.vertices.reserve(stack.size() - first);
                result.edge_ids.reserve(stack.size() - first);
                result.vertices.push_back(to);
                for (std::size_t i = first + 1; i < stack.size(); ++i)
                {
                    result.vertices.push_back(stack[i].vertex);
                    result.edge_ids.push_back(stack[i].incoming);
                }
                result.edge_ids.push_back(id);
                return result;
            }
        }
    }
    return std::nullopt;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
