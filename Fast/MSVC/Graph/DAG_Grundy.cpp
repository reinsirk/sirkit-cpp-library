// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/DAG_Grundy.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dag_grundy.md
//
// API一覧
// dag_grundy_numbers(moves): 各頂点から選べる手を有向辺とし、DAG上の不偏ゲームのGrundy数を返す。
//   moves[v]は0始まりの移動先の列。動けない頂点は0。閉路があればnullopt。
//   独立した局面を合わせるなら、局面のGrundy数のXORが0のとき手番側が負け。
//   構築・計算O(n+m)時間、追加O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_TOPOLOGICAL_RESULT_DEFINED
#define SIRKIT_TOPOLOGICAL_RESULT_DEFINED
struct TopologicalResult {
    std::vector<int> order;
    std::vector<int> position;
    bool acyclic = true;
    bool unique = true;
};
#endif

#ifndef SIRKIT_TOPOLOGICAL_READY_MASK_DEFINED
#define SIRKIT_TOPOLOGICAL_READY_MASK_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline std::uint64_t TopologicalReadyMask(const std::uint32_t* degree) {
    std::uint64_t mask = 0;
    for (unsigned i = 0; i < 64; ++i)
        mask |= static_cast<std::uint64_t>(degree[i] == 0) << i;
    return mask;
}
} // namespace reinsirk::detail
#endif

#ifndef SIRKIT_TOPOLOGICAL_CORE_DEFINED
#define SIRKIT_TOPOLOGICAL_CORE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void CheckTopologicalVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("TopologicalSort: too many vertices");
}

inline void AddTopologicalDegree(std::uint32_t& degree) {
    if (degree == std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("TopologicalSort: degree overflow");
    ++degree;
}

template <bool Lexicographical, bool Details, bool Bulk = false>
inline TopologicalResult TopologicalRun(const Graph& graph) {
    const auto n = graph.size();
    CheckTopologicalVertexCount(n);
    std::vector<std::uint32_t> indegree(n);
    for (const auto& edges : graph) for (int to : edges) {
        AddTopologicalDegree(indegree[static_cast<std::size_t>(to)]);
    }
    TopologicalResult result;
    result.order.reserve(n);
    if constexpr (Details) result.position.assign(n, -1);
    if constexpr (!Lexicographical) {
        // 各頂点は高々1回追加し、reserve(n)済みなので容量を超えない。
        const auto append = [&](int vertex) {
            if (result.order.size() == result.order.capacity()) std::unreachable();
            result.order.push_back(vertex);
        };
        std::uint32_t degree_bits = 0;
        {
            for (std::size_t i = 0; i < n; ++i) {
                degree_bits |= indegree[i];
                if (indegree[i] == 0) append(static_cast<int>(i));
            }
        }
        if constexpr (!Details && Bulk) {
            // 全頂点が初期候補なら辺はなく、行先の再走査は不要。
            if (result.order.size() == n) return result;
            // 入次数が高々1なら、行先は必ず初めて候補になる。
            if (degree_bits <= 1) {
                for (std::size_t head = 0; head < result.order.size(); ++head)
                    for (int to : graph[static_cast<std::size_t>(result.order[head])]) append(to);
                result.acyclic = result.order.size() == n;
                if (!result.acyclic) result.order.clear();
                return result;
            }
        }
        if (n >= 4096 && degree_bits > 1 && result.order.size() > n / 64 && result.order.size() < n - n / 4) {
            // 到達できる頂点が十分増えてから、添字式キューに切り替える。
            const auto switch_size = result.order.size() + n / 16;
            std::size_t head = 0;
            while (head < result.order.size() &&
                   (result.order.size() < switch_size || result.order.size() - head <= n / 64)) {
                if constexpr (Details) {
                    if (result.order.size() - head > 1) result.unique = false;
                    result.position[static_cast<std::size_t>(result.order[head])] = static_cast<int>(head);
                }
                for (int to : graph[static_cast<std::size_t>(result.order[head])])
                    if (--indegree[static_cast<std::size_t>(to)] == 0) append(to);
                ++head;
            }
            if (head < result.order.size()) {
                std::size_t tail = result.order.size();
                result.order.resize(n);
                for (; head < tail; ++head) {
                    if constexpr (Details) {
                        if (tail - head > 1) result.unique = false;
                        result.position[static_cast<std::size_t>(result.order[head])] = static_cast<int>(head);
                    }
                    for (int to : graph[static_cast<std::size_t>(result.order[head])]) {
                        const bool ready = --indegree[static_cast<std::size_t>(to)] == 0;
                        // 全頂点が入隊済みなら未処理の辺はなく、常にtail < n。
                        result.order[tail] = to;
                        tail += ready;
                    }
                }
                result.order.resize(tail);
            }
        } else {
            for (std::size_t head = 0; head < result.order.size(); ++head) {
                if constexpr (Details) {
                    if (result.order.size() - head > 1) result.unique = false;
                    result.position[static_cast<std::size_t>(result.order[head])] = static_cast<int>(head);
                }
                for (int to : graph[static_cast<std::size_t>(result.order[head])])
                    if (--indegree[static_cast<std::size_t>(to)] == 0) append(to);
            }
        }
    } else {
        std::array<std::size_t, 6> offsets{};
        std::size_t words = (n + 63) / 64, total = 0, levels = 0;
        // 頂点数はINT_MAX以下なので、64進の階層は高々6段。
        do {
            offsets[levels++] = total;
            total += words;
            if (words <= 1) break;
            words = (words + 63) / 64;
        } while (true);
        std::uint64_t single_word = 0;
        std::vector<std::uint64_t> large_bits;
        auto* bits = n <= 64 ? &single_word : nullptr;
        std::size_t ready_count = 0;
        // 最小値を別に保持し、候補1個の間はビット集合を使わない。
        int minimum = -1;
        const auto insert = [&](int vertex) {
            ++ready_count;
            if (minimum < 0) { minimum = vertex; return; }
            if (vertex < minimum) std::swap(vertex, minimum);
            if (bits == nullptr) {
                // 複数候補が初めて現れたときだけ確保する。
                large_bits.assign(total, 0);
                bits = large_bits.data();
            }
            std::size_t index = static_cast<std::size_t>(vertex);
            for (std::size_t level = 0; level < levels; ++level) {
                auto& word = bits[offsets[level] + (index >> 6)];
                const auto old = word;
                word |= std::uint64_t{1} << (index & 63);
                if (old != 0) break;
                index >>= 6;
            }
        };
        auto make_mask = &TopologicalReadyMask;
        bool bulk_initialize = false;
        if (bulk_initialize) {
            for (std::size_t base = 0; base < n; base += 64) {
                std::uint64_t mask = 0;
                if (n - base >= 64) mask = make_mask(indegree.data() + base);
                else for (std::size_t i = base; i < n; ++i)
                    mask |= static_cast<std::uint64_t>(indegree[i] == 0) << (i - base);
                ready_count += std::popcount(mask);
                if (mask != 0 && minimum < 0) {
                    minimum = static_cast<int>(base + std::countr_zero(mask));
                    mask &= mask - 1;
                }
                if (mask != 0) {
                    if (bits == nullptr) {
                        large_bits.assign(total, 0);
                        bits = large_bits.data();
                    }
                    bits[base >> 6] = mask;
                }
            }
            if (ready_count > 1) {
                std::size_t count = (n + 63) / 64;
                for (std::size_t level = 0; level + 1 < levels; ++level) {
                    for (std::size_t i = 0; i < count; ++i)
                        if (bits[offsets[level] + i] != 0)
                            bits[offsets[level + 1] + (i >> 6)] |= std::uint64_t{1} << (i & 63);
                    count = (count + 63) / 64;
                }
            }
        } else {
            for (std::size_t i = 0; i < n; ++i)
                if (indegree[i] == 0) insert(static_cast<int>(i));
        }
        while (ready_count != 0) {
            if constexpr (Details) if (ready_count > 1) result.unique = false;
            const int from = minimum;
            minimum = -1;
            if (--ready_count != 0) [[unlikely]] {
                std::size_t block = static_cast<std::size_t>(from) >> 6;
                auto remaining = bits[block];
                if (remaining == 0) {
                    block = std::countr_zero(bits[offsets[levels - 1]]);
                    for (std::size_t level = levels - 1; level-- > 1;)
                        block = (block << 6) + std::countr_zero(bits[offsets[level] + block]);
                    remaining = bits[block];
                }
                minimum = static_cast<int>((block << 6) + std::countr_zero(remaining));
                remaining &= remaining - 1;
                bits[block] = remaining;
                if (remaining == 0) {
                    std::size_t index = block;
                    for (std::size_t level = 1; level < levels; ++level) {
                        auto& word = bits[offsets[level] + (index >> 6)];
                        word &= word - 1;
                        if (word != 0) break;
                        index >>= 6;
                    }
                }
            }
            if constexpr (Details)
                result.position[static_cast<std::size_t>(from)] = static_cast<int>(result.order.size());
            result.order.push_back(from);
            for (int to : graph[static_cast<std::size_t>(from)])
                if (--indegree[static_cast<std::size_t>(to)] == 0) insert(to);
        }
    }
    result.acyclic = result.order.size() == n;
    if constexpr (Details) result.unique = result.unique && result.acyclic;
    else if (!result.acyclic) result.order.clear();
    return result;
}
} // namespace reinsirk::detail
#endif

#ifndef SIRKIT_TOPOLOGICAL_SORT_DEFINED
#define SIRKIT_TOPOLOGICAL_SORT_DEFINED
inline std::vector<int> TopologicalSort(const Graph& graph) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::TopologicalRun<false, false>(graph).order;
}
#endif

#ifndef SIRKIT_DAG_GRUNDY_DEFINED
#define SIRKIT_DAG_GRUNDY_DEFINED

inline std::optional<std::vector<int>> dag_grundy_numbers(const Graph &moves)
{
    const auto order = TopologicalSort(moves);
    if (order.size() != moves.size())
    {
        return std::nullopt;
    }

    std::vector<int> grundy(moves.size()), marked(moves.size() + 1, -1);
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int vertex = *it;
        for (const int next : moves[vertex])
        {
            marked[grundy[next]] = vertex;
        }
        int mex = 0;
        while (marked[mex] == vertex)
        {
            ++mex;
        }
        grundy[vertex] = mex;
    }
    return grundy;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
