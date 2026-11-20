// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/dynamic-binary-indexed-tree.cpp"
#include "internal_hash_capacity_checks.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace {

std::size_t blocked_bytes;
int blocked_allocations;
bool fail_large_allocation;

} // namespace

void* operator new(std::size_t bytes) {
    if ((blocked_bytes != 0 && bytes == blocked_bytes) ||
        (fail_large_allocation && bytes > (std::size_t{1} << 30))) {
        ++blocked_allocations;
        throw std::bad_alloc();
    }
    if (void* storage = std::malloc(bytes ? bytes : 1)) return storage;
    throw std::bad_alloc();
}
void operator delete(void* storage) noexcept { std::free(storage); }
void operator delete(void* storage, std::size_t) noexcept { std::free(storage); }

namespace {

using Value = std::int64_t;
using Tree = sirkit::DynamicFenwickTree<int, Value>;
using IndexedRow = sirkit::detail::SmallFenwickRow<Value, true>;

struct IndexSaltMember {
    friend constexpr auto member_pointer(IndexSaltMember);
};
template<class Tag, auto Member>
struct MemberAccess {
    friend constexpr auto member_pointer(Tag) { return Member; }
};
template struct MemberAccess<IndexSaltMember, &IndexedRow::initial_index_salt>;

void check(const Tree& tree, const std::vector<Value>& values, int stride) {
    const auto size = static_cast<int>(values.size());
    std::vector<Value> sums(values.size() + 1);
    std::partial_sum(values.begin(), values.end(), sums.begin() + 1);
    assert(tree.sum_all() == sums.back());
    for (int i = 0; i < size; i += stride) {
        const int end = std::min(size, i + 85);
        assert(tree.get(i) == values[i]);
        assert(tree.sum(i) == sums[i]);
        assert(tree.sum(i, end) == sums[end] - sums[i]);
        assert(tree.sum(i, i) == 0);
    }
    for (const auto target : {Value{-1}, Value{0}, Value{1}, sums.back() / 2,
                              sums.back(), sums.back() + 1}) {
        assert(tree.lower_bound(target) == std::lower_bound(sums.begin() + 1, sums.end(), target) -
                                               (sums.begin() + 1));
        assert(tree.upper_bound(target) == std::upper_bound(sums.begin() + 1, sums.end(), target) -
                                               (sums.begin() + 1));
    }
}

void check_storage_transitions() {
    constexpr int kSize = 8193;
    Tree tree(kSize);
    std::vector<Value> values(kSize);
    tree.reserve(0);
    for (const int i : {101, 103, 105, 107, 109, 111, 113, 115}) {
        tree.add(i, i);
        values[i] = i;
        assert(tree.sum(i, i + 1) == i);
    }
    tree.reserve(32);
    assert(tree.sum(109, 110) == 109);
    tree.clear();
    tree.reserve(800);
    assert(tree.get(0) == 0 && tree.lower_bound(1) == kSize && tree.upper_bound(0) == kSize);
    assert(tree.sum(17, 212) == 0);
    std::fill(values.begin(), values.end(), 0);

    // 密配列に必要な確保だけを失敗させ、疎な状態で更新と照会を続ける。
    blocked_bytes = (static_cast<std::size_t>(std::bit_ceil(unsigned{kSize})) + 1) * sizeof(Value);
    for (int j = 0; j < 2500; ++j) {
        const int i = (j * 4051 + 31) % kSize;
        const Value value = j % 13;
        tree.add(i, value);
        values[i] += value;
    }
    blocked_bytes = 0;
    assert(blocked_allocations == 1);
    check(tree, values, 23);
    tree.clear();
    std::fill(values.begin(), values.end(), 0);
    for (int j = 0; j < 2500; ++j) {
        const int i = (j * 4051 + 31) % kSize;
        tree.add(i, 1);
        values[i] += 1;
    }
    check(tree, values, 19);
}

void check_large_prefix() {
    constexpr int kSize = 1'000'001;
    Tree tree(kSize);
    std::vector<Value> values(kSize);
    for (int i = 0; i < 35'000; ++i) {
        tree.add(i, i % 11);
        values[i] = i % 11;
    }
    tree.set(kSize - 1, 93);
    values.back() = 93;
    check(tree, values, 317);
}

struct WideValue {
    std::array<Value, 4> components{};
    WideValue& operator+=(const WideValue& other) noexcept {
        for (std::size_t i = 0; i < components.size(); ++i) components[i] += other.components[i];
        return *this;
    }
    WideValue& operator-=(const WideValue& other) noexcept {
        for (std::size_t i = 0; i < components.size(); ++i) components[i] -= other.components[i];
        return *this;
    }
    bool operator==(const WideValue&) const = default;
};

void check_wide_values() {
    using WideTree = sirkit::DynamicFenwickTree<int, WideValue>;
    constexpr int kSize = 700'000;
    WideTree tree(kSize);
    tree.reserve(20);
    const WideValue first{{1, -3, 5, -7}}, last{{17, 19, -23, -29}};
    tree.add(5, first);
    tree.add(kSize - 1, last);
    WideValue both = first;
    both += last;
    assert(tree.sum_all() == both);
    assert(tree.sum(5, 6) == first);
    assert(tree.sum(6, kSize) == last);
    assert(tree.get(6) == WideValue{});
    tree.clear();
    assert(tree.sum_all() == WideValue{});
#ifndef SKIP_RUNTIME_CHECKS
    try {
        (void)WideTree(-1);
        assert(false);
    } catch (const std::invalid_argument&) {
    }
#endif
}

void check_direct_row() {
    constexpr std::uint32_t kWidth = 136;
    using Row = sirkit::detail::BPlusFenwickRow<Value, true>;
    Row row;
    const auto maximum = std::numeric_limits<std::size_t>::max();
    for (const auto points : {maximum, maximum - 15}) {
        if constexpr (sizeof(std::size_t) <= 4) if (points != maximum) continue;
        try {
            row.reserve_points(points);
            assert(false);
        } catch (const std::length_error&) {
        }
    }
    assert(row.empty());
    row.reserve(0, kWidth);
    row.add(1, 3, kWidth);
    if constexpr (sizeof(std::size_t) >= 8) {
        // 上限の半分を超える予約を要求し、確保自体は失敗させて既存の木を照合する。
        const int before = blocked_allocations;
        fail_large_allocation = true;
        try {
            row.reserve_points(static_cast<std::size_t>(std::uint64_t{1} << 35));
            assert(false);
        } catch (const std::bad_alloc&) {
        }
        fail_large_allocation = false;
        assert(blocked_allocations == before + 1);
        assert(row.get(1) == 3 && row.prefix(kWidth) == 3);
    }
    assert(row.bound<false>(0, kWidth) == 0);
    assert(row.bound<true>(-1, kWidth) == 0);
    assert(row.sum(2, 2) == 0);

    // 一次元の連続追加への移行に失敗しても、更新済みの値を保つ。
    const int before = blocked_allocations;
    blocked_bytes = (static_cast<std::size_t>(std::bit_ceil(kWidth)) + 1) * sizeof(Value);
    row.promote_prefix(kWidth, 1);
    blocked_bytes = 0;
    assert(blocked_allocations == before + 1);
    assert(row.get(1) == 3 && row.prefix(kWidth) == 3);
    row.clear();
    row.add(2, 7, kWidth);
    row.promote_prefix(kWidth);
    assert(row.get(2) == 7 && row.prefix(kWidth) == 7);
    assert(row.sum(2, 2) == 0 && row.sum(0, kWidth) == 7);
    assert(row.bound<false>(7, kWidth) == 1 && row.bound<true>(7, kWidth) == kWidth);
    for (std::uint32_t col = 1; col <= kWidth; ++col) row.add(col, 1, kWidth);
    row.promote_prefix(kWidth, kWidth);
    assert(row.get(2) == 8 && row.prefix(kWidth) == kWidth + 7);
}

void check_small_row_states() {
    using Row = sirkit::detail::SmallFenwickRow<Value, true>;
    constexpr std::uint32_t kShort = 250;
    constexpr std::uint32_t kLong = 8193;
    Row row;
    row.clear();
    row.reserve(0, kShort);
    row.reserve(17, 0);
    assert(row.size_mode<false>() == 0);
    assert(row.bound<false>(0, kShort) == 0 && row.bound<true>(-1, kShort) == 0);
    row.add(19, 3, kShort);
    assert(row.get(19) == 3);
    row.clear();
    row.reserve(100, kShort);
    std::vector<Value> cells;
    std::vector<std::uint8_t> used;
    row.export_cells(kShort, cells, used);
    assert(std::ranges::count(used, 1) == 0);
    auto* hash = row.fenwick_map();
    assert(hash != nullptr);
    assert(hash->enumerate().empty() && hash->count(13) == 0);
    row.add(13, 7, kShort);
    assert(hash->count(13) == 1 && row.get(13) == 7);
    assert(row.size_mode<false>() == row.size());
    row.clear();
    assert(hash->empty() && row.get(13) == 0);
    row.clear(kLong);
    assert(row.empty());
    row.add(kLong, 4, kLong);
    assert(row.get(kLong) == 4);
    row.clear();
    row.reserve(80, kLong);
    row.add(5, 17, kLong);
    assert(row.get(5) == 17);
    row.clear();
    assert(row.empty() && row.get(5) == 0);
    row.reserve(800, kLong);
    row.add(17, 33, kLong);
    assert(row.get(17) == 33);
    row.clear();
    assert(row.empty() && row.get(17) == 0);
}

void check_index_reserve_collision() {
    constexpr std::uint32_t kWidth = 100;
    constexpr std::size_t kBuckets = 16;
    const auto salt = member_pointer(IndexSaltMember{})();
    std::array<std::uint32_t, kBuckets> first{};
    std::array<std::uint32_t, 5> columns{};
    // 新しい16枠に対して17列を調べ、ソルトにかかわらず衝突する2点を選ぶ。
    for (std::uint32_t col = 1; col <= kBuckets + 1; ++col) {
        const auto bucket = static_cast<std::uint32_t>(col * salt) >> 28;
        if (first[bucket]) {
            columns = {first[bucket], col, 21, 22, 23};
            break;
        }
        first[bucket] = col;
    }
    assert(columns[0] != 0);
    IndexedRow row;
    Value expected = 0;
    for (const auto col : columns) {
        row.add(col, col, kWidth);
        expected += col;
    }
    row.reserve(10, kWidth);
    for (const auto col : columns) assert(row.get(col) == col);
    assert(row.prefix(kWidth) == expected);
}

} // namespace

int main() {
    sirkit_internal_hash_capacity_test::run();
    check_storage_transitions();
    check_large_prefix();
    check_wide_values();
    check_direct_row();
    check_small_row_states();
    check_index_reserve_collision();
}
