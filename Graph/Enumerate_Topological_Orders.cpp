// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/enumerate_topological_orders.md
//
// API一覧
// enumerate_topological_orders(n,edges,k): 相異なるトポロジカル順序を最大 k 個返す。
//   頂点は0始まり。edges は有向辺(先に置く頂点,後に置く頂点)。
//   順序の並び方は未規定。閉路があれば空、0頂点なら空順序が1個ある。
//   返した個数を r とすると O((r+1)(n+m)) 時間、O(n+m+rn) 領域。再帰を使わない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ENUMERATE_TOPOLOGICAL_ORDERS_DEFINED
#define SIRKIT_ENUMERATE_TOPOLOGICAL_ORDERS_DEFINED

inline std::vector<std::vector<int>> enumerate_topological_orders(
    int n, std::span<const std::pair<int, int>> edges, std::size_t k)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::length_error("enumerate_topological_orders: negative vertex count");
    }
#endif
    if (k == 0)
    {
        return {};
    }
    std::vector<std::vector<int>> graph(n);
    std::vector<std::size_t> indegree(n);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("enumerate_topological_orders: invalid endpoint");
        }
#endif
        graph[u].push_back(v);
        ++indegree[v];
    }
    std::vector<int> available;
    available.reserve(static_cast<std::size_t>(n));
    for (int v = 0; v < n; ++v)
    {
        if (indegree[v] == 0)
        {
            available.push_back(v);
        }
    }

    struct Step
    {
        int vertex;
        std::size_t index;
        std::size_t available_before;
    };
    const auto vertex_count = static_cast<std::size_t>(n);
    std::vector<Step> prefix;
    prefix.reserve(vertex_count);
    std::vector<std::size_t> next_choice(vertex_count + 1);
    std::vector<std::vector<int>> result;
    for (;;)
    {
        const auto depth = prefix.size();
        if (depth == vertex_count)
        {
            auto &answer = result.emplace_back();
            answer.reserve(vertex_count);
            for (const auto &step : prefix)
            {
                answer.push_back(step.vertex);
            }
            if (result.size() == k)
            {
                return result;
            }
        }
        else if (available.empty())
        {
            // 頂点が残ったまま入次数0の候補が尽きたので、元のグラフに閉路がある。
            return {};
        }
        else if (next_choice[depth] < available.size())
        {
            const auto index = next_choice[depth]++;
            const int u = available[index];
            prefix.push_back({u, index, available.size()});
            std::swap(available[index], available.back());
            available.pop_back();
            for (int v : graph[u])
            {
                if (--indegree[v] == 0)
                {
                    available.push_back(v);
                }
            }
            next_choice[depth + 1] = 0;
            continue;
        }

        if (prefix.empty())
        {
            return result;
        }
        const auto [u, index, before] = prefix.back();
        prefix.pop_back();
        for (int v : graph[u])
        {
            ++indegree[v];
        }
        available.resize(before - 1);
        available.push_back(u);
        std::swap(available[index], available.back());
    }
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
