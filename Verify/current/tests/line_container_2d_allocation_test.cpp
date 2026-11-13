// SPDX-License-Identifier: CC0-1.0
#include <cstdlib>
#include <array>
#include <new>
#include <cstddef>
#include <limits>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <utility>

namespace allocation_test {
std::int64_t fail_after = -1;
std::size_t live = 0, attempts = 0, failures = 0;
void* allocate(std::size_t size, std::size_t alignment) {
    ++attempts;
    if (fail_after >= 0) {
        if (fail_after == 0) { ++failures; throw std::bad_alloc(); }
        --fail_after;
    }
    void* result;
    if (alignment <= alignof(std::max_align_t)) result = std::malloc(size ? size : 1);
    else {
        if (size > std::numeric_limits<std::size_t>::max() - alignment + 1) throw std::bad_alloc();
        const auto rounded = (size + alignment - 1) / alignment * alignment;
        result = std::aligned_alloc(alignment, rounded ? rounded : alignment);
    }
    if (!result) throw std::bad_alloc();
    ++live;
    return result;
}
void release(void* pointer) noexcept {
    if (pointer) { assert(live != 0); --live; std::free(pointer); }
}
}
void* operator new(std::size_t size) { return allocation_test::allocate(size, alignof(std::max_align_t)); }
void* operator new[](std::size_t size) { return allocation_test::allocate(size, alignof(std::max_align_t)); }
void operator delete(void* pointer) noexcept { allocation_test::release(pointer); }
void operator delete[](void* pointer) noexcept { allocation_test::release(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { allocation_test::release(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { allocation_test::release(pointer); }
void* operator new(std::size_t size, std::align_val_t alignment) { return allocation_test::allocate(size, std::size_t(alignment)); }
void* operator new[](std::size_t size, std::align_val_t alignment) { return allocation_test::allocate(size, std::size_t(alignment)); }
void operator delete(void* pointer, std::align_val_t) noexcept { allocation_test::release(pointer); }
void operator delete[](void* pointer, std::align_val_t) noexcept { allocation_test::release(pointer); }
void operator delete(void* pointer, std::size_t, std::align_val_t) noexcept { allocation_test::release(pointer); }
void operator delete[](void* pointer, std::size_t, std::align_val_t) noexcept { allocation_test::release(pointer); }
#ifndef HEADER
#define HEADER "../../../DataStructure/line-container-2d.cpp"
#endif
#include HEADER
using Point = std::pair<std::int64_t, std::int64_t>;
using Points = std::vector<Point>;
std::uint64_t checks = 0;
void verify(const LineContainer2D& tree, const Points& points) {
    assert(tree.size() == points.size());
    if (points.empty()) {
        assert(tree.empty()); assert(!tree.min_optional(1, 1)); checks += 3; return;
    }
    for (auto [a, b] : Points{{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {3, 7}, {-11, 5}}) {
        auto lo = std::numeric_limits<std::int64_t>::max();
        auto hi = std::numeric_limits<std::int64_t>::min();
        for (auto [x, y] : points) { lo = std::min(lo, a * x + b * y); hi = std::max(hi, a * x + b * y); }
        assert(tree.min_ld(a, b) == lo && tree.max_ld(a, b) == hi); checks += 2;
    }
    ++checks;
}
int main() {
    const auto initial_live = allocation_test::live;
    std::size_t copy_failures = 0, copy_successes = 0, replacement_attempts = 0;
    // min側の同傾き置換を経て、次の追加が両包絡を変える状態を作る。
    // 一方の包絡の更新後、もう一方の確保で失敗する条件も含める。
    for (std::int64_t failure = 0; failure < 4; ++failure) {
        {
            LineContainer2D tree; tree.add(0, 0); tree.add(-1, 0);
            allocation_test::fail_after = failure;
            bool failed = false;
            try { tree.add(0, 1); }
            catch (const std::bad_alloc&) { failed = true; }
            allocation_test::fail_after = -1;
            if (failed) verify(tree, {});
            else verify(tree, {{0, 0}, {-1, 0}, {0, 1}});
            tree.clear(); verify(tree, {});
            tree.add(7, -3); verify(tree, {{7, -3}});
        }
        assert(allocation_test::live == initial_live); ++checks;
    }
    // 挿入時に一切確保しない場合も、APIの結果と破棄を検証する。
    {
        LineContainer2D tree; tree.add(0, 0); tree.add(-1, 0);
        const auto before = allocation_test::attempts;
        for (int i = 2; i <= 64; ++i) tree.add(-i, 0);
        replacement_attempts = allocation_test::attempts - before;
        Points points;
        for (std::int64_t i = 0; i <= 64; ++i) points.push_back({-i, 0});
        verify(tree, points);
    }
    assert(allocation_test::live == initial_live);
    for (std::int64_t failure : std::array<std::int64_t, 9>{0, 1, 2, 3, 4, 8, 16, 32, 1024}) {
        {
            LineContainer2D source, target; Points source_points, target_points{{0, 0}, {-1, 0}};
            for (std::int64_t i = -24; i <= 24; ++i) { source.add(i, i * i); source_points.push_back({i, i * i}); }
            target.add(0, 0); target.add(-1, 0);
            allocation_test::fail_after = failure;
            bool failed = false;
            try { target = source; }
            catch (const std::bad_alloc&) { failed = true; }
            allocation_test::fail_after = -1;
            verify(source, source_points);
            if (failed) { verify(target, target_points); ++copy_failures; }
            else { verify(target, source_points); target_points = source_points; ++copy_successes; }
            target.add(5, -9); target_points.push_back({5, -9}); verify(target, target_points);
        }
        assert(allocation_test::live == initial_live); ++checks;
    }
    assert(copy_failures != 0 && copy_successes != 0);
    {
        LineContainer2D source, target;
        source.add(0, 0); source.add(-1, 0); source.add(1, 0);
        target.add(9, 7); target.add(10, 7);
        const auto before = allocation_test::attempts;
        allocation_test::fail_after = 0;
        target = std::move(source);
        LineContainer2D moved(std::move(target));
        source.swap(moved);
        allocation_test::fail_after = -1;
        assert(allocation_test::attempts == before);
        verify(target, {}); verify(moved, {}); verify(source, {{0, 0}, {-1, 0}, {1, 0}});
        source.clear(); source.add(3, 5); verify(source, {{3, 5}});
    }
    assert(allocation_test::live == initial_live);
    std::cout << checks << " allocation/rollback/ownership checks, " << copy_failures << " copy failures, "
              << copy_successes << " successful copies, " << replacement_attempts << " replacement allocations\n";
}
