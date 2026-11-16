// SPDX-License-Identifier: CC0-1.0
#include "benchmark_common.hpp"
#include "library.hpp"
#include <algorithm>
#include <vector>

namespace sirkit::detail::benchmark_union_find {
struct Input {
    std::size_t n, pattern;
    std::vector<std::pair<int, int>> edges, queries;
};
Input generate(std::size_t n, std::size_t pattern) {
    std::uint64_t seed = 20261114;
    Input input{n, pattern, {}, {}};
    const auto balanced = [&](std::size_t first, std::size_t last) {
        for (std::size_t step = 1; step < last - first; step *= 2)
            for (std::size_t i = first; i + step < last; i += step * 2)
                input.edges.emplace_back(static_cast<int>(i), static_cast<int>(i + step));
    };
    if (pattern < 2) {
        std::vector<int> order(n);
        for (std::size_t i = 0; i < n; ++i) order[i] = static_cast<int>(i);
        if (!pattern)
            for (std::size_t i = n; i > 1; --i)
                std::swap(order[i - 1], order[::sirkit::detail::benchmark_random(seed) % i]);
        for (int vertex : order) input.edges.emplace_back(vertex, (vertex + 1) % static_cast<int>(n));
    } else if (pattern == 2) {
        // 同じ大きさ同士を結び、最初の照会前のUF木を深くする。
        balanced(0, n);
    } else {
        const auto half = n / 2;
        balanced(0, half); balanced(half, n);
        for (std::size_t i = 0; i < n; ++i) {
            input.edges.emplace_back(static_cast<int>(i), static_cast<int>(i));
            input.edges.emplace_back(static_cast<int>(i), static_cast<int>(i < half ? 0 : half));
        }
    }
    input.queries.reserve(n * 4);
    for (std::size_t i = 0; i < n * 4; ++i) {
        const int vertex = pattern == 2 ? static_cast<int>(n - 1 - i % n)
            : static_cast<int>(::sirkit::detail::benchmark_random(seed) % n);
        int other = static_cast<int>((static_cast<std::size_t>(vertex) + n / 2) % n);
        if (pattern == 3) {
            const bool first = static_cast<std::size_t>(vertex) < n / 2;
            other = static_cast<int>((i & 1) ? (first ? 0 : n / 2) : (first ? n / 2 : 0));
        }
        input.queries.emplace_back(vertex, other);
    }
    return input;
}
std::uint64_t reference(const Input& input) {
    // 結合の定義から成分は全体、または[0,n/2)・[n/2,n)の2個。
    std::uint64_t checksum = 0;
    for (auto [vertex, other] : input.queries) {
        const bool first = static_cast<std::size_t>(vertex) < input.n / 2;
        const auto size = input.pattern == 3 ? (first ? input.n / 2 : input.n - input.n / 2) : input.n;
        const bool same = input.pattern != 3 || first == (static_cast<std::size_t>(other) < input.n / 2);
        checksum = ::sirkit::detail::benchmark_digest(checksum, size);
        checksum = ::sirkit::detail::benchmark_digest(checksum, same);
    }
    return checksum;
}
std::uint64_t kernel(const Input& input) {
    UnionFind tree(input.n);
    for (auto [from, to] : input.edges) tree.unite(from, to);
    std::uint64_t checksum = 0;
    for (auto [vertex, other] : input.queries) {
        checksum = ::sirkit::detail::benchmark_digest(checksum, static_cast<std::uint64_t>(tree.size(vertex)));
        checksum = ::sirkit::detail::benchmark_digest(checksum, tree.issame(vertex, other));
    }
    return checksum;
}
}
int main(int argc, char** argv) {
    return ::sirkit::detail::benchmark_main(argc, argv, ::sirkit::detail::benchmark_union_find::generate,
        ::sirkit::detail::benchmark_union_find::reference, ::sirkit::detail::benchmark_union_find::kernel);
}
