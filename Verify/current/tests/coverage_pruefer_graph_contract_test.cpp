// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Pruefer_Code.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

using Edge = std::pair<int, int>;
constexpr int kVertices = 4;
constexpr std::array<Edge, 6> kEdges{{{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}}};

std::array<int, kVertices> degrees(const std::vector<Edge>& edges) {
    std::array<int, kVertices> result{};
    for (const auto& [u, v] : edges) {
        ++result[u];
        ++result[v];
    }
    return result;
}

bool spans_tree(const std::vector<Edge>& edges) {
    if (edges.size() != kVertices - 1) return false;
    std::array<bool, kVertices> seen{true};
    for (int step = 0; step < kVertices - 1; ++step) {
        for (const auto& [u, v] : edges) {
            if (seen[u] || seen[v]) seen[u] = seen[v] = true;
        }
    }
    return std::ranges::all_of(seen, [](bool reachable) { return reachable; });
}

std::vector<Edge> edge_set(unsigned mask) {
    std::vector<Edge> result;
    for (std::size_t i = 0; i < kEdges.size(); ++i) {
        if ((mask & (1U << i)) != 0) result.push_back(kEdges[i]);
    }
    return result;
}

template <class Error, class Call>
void rejects(Call call) {
    try {
        call();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

} // namespace

int main() {
    const std::vector<Edge> example{{0, 2}, {1, 2}, {2, 3}, {3, 4}};
    assert(sirkit::pruefer_encode(5, example) == (std::vector{2, 2, 3}));
    std::vector<std::pair<unsigned, std::array<int, kVertices>>> trees;
    for (unsigned mask = 0; mask < (1U << kEdges.size()); ++mask) {
        const auto edges = edge_set(mask);
        if (!spans_tree(edges)) continue;
        trees.emplace_back(mask, degrees(edges));
        const auto code = sirkit::pruefer_encode(kVertices, edges);
        auto decoded = sirkit::pruefer_decode(code);
        for (auto& [u, v] : decoded) if (u > v) std::swap(u, v);
        std::ranges::sort(decoded);
        assert(decoded == edges);
    }
    for (const auto& candidate : trees) {
        const auto& target = candidate.second;
        for (unsigned partial = 0; partial < (1U << kEdges.size()); ++partial) {
            auto fixed = edge_set(partial);
            for (auto& [u, v] : fixed) std::swap(u, v);
            std::ranges::reverse(fixed);
            const bool possible = std::ranges::any_of(trees, [&](const auto& tree) {
                return tree.second == target && (tree.first & partial) == partial;
            });
            const auto extra = sirkit::complete_tree_with_degrees(target, fixed);
            assert(extra.has_value() == possible);
            if (extra) {
                fixed.insert(fixed.end(), extra->begin(), extra->end());
                assert(spans_tree(fixed));
                assert(degrees(fixed) == target);
            }
        }
    }
    assert(!sirkit::pruefer_from_degrees(std::array{1, 1, 1}));
    assert(!sirkit::complete_tree_with_degrees({}, {}));
    // 成分数は4。辺0-1を含む成分の残り次数4は、追加辺3本で実現できない。
    assert(!sirkit::complete_tree_with_degrees(std::array{4, 2, 1, 1, 1, 1},
                                               std::array{Edge{0, 1}, Edge{2, 3}}));
    constexpr auto kIntLimit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    assert(sirkit::detail::PrueferVertexCount(kIntLimit) == std::numeric_limits<int>::max());
    assert(sirkit::detail::PrueferVertexCount(kIntLimit - 2, 2) == std::numeric_limits<int>::max());
    rejects<std::length_error>([] { (void)sirkit::detail::PrueferVertexCount(kIntLimit + 1); });
    rejects<std::length_error>([] { (void)sirkit::detail::PrueferVertexCount(kIntLimit - 1, 2); });

#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([] { (void)sirkit::pruefer_encode(0, {}); });
    rejects<std::invalid_argument>([] { (void)sirkit::pruefer_encode(2, {}); });
    rejects<std::out_of_range>([] {
        (void)sirkit::pruefer_degrees(3, std::array{3});
    });
    for (const Edge edge : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}, Edge{0, 0}}) {
        rejects<std::invalid_argument>([&] {
            (void)sirkit::pruefer_encode(2, std::array{edge});
        });
    }
    for (const Edge edge : {Edge{-1, 0}, Edge{4, 0}, Edge{0, -1}, Edge{0, 4}}) {
        rejects<std::out_of_range>([&] {
            (void)sirkit::complete_tree_with_degrees(std::array{1, 2, 2, 1}, std::array{edge});
        });
    }
    for (std::size_t i = 0; i < kEdges.size(); ++i) {
        for (std::size_t j = i; j < kEdges.size(); ++j) {
            for (std::size_t k = j; k < kEdges.size(); ++k) {
                const std::vector<Edge> edges{kEdges[i], kEdges[j], kEdges[k]};
                if (spans_tree(edges)) continue;
                rejects<std::invalid_argument>([&] { (void)sirkit::pruefer_encode(4, edges); });
            }
        }
    }
#endif
}
