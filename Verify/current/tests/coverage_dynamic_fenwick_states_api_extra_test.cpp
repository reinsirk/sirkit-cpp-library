// SPDX-License-Identifier: CC0-1.0
// ハッシュ表の標準C++再配置を、独立した配列と照合する。
#ifndef SIRKIT_DISABLE_SIMD
#define SIRKIT_DISABLE_SIMD
#endif
#include "../../../DataStructure/dynamic-binary-indexed-tree.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
template<class Error>
bool rejects(auto operation) {
    try {
        operation();
    } catch (const Error&) {
        return true;
    }
    return false;
}

template<class S, class T>
void check(const sirkit::DynamicFenwickTree<S, T>& tree, const std::vector<T>& values) {
    const auto size = values.size();
    assert(tree.size() == static_cast<S>(size));
    std::vector<T> prefix(size + 1);
    std::partial_sum(values.begin(), values.end(), prefix.begin() + 1);
    assert(tree.sum_all() == prefix.back());
    const auto stored = tree.stored_nodes();
    for (std::size_t i = 0; i < size; ++i) {
        assert(tree[static_cast<S>(i)] == values[i]);
        assert(tree.sum(static_cast<S>(i)) == prefix[i]);
        const auto r = std::min(size, i + 9);
        assert(tree.sum(static_cast<S>(i), static_cast<S>(r)) == prefix[r] - prefix[i]);
        assert(tree.sum(static_cast<S>(i), static_cast<S>(i + 1)) == values[i]);
    }
    for (const T threshold : {T{-1}, T{0}, T{1}, prefix.back() / T{2}, prefix.back(), prefix.back() + T{1}}) {
        const auto first = prefix.begin() + 1;
        assert(tree.lower_bound(threshold) == static_cast<S>(std::lower_bound(first, prefix.end(), threshold) - first));
        assert(tree.upper_bound(threshold) == static_cast<S>(std::upper_bound(first, prefix.end(), threshold) - first));
    }
    assert(tree.stored_nodes() == stored);
}

void check_fallback() {
    // 浮動型でも二進数で正確な0.5の整数倍だけを加算する。
    constexpr int kSize = 513;
    using Tree = sirkit::DynamicFenwickTree<int, double>;
    Tree tree(kSize);
    std::vector<double> values(kSize);
    tree.reserve(0);
    tree.reserve(100);
    check(tree, values);
    for (int i = 0; i < 16; ++i) {
        const int position = i * 29;
        values[position] = (i + 1) * 0.5;
        tree.set(position, values[position]);
    }
    const auto stored = tree.stored_nodes();
    tree.reserve(300);
    assert(tree.stored_nodes() == stored);
    check(tree, values);

    // 全セルを登録した後も、値の変更と検索が一致する。
    for (int i = 0; i < kSize; ++i) {
        tree.add(i, 0.5);
        values[i] += 0.5;
    }
    assert(tree.stored_nodes() == values.size());
    tree.add(0, 1.5);
    values[0] += 1.5;
    tree.reserve(800);
    check(tree, values);

#ifndef SKIP_RUNTIME_CHECKS
    assert(rejects<std::invalid_argument>([] { (void)Tree(-1); }));
    assert(rejects<std::invalid_argument>([&] { (void)tree.sum(3, 1); }));
    assert(rejects<std::out_of_range>([&] { (void)tree.sum(kSize + 1); }));
    assert(rejects<std::out_of_range>([&] { (void)tree.get(kSize); }));
    assert(rejects<std::out_of_range>([&] { tree.add(-1, 1.0); }));
    assert(rejects<std::invalid_argument>([&] { tree.reset(-1); }));
    check(tree, values);
#endif
}

void check_compact() {
    constexpr int kSize = 8193;
    using Tree = sirkit::DynamicFenwickTree<int, std::int64_t>;
    Tree sparse(kSize);
    std::vector<std::int64_t> values(kSize);
    for (const int position : {61, 64, 4095, 8192}) {
        sparse.add(position, 2);
        values[position] = 2;
    }
    sparse.reserve(0);
    sparse.reserve(2);
    check(sparse, values);

    // 予約後のコピー・ムーブと再予約でも、点値と更新済みセル数を保つ。
    sparse.reserve(100);
    Tree copied(sparse);
    Tree moved(std::move(copied));
    assert(copied.empty());
    moved.reserve(300);
    assert(moved.stored_nodes() == sparse.stored_nodes());
    check(moved, values);
    moved.set(63, 9);
    assert(moved.get(63) == 9 && sparse.get(63) == 0);

    Tree tiny(13);
    assert(tiny.sum(1, 12) == 0);
    for (int i = 0; i < tiny.size(); ++i) tiny.add(i, i);
    assert(tiny.sum(3, 12) == 63);
    assert(tiny.sum(5, 5) == 0);
}

void check_wide() {
    using Index = std::uint64_t;
    using Tree = sirkit::DynamicFenwickTree<Index, std::int64_t>;
    constexpr Index kSize = std::numeric_limits<Index>::max();
    Tree original(kSize);
    Tree copied(original);
    assert(copied.size() == kSize && copied.sum_all() == 0);
    copied.reset(kSize - 1);
    copied.set(kSize - 2, 5);
    assert(copied.get(kSize - 2) == 5 && copied.sum(kSize - 3, kSize - 1) == 5);
    assert(copied.lower_bound(5) == kSize - 2 && copied.upper_bound(5) == kSize - 1);
    assert(original.get(kSize - 1) == 0);
    original.reserve(10);
    assert(rejects<std::length_error>([&] { original.reserve(std::numeric_limits<std::size_t>::max()); }));
    original.add(kSize - 1, 7);
    assert(original.get(kSize - 1) == 7 && original.sum(kSize - 1, kSize) == 7);
    assert(original.lower_bound(7) == kSize - 1 && original.upper_bound(7) == kSize);
}
}

int main() {
    check_fallback();
    check_compact();
    check_wide();
}
