// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/LCA.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

void rejects_oversized_ladder() {
    // 配列を読み始める前に頂点数の範囲を検査する。
    constexpr auto kTooMany = static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1;
    sirkit::detail::AncestorLadder ladder;
    bool rejected = false;
    try {
        ladder.build(kTooMany, nullptr, nullptr, [](int) { return 0; }, nullptr, nullptr);
    } catch (const std::length_error& error) {
        rejected = std::string_view(error.what()) == "LCA: ancestor ladders too large";
    }
    assert(rejected);
}

void swaps_prepared_ladders() {
    constexpr std::array forward_parent{-1, 0, 1}, reverse_parent{1, 2, -1};
    constexpr std::array forward_order{0, 1, 2}, reverse_order{2, 1, 0};
    std::array<int, 3> height, heavy;
    sirkit::detail::AncestorLadder forward, reverse;
    forward.build(forward_parent.size(), forward_parent.data(), forward_order.data(),
        [](int vertex) { return vertex; }, height.data(), heavy.data());
    reverse.build(reverse_parent.size(), reverse_parent.data(), reverse_order.data(),
        [](int vertex) { return 2 - vertex; }, height.data(), heavy.data());
    swap(forward, reverse);
    assert(forward.climb(0, 2, reverse_parent.data()) == 2);
    assert(reverse.climb(2, 2, forward_parent.data()) == 0);
}

} // namespace

int main() {
    rejects_oversized_ladder();
    swaps_prepared_ladders();
    // 根0から子1への辺は1本。子側だけの重複で親辺の検査に入る。
    std::vector<std::vector<int>> graph{{1}, {0, 2}, {1, 3}, {2}, {}};
    sirkit::LCA tree(graph);
#ifndef SKIP_RUNTIME_CHECKS
    graph[1].push_back(0);
    bool rejected = false;
    try {
        tree.build(graph);
    } catch (const std::invalid_argument& error) {
        rejected = std::string_view(error.what()) == "LCA: parallel edges";
    }
    assert(rejected);
    graph[1].pop_back();
#else
    // 検査を省いた構成では、訪問済みの行先を再登録しない。
    const std::vector<std::vector<int>> cycle{{1, 2}, {0, 2}, {0, 1}};
    const sirkit::LCA traversal(cycle);
    assert(traversal.size() == 3 && traversal.component_count() == 1);
    assert(traversal.get(1, 2) == 0 && traversal.kth_ancestor(2, 1) == 0);
#endif
    // 構築失敗前の経路・成分・根を保ち、修復後の根変更にも対応する。
    assert(tree.size() == 5 && tree.component_count() == 2 && tree.root() == 0);
    assert(tree.get(1, 3) == 1 && tree.get(3, 4) == -1 && tree.kth_on_path(3, 0, 2) == 1);
    tree.build(graph, 3);
    assert(tree.root() == 3 && tree.get(1, 3) == 3 && tree.kth_ancestor(0, 2) == 2);
}
