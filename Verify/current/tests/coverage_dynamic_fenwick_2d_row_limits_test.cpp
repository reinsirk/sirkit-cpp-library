// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/dynamic-binary-indexed-tree-2d.cpp"
#include "internal_hash_capacity_checks.hpp"

#include <algorithm>
#include <array>
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

using Value = std::int64_t;
using Row = sirkit::detail::BPlusFenwickRow<Value>;
using Tree = sirkit::DynamicFenwickTree2D<Value>;
bool deny_huge_allocation;
int denied_allocations;

struct NodeCountMember {
    friend constexpr auto member_pointer(NodeCountMember);
};
template<class Tag, auto Member>
struct MemberAccess {
    friend constexpr auto member_pointer(Tag) { return Member; }
};
template struct MemberAccess<NodeCountMember, &Tree::add_nodes>;

} // namespace

void* operator new(std::size_t bytes) {
    if (deny_huge_allocation && bytes > (std::size_t{1} << 30)) {
        ++denied_allocations;
        throw std::bad_alloc();
    }
    if (void* memory = std::malloc(bytes ? bytes : 1)) return memory;
    throw std::bad_alloc();
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

namespace {

void rejects_reservation(Row& row, std::size_t count) {
    try {
        row.reserve_points(count);
    } catch (const std::length_error&) {
        return;
    }
    assert(false);
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

void check_sparse_rows() {
    constexpr std::uint32_t kWidth = 81920;
    Row row;
    assert(row.prefix(kWidth) == 0);
    for (std::uint32_t col = 1; col <= 96; ++col) row.add(2 * col, 1, kWidth);
    assert(row.prefix(kWidth) == 96 && row.prefix(119) == 59);
    assert(row.sum(10, 81) == 35 && row.sum(193, kWidth) == 0);

    if constexpr (sizeof(std::size_t) >= 8) {
        // 公開の予約ヒントで成長上限近くまで進み、実際の確保は失敗させる。
        deny_huge_allocation = true;
        bool rejected = false;
        try {
            row.reserve_points(static_cast<std::size_t>(std::uint64_t{1} << 35));
        } catch (const std::bad_alloc&) {
            rejected = true;
        }
        deny_huge_allocation = false;
        assert(rejected && denied_allocations == 1);
        assert(row.prefix(kWidth) == 96 && row.sum(10, 81) == 35);
    }
}

void check_fallback_rows() {
    using Fallback = sirkit::detail::FenwickFallback<int, WideValue>;
    Fallback row(700'000);
    const WideValue first{{1, -3, 5, -7}}, last{{17, 19, -23, -29}};
    row.add(5, first);
    row.add(row.size() - 1, last);
    assert(row.sum(5, 6) == first);
    assert(row.sum(6, row.size()) == last);
#ifndef SKIP_RUNTIME_CHECKS
    bool rejected = false;
    try {
        row.reset(-1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected && row.sum(6, row.size()) == last);
#endif
}

void check_contiguous_append(std::uint32_t initial_prefix, bool complete) {
    constexpr std::uint32_t kWidth = 130;
    sirkit::detail::BPlusFenwickRow<Value, true> row;
    std::vector<Value> values(kWidth);
    const auto add = [&](std::uint32_t column, Value delta) {
        row.add(column, delta, kWidth);
        values[column - 1] += delta;
    };
    const auto check = [&] {
        std::vector<Value> prefixes(kWidth + 1);
        std::partial_sum(values.begin(), values.end(), prefixes.begin() + 1);
        for (std::uint32_t i = 0; i <= kWidth; ++i) {
            assert(row.prefix(i) == prefixes[i]);
            const auto end = std::min(kWidth, i + 31);
            assert(row.sum(i, end) == prefixes[end] - prefixes[i]);
            if (i > 0) assert(row.get(i) == values[i - 1]);
        }
        for (const Value target : {Value{1}, prefixes.back() / 2, prefixes.back(), prefixes.back() + 1}) {
            const auto begin = prefixes.begin() + 1;
            assert(row.bound<false>(target, kWidth) == std::lower_bound(begin, prefixes.end(), target) - begin);
            assert(row.bound<true>(target, kWidth) == std::upper_bound(begin, prefixes.end(), target) - begin);
        }
    };

    for (std::uint32_t col = 1; col <= initial_prefix; ++col) add(col, col % 5 + 1);
    assert(row.direct_values() != nullptr);
    row.promote_prefix(kWidth, initial_prefix);
    assert(row.direct_values() == nullptr);
    row.reserve(1, kWidth);
    check();
    row.promote_prefix(kWidth, initial_prefix);
    check();
    if (complete) {
        for (std::uint32_t col = initial_prefix + 1; col <= kWidth; ++col) {
            add(col, col % 5 + 1);
            check();
        }
        row.promote_prefix(kWidth, kWidth);
        assert(row.direct_values() != nullptr);
        check();
    } else {
        // 順番外の更新で連続追加を終える。占有済みと未占有のセルを続けて更新する。
        add(40, 3);
        check();
        add(97, 7);
        check();
    }

    row.clear();
    assert(row.direct_values() == nullptr);
    std::fill(values.begin(), values.end(), 0);
    check();
    add(3, 17);
    add(3, 5);
    check();
    for (std::uint32_t col = 1; col <= kWidth; ++col) add(col, 1);
    assert(row.direct_values() != nullptr);
    check();
}

void check_node_count_limit() {
    const auto add = member_pointer(NodeCountMember{});
    constexpr auto maximum = std::numeric_limits<std::size_t>::max();
    std::size_t count = maximum - 1;
    add(count, 1);
    assert(count == maximum);
    bool rejected = false;
    try {
        add(count, 1);
    } catch (const std::length_error&) {
        rejected = true;
    }
    assert(rejected && count == maximum);
}

} // namespace

int main() {
    sirkit_internal_hash_capacity_test::run();
    constexpr std::uint32_t kWidth = 257;
    Row row;
    const auto maximum = std::numeric_limits<std::size_t>::max();
    rejects_reservation(row, maximum);
    if constexpr (sizeof(std::size_t) > 4) rejects_reservation(row, maximum - 15);
    assert(row.empty() && row.sum(0, kWidth) == 0);
    row.add(2, 7, kWidth);
    assert(row.sum(0, 2) == 7 && row.sum(2, 2) == 0);
    row.promote_prefix(kWidth);
    row.promote_prefix(kWidth);
    assert(row.sum(2, 2) == 0);
    assert(row.sum(0, kWidth) == 7);
    for (std::uint32_t col = 1; col <= kWidth; ++col) row.add(col, 1, kWidth);
    assert(row.sum(0, kWidth) == kWidth + 7);
    rejects_reservation(row, maximum);
    assert(row.sum(1, 2) == 8 && row.sum(2, kWidth) == kWidth - 2);

    sirkit::detail::BPlusFenwickRow<Value, true> direct;
    assert(direct.direct_values() == nullptr);
    direct.reserve(0, kWidth);
    direct.reserve(9, kWidth);
    direct.add(2, 7, kWidth);
    direct.promote_prefix(kWidth);
    assert(direct.sum(0, kWidth) == 7 && direct.sum(2, kWidth) == 0);
    assert(row.prefix(kWidth) == kWidth + 7);
    check_contiguous_append(63, true);
    check_contiguous_append(63, false);
    check_contiguous_append(64, false);
    constexpr std::uint32_t kSparseWidth = 1'000'001;
    sirkit::detail::BPlusFenwickRow<Value, true> sparse;
    sparse.add(1, 7, kSparseWidth);
    sparse.promote_prefix(kSparseWidth, 1);
    assert(sparse.prefix(kSparseWidth) == 7 && sparse.sum(0, 1) == 7 && sparse.get(1) == 7);
    check_sparse_rows();
    {
        // 密配列に切り替わらない幅で、親と葉をまたぐ累積値の境界を調べる。
        sirkit::detail::BPlusFenwickRow<Value, true> indexed;
        std::vector<Value> sums;
        Value total = 0;
        for (std::uint32_t i = 1; i <= 96; ++i) {
            const Value value = i % 5 + 1;
            indexed.add(3 * i, value, kSparseWidth);
            total += value;
            sums.push_back(total);
        }
        assert(indexed.bound<true>(-1, kSparseWidth) == 0);
        const auto position = [&](auto iterator) {
            return iterator == sums.end() ? kSparseWidth
                : 3 * static_cast<std::uint32_t>(iterator - sums.begin()) + 2;
        };
        for (Value target = 1; target <= total + 1; ++target) {
            assert(indexed.bound<false>(target, kSparseWidth) ==
                   position(std::lower_bound(sums.begin(), sums.end(), target)));
            assert(indexed.bound<true>(target, kSparseWidth) ==
                   position(std::upper_bound(sums.begin(), sums.end(), target)));
        }
    }
    check_fallback_rows();
    check_node_count_limit();

    sirkit::detail::InternalHashMap<int, int> empty;
    bool visited = false;
    empty.for_each([&](int, int) { visited = true; });
    assert(!visited);
}
