// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_FENWICK_SOURCE
#define SIRKIT_TEST_FENWICK_SOURCE "../../../DataStructure/dynamic-binary-indexed-tree.cpp"
#endif
#include SIRKIT_TEST_FENWICK_SOURCE

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>
#include <numeric>
#include <utility>
#include <vector>

namespace {
std::size_t blocked_bytes = 0;
int blocked_calls = 0;
}

void* operator new(std::size_t bytes) {
    if (bytes == blocked_bytes && blocked_bytes != 0) {
        ++blocked_calls;
        throw std::bad_alloc();
    }
    if (void* memory = std::malloc(bytes ? bytes : 1)) return memory;
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

namespace {

void check_failed_promotion() {
    constexpr int kSize = 257;
    sirkit::DynamicFenwickTree<int, double> tree(kSize);
    std::array<int, kSize> halves{};
    tree.reserve(kSize);
    tree.reserve(0);
    // 直接配列の確保だけを失敗させ、加算済みの値は疎な表に残る。
    blocked_bytes = (kSize + 1) * sizeof(double);
    for (int pos = 0; pos < kSize; ++pos) {
        halves[pos] = pos % 7;
        tree.add(pos, halves[pos] / 2.0);
    }
    blocked_bytes = 0;
    assert(blocked_calls == 1);
    const double total = std::accumulate(halves.begin(), halves.end(), 0) / 2.0;
    auto& alias = tree;
    tree = std::move(alias);
    assert(tree.sum_all() == total && tree.stored_nodes() == kSize);
    assert(tree.sum(135, 135) == 0.0);
    for (int pos = 0; pos < kSize; ++pos) {
        assert(tree.get(pos) == halves[pos] / 2.0);
        assert(tree.sum(pos, kSize) == std::accumulate(halves.begin() + pos, halves.end(), 0) / 2.0);
    }

    // clearのたびに再び密化でき、同じ加算列から同じ値が得られる。
    tree.clear();
    assert(tree.sum_all() == 0.0 && tree.stored_nodes() == 0);
    for (int pos = kSize - 1; pos >= 0; --pos) tree.set(pos, halves[pos] / 2.0);
    assert(tree.sum_all() == total && tree.stored_nodes() == kSize);
    for (int pos = 0; pos < kSize; ++pos) assert(tree.get(pos) == halves[pos] / 2.0);
}

#if !defined(SIRKIT_TEST_FENWICK_2D) || defined(SIRKIT_TEST_FENWICK_SHARED)
void check_wide_reset() {
    using Index = std::uint64_t;
    constexpr Index kInitialSize = Index{1} << 35;
    constexpr Index kFinalSize = std::numeric_limits<Index>::max();
    sirkit::DynamicFenwickTree<Index, std::int64_t> tree(kInitialSize);
    tree.add(kInitialSize - 1, 7);
    assert(tree.sum_all() == 7);
    // 保持領域が既にある場合にも、新しい長さに更新してから読み書きできる。
    tree.reset(kFinalSize);
    assert(tree.size() == kFinalSize && tree.stored_nodes() == 0);
    tree.set(kFinalSize - 1, 3);
    tree.set(0, 2);
    assert(tree.sum_all() == 5 && tree.sum(1, kFinalSize) == 3);
    assert(tree.lower_bound(3) == kFinalSize - 1 && tree.upper_bound(5) == kFinalSize);
    tree.reset(kInitialSize);
    assert(tree.size() == kInitialSize && tree.stored_nodes() == 0 && tree.sum_all() == 0);
}

void check_width_limit() {
    constexpr int kSize = 1'000'001;
    sirkit::DynamicFenwickTree<int, std::int64_t> tree(kSize);
    std::array<int, 260> values{};
    for (int pos = 0; pos < static_cast<int>(values.size()); ++pos) {
        values[pos] = (pos + 1) % 9;
        tree.add(pos, values[pos]);
    }
    assert(tree.sum_all() == std::accumulate(values.begin(), values.end(), 0));
    assert(tree.get(kSize - 1) == 0);
}
#endif

#ifdef SIRKIT_TEST_FENWICK_2D
void check_row_reuse() {
    constexpr int kColumns = 8192;
    sirkit::DynamicFenwickTree2D<std::int64_t> tree(1, kColumns);
    std::vector<std::int64_t> values(kColumns);
    for (int i = 0; i < kColumns; ++i) {
        const auto col = (i * 17) % kColumns;
        values[col] = col % 13 - 6;
        tree.add(0, col, values[col]);
    }
    assert(tree.stored_nodes() == kColumns);
    std::vector<std::int64_t> prefix(kColumns + 1);
    std::partial_sum(values.begin(), values.end(), prefix.begin() + 1);
    for (int col = 0; col < kColumns; ++col) {
        assert(tree.get(0, col) == values[col]);
        assert(tree.sum(0, col, 1, kColumns) == prefix.back() - prefix[col]);
    }
    tree.clear();
    assert(tree.stored_nodes() == 0 && tree.sum_all() == 0);
    tree.set(0, kColumns - 1, 7);
    tree.set(0, 2, -4);
    assert(tree.get(0, kColumns - 1) == 7);
    assert(tree.sum(1, kColumns - 1) == -4 && tree.sum_all() == 3);
}
#endif
} // namespace

int main() {
    check_failed_promotion();
#if !defined(SIRKIT_TEST_FENWICK_2D) || defined(SIRKIT_TEST_FENWICK_SHARED)
    check_wide_reset();
    check_width_limit();
#endif
#ifdef SIRKIT_TEST_FENWICK_2D
    check_row_reuse();
#endif
}
