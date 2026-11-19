// SPDX-License-Identifier: CC0-1.0
// AVX2搭載機でも疎索引の標準C++経路を検査する。
#ifndef SIRKIT_DISABLE_SIMD
#define SIRKIT_DISABLE_SIMD
#endif
#include "../../../DataStructure/dynamic-binary-indexed-tree-2d.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

void scalar_sparse_rectangles() {
    constexpr int kRows = 1'000'003;
    constexpr int kCols = 9'001;
    sirkit::DynamicFenwickTree2D<double> bit(kRows, kCols);
    std::map<std::pair<int, int>, int> quarters;
    // 離れた行と同じ行への更新を混ぜ、両軸の疎な表を再配置させる。
    for (int i = 0; i < 160; ++i) {
        const int row = (i * 6'227) % kRows;
        const int col = (i * 4'091) % kCols;
        const int value = i % 23 - 11;
        bit.add(row, col, value / 4.0);
        quarters[{row, col}] += value;
        bit.set(kRows - 1, col, (value + 1) / 4.0);
        quarters[{kRows - 1, col}] = value + 1;
    }
    const auto stored = bit.stored_nodes();
    bit.reserve(1'024);
    bit.reserve(1);
    assert(bit.stored_nodes() == stored);

    const auto check = [&](int top, int left, int bottom, int right) {
        int expected = 0;
        for (const auto& [point, value] : quarters) {
            const auto [row, col] = point;
            if (top <= row && row < bottom && left <= col && col < right) {
                expected += value;
            }
        }
        assert(bit.sum(top, left, bottom, right) == expected / 4.0);
        assert(bit.stored_nodes() == stored);
    };
    for (const int row : {1, kRows / 2, kRows - 1, kRows}) {
        for (const int col : {1, kCols / 2, kCols - 1, kCols}) {
            check(0, 0, row, col);
            check(row - 1, col - 1, row, col);
            check(row - 1, col - 1, kRows, kCols);
        }
    }
    for (const auto& [point, value] : quarters) {
        assert(bit.get(point.first, point.second) == value / 4.0);
    }
}

void bundled_wide_fenwick() {
    using Index = std::uint64_t;
    constexpr auto kSize = std::numeric_limits<Index>::max();
    sirkit::DynamicFenwickTree<Index, double> bit(kSize);
    assert(bit.size() == kSize && bit.get(kSize - 1) == 0.0);
    assert(bit.lower_bound(0.25) == kSize && bit.upper_bound(0.0) == kSize);
    constexpr std::array<Index, 4> kIndices{0, Index{1} << 31, Index{1} << 63, kSize - 1};
    constexpr std::array<int, 4> kQuarters{7, 0, 11, 3};
    for (std::size_t i = 0; i < kIndices.size(); ++i) {
        bit.set(kIndices[i], kQuarters[i] / 4.0);
    }
    const auto stored = bit.stored_nodes();
    for (int weight = -1; weight <= 22; ++weight) {
        int cumulative = 0;
        Index lower = weight <= 0 ? 0 : kSize;
        Index upper = weight < 0 ? 0 : kSize;
        for (std::size_t i = 0; i < kIndices.size(); ++i) {
            cumulative += kQuarters[i];
            assert(bit[kIndices[i]] == kQuarters[i] / 4.0);
            assert(bit.sum(kIndices[i] + 1) == cumulative / 4.0);
            if (lower == kSize && cumulative >= weight) lower = kIndices[i];
            if (upper == kSize && cumulative > weight) upper = kIndices[i];
        }
        assert(bit.lower_bound(weight / 4.0) == lower);
        assert(bit.upper_bound(weight / 4.0) == upper);
    }
    // 配列を確保できない予約要求の後も、更新済みの値と寸法を維持する。
    bool rejected = false;
    try {
        bit.reserve(std::numeric_limits<std::size_t>::max());
    } catch (const std::length_error&) {
        rejected = true;
    }
    assert(rejected);
    assert(bit.size() == kSize && bit.sum_all() == 21 / 4.0);
    assert(bit.stored_nodes() == stored);
    bit.add(kSize - 1, 0.25);
    assert(bit.get(kSize - 1) == 1.0 && bit.sum(kIndices[2], kSize) == 15 / 4.0);
}

void bundled_dense_fenwick() {
    using Index = std::uint64_t;
    sirkit::DynamicFenwickTree<Index, double> bit(63);
    for (const Index size : {63, 257}) {
        bit.reset(size);
        bit.reserve(32);
        assert(bit.get(0) == 0.0 && bit.sum(1, size) == 0.0);
        assert(bit.lower_bound(0.25) == size && bit.upper_bound(0.0) == size);
        std::vector<int> prefix(size + 1);
        for (Index pos = 0; pos < size; ++pos) {
            const int value = (pos + 3) % 7;
            prefix[pos + 1] = prefix[pos] + value;
            bit.add(pos, value / 4.0);
            assert(bit.get(pos) == value / 4.0);
            assert(bit.sum(pos + 1) == prefix[pos + 1] / 4.0);
        }
        for (Index pos = 0; pos < size; ++pos) {
            bit.add(pos, 1.0);
            prefix[pos + 1] += static_cast<int>(pos + 1) * 4;
        }
        auto copy = bit;
        bit.clear();
        bit = std::move(copy);
        assert(copy.empty());
        const auto stored = bit.stored_nodes();
        for (Index pos = 0; pos < size; ++pos) {
            const Index left = pos / 3;
            const Index right = pos + 1;
            assert(bit.sum(left, right) == (prefix[right] - prefix[left]) / 4.0);
            assert(bit.get(pos) == (prefix[right] - prefix[pos]) / 4.0);
        }
        for (int weight = 0; weight <= prefix.back() + 1; ++weight) {
            const auto begin = prefix.begin() + 1;
            const auto lower = std::lower_bound(begin, prefix.end(), weight);
            const auto upper = std::upper_bound(begin, prefix.end(), weight);
            assert(bit.lower_bound(weight / 4.0) == static_cast<Index>(lower - begin));
            assert(bit.upper_bound(weight / 4.0) == static_cast<Index>(upper - begin));
        }
#ifndef SKIP_RUNTIME_CHECKS
        const auto expect_out_of_range = [](auto&& action) {
            bool rejected = false;
            try {
                action();
            } catch (const std::out_of_range&) {
                rejected = true;
            }
            assert(rejected);
        };
        expect_out_of_range([&] { bit.add(size, 3.0); });
        expect_out_of_range([&] { (void)bit.sum(size + 1); });
        bool reversed = false;
        try {
            (void)bit.sum(size, 0);
        } catch (const std::invalid_argument&) {
            reversed = true;
        }
        assert(reversed);
#endif
        assert(bit.stored_nodes() == stored && bit.sum_all() == prefix.back() / 4.0);
    }
}

} // namespace

int main() {
    scalar_sparse_rectangles();
    bundled_wide_fenwick();
    bundled_dense_fenwick();
}
