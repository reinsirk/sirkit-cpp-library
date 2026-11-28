// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_heap_orderings.md
//
// API一覧
// tree_heap_orderings(n, edges, prime=1000000007): 0 始まりの無向木を入力。
//   戻り値 [r] は、r を根として親を子より先に並べる頂点順列の個数を prime で割った余り。
//   prime は n より大きい素数。素数性は呼び出し側で保証する。n=0 かつ辺なしなら空列。
//   O(n+1) 時間・領域。再帰を使わない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_HEAP_ORDERINGS_DEFINED
#define SIRKIT_TREE_HEAP_ORDERINGS_DEFINED

inline std::vector<std::uint32_t> tree_heap_orderings(
    int n, std::span<const std::pair<int, int>> edges,
    std::uint32_t prime = 1000000007)
{
    if (n < 0)
    {
        throw std::length_error("tree_heap_orderings: negative vertex count");
    }
    const auto vertex_count = static_cast<std::uint32_t>(n);
    if (prime < 2 || prime <= vertex_count)
    {
        throw std::invalid_argument("tree_heap_orderings: modulus must be a prime larger than n");
    }
    if (edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1))
    {
        throw std::invalid_argument("tree_heap_orderings: wrong edge count");
    }
    if (n == 0)
    {
        return {};
    }
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("tree_heap_orderings: edge endpoint");
        }
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    std::vector<int> parent(n, -1), order{0};
    order.reserve(n);
    parent[0] = 0;
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        const int v = order[i];
        for (const int w : graph[v])
        {
            if (parent[w] == -1)
            {
                parent[w] = v;
                order.push_back(w);
            }
        }
    }
    if (order.size() != vertex_count)
    {
        throw std::invalid_argument("tree_heap_orderings: graph is disconnected");
    }
    std::vector<std::uint32_t> subtree(n, 1);
    for (std::size_t i = order.size() - 1; i != 0; --i)
    {
        const int v = order[i];
        subtree[parent[v]] += subtree[v];
    }

    const auto multiply = [prime](std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>(static_cast<std::uint64_t>(a) * b % prime);
    };
    std::vector<std::uint32_t> inverse(static_cast<std::size_t>(n) + 1);
    inverse[1] = 1;
    std::uint32_t factorial = 1;
    for (std::uint32_t k = 2; k <= vertex_count; ++k)
    {
        inverse[k] = prime - multiply(prime / k, inverse[prime % k]);
        factorial = multiply(factorial, k);
    }

    // フック長公式の分母は全頂点の部分木サイズの積。
    std::vector<std::uint32_t> answer(n);
    for (const auto size : subtree)
    {
        factorial = multiply(factorial, inverse[size]);
    }
    answer[0] = factorial;
    for (std::size_t i = 1; i < order.size(); ++i)
    {
        const int v = order[i];
        const auto size = subtree[v];
        answer[v] = multiply(multiply(answer[parent[v]], size), inverse[vertex_count - size]);
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
