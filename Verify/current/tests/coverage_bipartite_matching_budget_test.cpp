// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bipartite_Matching.cpp"

#include <array>
#include <cassert>
#include <limits>
#include <stdexcept>

namespace {

template<class Error, class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

void empty_sides_and_invalid_arguments() {
    using Matching = sirkit::BipartiteMatching;
    assert(Matching{}.empty());
    assert(!Matching(0, 1).empty());
    assert(!Matching(1, 0).empty());
    rejects<std::length_error>([] { Matching invalid(-1, 0); });
    rejects<std::length_error>([] { Matching invalid(0, -1); });
    Matching matching(1, 1);
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::out_of_range>([&] { matching.add_edge(1, 0); });
    rejects<std::out_of_range>([&] { matching.add_edge(0, -1); });
#endif
    assert(matching.edge_count() == 0 && matching.solve() == 0);
    matching.add_edge(0, 0);
    const auto& same = matching;
    matching = same;
    assert(matching.edge_count() == 1 && matching.solve() == 1);
}

void work_budget_boundaries() {
    constexpr auto kLimit = std::numeric_limits<std::size_t>::max();
    using sirkit::detail::BipartiteWorkBudget;
    using sirkit::detail::CheckBipartiteEdgeCount;
    CheckBipartiteEdgeCount(kLimit - 1);
    rejects<std::length_error>([] { CheckBipartiteEdgeCount(kLimit); });
    const auto [small_unit, small_total] = BipartiteWorkBudget(4, 9);
    assert(small_unit == 13 && small_total == 64 * 13);
    const auto [large_unit, large_total] = BipartiteWorkBudget(kLimit / 64, 1);
    assert(large_unit == kLimit / 64 + 1 && large_total == kLimit);
    const auto [saturated_unit, saturated_total] = BipartiteWorkBudget(kLimit - 1, 2);
    assert(saturated_unit == kLimit && saturated_total == kLimit);
}

void long_paths_with_contended_vertex(bool transpose, bool hall_deficiency, bool equal_lengths = false) {
    constexpr int kChain = 512;
    constexpr int kShorterChain = 448;
    constexpr int kContenders = 3;
    constexpr int kParallel = 1'024;
    const int second_length = equal_lengths ? kChain : kShorterChain;
    const std::array chain_lengths{kChain, hall_deficiency ? second_length : 0};
    const int chain_vertices = chain_lengths[0] + chain_lengths[1];
    const int left = chain_vertices + kContenders + hall_deficiency;
    const int right = chain_vertices + 1 + (hall_deficiency ? kContenders : 0);
    sirkit::BipartiteMatching matching(transpose ? right : left, transpose ? left : right);
    const auto add = [&](int u, int v) {
        if (transpose) matching.add_edge(v, u);
        else matching.add_edge(u, v);
    };
    const auto mate = [&](int u) {
        return transpose ? matching.match_right(u) : matching.match_left(u);
    };

    // ずれた対応から再開し、各鎖の最後の1頂点を増加させる。
    int shift = 0;
    for (const int length : chain_lengths) {
        for (int i = 0; i + 1 < length; ++i) add(shift + i + kContenders, shift + i + 2);
        shift += length;
    }
    assert(matching.solve() == chain_vertices - 1 - hall_deficiency);
    for (int i = 0; i < chain_vertices; ++i) add(i + kContenders, i + 1);

    // 別成分で平行辺を何度も走査して、再割当の予算切替を通す。
    for (int u = 0; u < kContenders; ++u)
        for (int copy = 0; copy < kParallel; ++copy)
            add(u, 0);
    // 反対向きの星を加えると、両側の非孤立頂点数では停止できない。
    if (hall_deficiency)
        for (int v = chain_vertices + 1; v < right; ++v) add(left - 1, v);
    const int expected = chain_vertices + 1 + hall_deficiency;
    assert(matching.solve() == expected);
    assert(matching.is_maximum());
    // 鎖の端から順に対応が強制され、残る星はちょうど1本を使う。
    for (int i = 0; i < chain_vertices; ++i) assert(mate(i + kContenders) == i + 1);
    int matched = 0;
    for (int u = 0; u < kContenders; ++u) {
        const int v = mate(u);
        assert(v == -1 || v == 0);
        matched += v == 0;
    }
    assert(matched == 1);
    if (hall_deficiency) assert(mate(left - 1) > chain_vertices);
    assert(matching.min_vertex_cover().size() == static_cast<std::size_t>(expected));
}

} // namespace

int main() {
    empty_sides_and_invalid_arguments();
    work_budget_boundaries();
    for (bool transpose : {false, true}) {
        for (bool hall_deficiency : {false, true})
            long_paths_with_contended_vertex(transpose, hall_deficiency);
        // 同じ長さの増加路を同じ探索段階で見つける。
        long_paths_with_contended_vertex(transpose, true, true);
    }
}
