// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_FENWICK_MOVE_SOURCE
#define SIRKIT_TEST_FENWICK_MOVE_SOURCE "../../../DataStructure/dynamic-binary-indexed-tree.cpp"
#endif
#include SIRKIT_TEST_FENWICK_MOVE_SOURCE

#include <cassert>
#include <cstdint>
#include <map>
#include <utility>

namespace {

template<class Tree, class Values>
void check_prefix_search(const Tree& tree, const Values& values, double threshold) {
    int lower = threshold <= 0 ? 0 : tree.size();
    int upper = threshold < 0 ? 0 : tree.size();
    double sum = 0;
    for (const auto& [position, value] : values) {
        sum += value;
        if (lower == tree.size() && sum >= threshold) lower = position;
        if (upper == tree.size() && sum > threshold) upper = position;
    }
    assert(tree.lower_bound(threshold) == lower);
    assert(tree.upper_bound(threshold) == upper);
}

void check_sparse_move_assignment() {
    using Tree = sirkit::DynamicFenwickTree<int, double>;
    constexpr int kSize = 1'000'003;
    const std::map<int, double> values{{0, 2.0}, {31, 0.5}, {1024, 1.0}, {kSize - 2, 2.0}};
    Tree source(kSize), target(17);
    for (const auto& [pos, value] : values) source.add(pos, value);
    target.add(3, 99.0);
    target = std::move(source);
    assert(source.empty() && source.stored_nodes() == 0);
    assert(target.size() == kSize);
    for (double threshold : {-1.0, 0.0, 0.5, 2.0, 2.5, 3.0, 3.5, 5.5, 6.0})
        check_prefix_search(target, values, threshold);
    for (int first : {0, 1, 31, 500, kSize - 2, kSize - 1}) {
        double expected = 0.0;
        for (const auto& [pos, value] : values)
            if (first <= pos) expected += value;
        assert(target.sum(first, kSize) == expected);
    }
    source.reset(9);
    source.add(8, 7.0);
    assert(source.get(8) == 7.0 && target.sum_all() == 5.5);
}

#ifdef SIRKIT_TEST_FENWICK_MOVE_2D
void check_recycling_across_widths() {
    using Point = std::pair<int, int>;
    constexpr int kRows = 1'000'003;
    sirkit::DynamicFenwickTree2D<std::int64_t> tree(kRows, 3001);
    // 既存の行を使い、同種の幅変更と異種の幅変更を通す。
    for (int width : {3001, 4001, 9001, 8192, 3101}) {
        tree.reset(kRows, width);
        assert(tree.stored_nodes() == 0 && tree.sum_all() == 0);
        std::map<Point, std::int64_t> values;
        for (int row : {kRows - 4, kRows - 2}) {
            for (int i = 0; i < 140; ++i) {
                const int col = (i * 131 + 11) % width;
                const int value = i % 11 - 5;
                tree.add(row, col, value);
                values[{row, col}] += value;
            }
        }
        tree.set(kRows - 4, width - 1, 91);
        values[{kRows - 4, width - 1}] = 91;
        for (const auto& [point, value] : values) assert(tree.get(point.first, point.second) == value);
        assert(tree.get(kRows - 3, width - 1) == 0);
        for (int first_row : {kRows - 5, kRows - 3, kRows - 1})
            for (int first_col : {0, 13, width / 2, width - 1}) {
                std::int64_t expected = 0;
                for (const auto& [point, value] : values)
                    if (point.first >= first_row && point.second >= first_col) expected += value;
                assert(tree.sum(first_row, first_col, kRows, width) == expected);
            }
    }
}
#else
void check_compact_move_assignment() {
    constexpr int kSize = 100003;
    using Tree = sirkit::DynamicFenwickTree<int, std::int64_t>;
    Tree source(kSize), target(kSize);
    std::map<int, std::int64_t> values;
    for (int i = 0; i < 80; ++i) {
        const int pos = 11 + i * 701;
        source.add(pos, i % 5);
        target.add(2 + i * 301, 41);
        values[pos] = i % 5;
    }
    target = std::move(source);
    assert(source.empty() && target.size() == kSize);
    std::int64_t sum = 0;
    for (const auto& [position, value] : values) {
        assert(target.sum(position) == sum && target.get(position) == value);
        sum += value;
    }
    assert(target.sum_all() == sum && target.get(kSize - 1) == 0);
}
#endif

} // namespace

int main() {
    check_sparse_move_assignment();
#ifdef SIRKIT_TEST_FENWICK_MOVE_2D
    check_recycling_across_widths();
#else
    check_compact_move_assignment();
#endif
}
