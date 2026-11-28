// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Palindromic_Shortest_Paths.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/palindromic_shortest_paths.md
//
// API一覧
// PalindromicPathEdge{from,to,label}: 0始まりの端点と uint8_t のラベル。
// PalindromicShortestPaths(n,edges,undirected=false): ラベル列が回文になる最短歩道を全頂点対で前計算。
//   自己辺・多重辺可。頂点や辺は繰り返し通れる。辺 ID は入力順。
//   ラベルの種類を σ、ラベル c の辺数を m_c として時間 O((σ+1)n²+Σm_c²)、領域 O(n²+m)。
// size(): 頂点数。distance(from,to): 最短歩道の長さ、存在しなければ -1。ともに O(1)。
// path(from,to): 頂点列 vertices と辺 ID 列 edges を含む Walk。存在しなければ nullopt。長さ ℓ で O(ℓ+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PALINDROMIC_SHORTEST_PATHS_DEFINED
#define SIRKIT_PALINDROMIC_SHORTEST_PATHS_DEFINED

struct PalindromicPathEdge
{
    int from;
    int to;
    std::uint8_t label;
};

class PalindromicShortestPaths
{
public:
    struct Walk
    {
        std::vector<int> vertices;
        std::vector<std::size_t> edges;
    };

    explicit PalindromicShortestPaths(int n, std::span<const PalindromicPathEdge> edges, bool undirected = false)
        : n_(n)
    {
        if (n < 0)
        {
            throw std::length_error("PalindromicShortestPaths: negative vertex count");
        }
        const auto count = static_cast<std::size_t>(n);
        if (count != 0 && count > states_.max_size() / count)
        {
            throw std::length_error("PalindromicShortestPaths: too many vertex pairs");
        }
        states_.resize(count * count);
        std::vector<std::size_t> pending;
        pending.reserve(count);
        for (int u = 0; u < n; ++u)
        {
            const auto where = index(u, u);
            states_[where].distance = 0;
            pending.push_back(where);
        }

        std::array<int, 256> label_ids;
        label_ids.fill(-1);
        int alphabet = 0;
        for (const auto &[from, to, label] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            check_vertex(from);
            check_vertex(to);
#endif
            if (label_ids[label] == -1)
            {
                label_ids[label] = alphabet++;
            }
        }

        struct Arc
        {
            int vertex;
            std::size_t id;
        };
        std::vector<std::vector<Arc>> incoming(count * alphabet), outgoing(count * alphabet);
        auto bucket = [alphabet](int vertex, int label)
        {
            return static_cast<std::size_t>(vertex) * alphabet + label;
        };
        auto add_arc = [&](int from, int to, int label, std::size_t id)
        {
            outgoing[bucket(from, label)].push_back({to, id});
            incoming[bucket(to, label)].push_back({from, id});
            const auto where = index(from, to);
            if (states_[where].distance == -1)
            {
                states_[where] = State{1, 0, id, 0};
                pending.push_back(where);
            }
        };
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto [u, v, label] = edges[id];
            add_arc(u, v, label_ids[label], id);
            if (undirected && u != v)
            {
                add_arc(v, u, label_ids[label], id);
            }
        }

        // 長さ0と1の中心を入れてから、同じラベルの辺を両端に足す。
        for (std::size_t cursor = 0; cursor < pending.size() && pending.size() != states_.size(); ++cursor)
        {
            const auto inside = pending[cursor];
            const int u = static_cast<int>(inside / count);
            const int v = static_cast<int>(inside % count);
            const std::int64_t length = states_[inside].distance + 2;
            for (int label = 0; label < alphabet; ++label)
            {
                for (const auto &[from, left_id] : incoming[bucket(u, label)])
                {
                    for (const auto &[to, right_id] : outgoing[bucket(v, label)])
                    {
                        const auto outside = index(from, to);
                        if (states_[outside].distance == -1)
                        {
                            states_[outside] = State{length, inside, left_id, right_id};
                            pending.push_back(outside);
                        }
                    }
                }
            }
        }
    }

    int size() const noexcept
    {
        return n_;
    }

    std::int64_t distance(int from, int to) const
    {
        return states_[checked_index(from, to)].distance;
    }

    std::optional<Walk> path(int from, int to) const
    {
        auto where = checked_index(from, to);
        const std::int64_t distance = states_[where].distance;
        if (distance == -1)
        {
            return std::nullopt;
        }
        const auto length = static_cast<std::size_t>(distance);
        Walk result{std::vector<int>(length + 1), std::vector<std::size_t>(length)};
        std::size_t left = 0;
        std::size_t right = length;
        const auto count = static_cast<std::size_t>(n_);
        for (;;)
        {
            result.vertices[left] = static_cast<int>(where / count);
            result.vertices[right] = static_cast<int>(where % count);
            if (left == right)
            {
                break;
            }
            const auto &state = states_[where];
            result.edges[left] = state.left_edge;
            if (left + 1 == right)
            {
                break;
            }
            result.edges[right - 1] = state.right_edge;
            where = state.inside;
            ++left;
            --right;
        }
        return result;
    }

private:
    struct State
    {
        std::int64_t distance = -1;
        std::size_t inside = 0;
        std::size_t left_edge = 0;
        std::size_t right_edge = 0;
    };

    int n_;
    std::vector<State> states_;

#ifndef SKIP_RUNTIME_CHECKS
    void check_vertex(int v) const
    {
        if (v < 0 || v >= n_)
        {
            throw std::out_of_range("PalindromicShortestPaths: invalid vertex");
        }
    }
#endif

    std::size_t index(int from, int to) const noexcept
    {
        return static_cast<std::size_t>(from) * n_ + to;
    }

    std::size_t checked_index(int from, int to) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        check_vertex(from);
        check_vertex(to);
#endif
        return index(from, to);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
