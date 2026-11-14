// SPDX-License-Identifier: CC0-1.0
#include "benchmark_common.hpp"
#include "library.hpp"
#include <vector>

namespace sirkit::detail::benchmark_union_find {
struct Input { std::vector<int> edges, queries; };
Input generate(std::size_t n, std::size_t pattern) {
    std::uint64_t seed = 20261114;
    Input input;
    input.edges.resize(n);
    for (std::size_t i = 0; i < n; ++i) input.edges[i] = static_cast<int>(i);
    if (!pattern) {
        for (std::size_t i = n; i > 1; --i)
            std::swap(input.edges[i - 1], input.edges[::sirkit::detail::benchmark_random(seed) % i]);
    }
    input.queries.reserve(n * 4);
    for (std::size_t i = 0; i < n * 4; ++i)
        input.queries.push_back(static_cast<int>(::sirkit::detail::benchmark_random(seed) % n));
    return input;
}
std::uint64_t reference(const Input& input) {
    // 全ての隣接辺を結ぶので、最後には全頂点が同じ成分となる。
    std::uint64_t checksum = 0;
    for (int ignored : input.queries) {
        (void)ignored;
        checksum = ::sirkit::detail::benchmark_digest(checksum, input.edges.size());
        checksum = ::sirkit::detail::benchmark_digest(checksum, 1);
    }
    return checksum;
}
std::uint64_t kernel(const Input& input) {
    const int n = static_cast<int>(input.edges.size());
    UnionFind tree(input.edges.size());
    for (int vertex : input.edges) tree.unite(vertex, (vertex + 1) % n);
    std::uint64_t checksum = 0;
    for (int vertex : input.queries) {
        checksum = ::sirkit::detail::benchmark_digest(checksum, static_cast<std::uint64_t>(tree.size(vertex)));
        checksum = ::sirkit::detail::benchmark_digest(checksum, tree.issame(vertex, (vertex + n / 2) % n));
    }
    return checksum;
}
}
int main(int argc, char** argv) {
    return ::sirkit::detail::benchmark_main(argc, argv, ::sirkit::detail::benchmark_union_find::generate,
        ::sirkit::detail::benchmark_union_find::reference, ::sirkit::detail::benchmark_union_find::kernel);
}
