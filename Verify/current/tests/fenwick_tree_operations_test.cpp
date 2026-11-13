// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_FENWICK_TEST_SOURCE
#define SIRKIT_FENWICK_TEST_SOURCE "../../../DataStructure/Fenwick_Tree.cpp"
#endif
#include SIRKIT_FENWICK_TEST_SOURCE
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <utility>
#include <vector>

namespace {
std::uint64_t checks = 0;
void require(bool condition) { ++checks; if (!condition) std::abort(); }

// 順序も整数変換も要求しない、2成分の加法群。
struct Group {
    std::uint32_t first = 0, second = 0;
    Group& operator+=(const Group& x) { first += x.first; second += x.second; return *this; }
    Group& operator-=(const Group& x) { first -= x.first; second -= x.second; return *this; }
    friend Group operator+(Group a, const Group& b) { return a += b; }
    friend Group operator-(Group a, const Group& b) { return a -= b; }
    friend bool operator==(const Group&, const Group&) = default;
};

template<class T>
void check_intervals(fenwick_tree<T>& tree, const std::vector<T>& values) {
    require(tree.size() == values.size());
    require(tree.empty() == values.empty());
    require(tree.values() == values);
    T total{};
    for (std::size_t i = 0; i < values.size(); ++i) {
        require(tree.get(i) == values[i]);
        total += values[i];
        require(tree.sum(0, i + 1) == total);
    }
    require(tree.sum(0, 0) == T{});
    require(tree.sum(0, tree.size()) == total);
    for (std::size_t l = 0; l <= values.size(); ++l) {
        T expected{};
        for (std::size_t r = l; r <= values.size(); ++r) {
            require(tree.sum(l, r) == expected);
            if (r < values.size()) expected += values[r];
        }
    }
}

template<class T>
void check_operations(std::vector<T> values) {
    fenwick_tree<T> tree(values);
    check_intervals(tree, values);
    for (std::size_t i = 0; i < values.size(); ++i) {
        const T delta = values[values.size() - 1 - i];
        tree.add(i, delta); values[i] += delta;
        require(tree.get(i) == values[i]);
    }
    check_intervals(tree, values);
    for (std::size_t i = 0; i < values.size(); ++i) {
        const T replacement = values[values.size() - 1 - i];
        tree.set(i, replacement); values[i] = replacement;
    }
    check_intervals(tree, values);
    auto copy = tree;
    if (!values.empty()) copy.set(0, T{});
    require(tree.values() == values);
    copy = tree;
    auto moved = std::move(copy);
    require(copy.empty()); require(copy.sum(0, 0) == T{});
    check_intervals(moved, values);
    copy.reset(3); copy = std::move(moved);
    require(moved.empty()); require(copy.values() == values);
    copy = std::move(copy);
    require(copy.values() == values);
    moved.swap(copy);
    require(copy.empty()); require(moved.values() == values);
    moved.reset();
    check_intervals(moved, std::vector<T>(values.size()));
    moved.clear(); check_intervals(moved, std::vector<T>{});
    moved.reset(5); check_intervals(moved, std::vector<T>(5));
    moved.reset(0); check_intervals(moved, std::vector<T>{});
}

void check_signed_limits(const std::vector<std::int64_t>& values) {
    fenwick_tree<std::int64_t> tree(values);
    require(tree.values() == values);
    for (std::size_t i = 0; i < values.size(); ++i) require(tree.get(i) == values[i]);
#if defined(__SIZEOF_INT128__)
    // 参照計算だけ拡張整数を使用し、答えがint64_tに収まる区間を照合する。
    for (std::size_t l = 0; l <= values.size(); ++l) {
        __int128 expected = 0;
        for (std::size_t r = l; r <= values.size(); ++r) {
            if (expected >= std::numeric_limits<std::int64_t>::min()
                && expected <= std::numeric_limits<std::int64_t>::max())
                require(tree.sum(l, r) == static_cast<std::int64_t>(expected));
            if (r < values.size()) expected += values[r];
        }
    }
#endif
}

void check_bounds(const std::vector<std::int64_t>& values) {
    fenwick_tree<std::int64_t> tree(values);
    std::int64_t total = 0;
    for (auto value : values) total += value;
    for (std::int64_t target = -1; target <= total + 1; ++target) {
        std::size_t lower = values.size(), upper = values.size();
        std::int64_t prefix = 0;
        for (std::size_t i = 0; i < values.size(); ++i) {
            prefix += values[i];
            if (lower == values.size() && prefix >= target) lower = i;
            if (upper == values.size() && prefix > target) upper = i;
        }
        if (target <= 0) lower = 0;
        if (target < 0) upper = 0;
        require(tree.lower_bound(target) == lower);
        require(tree.upper_bound(target) == upper);
    }
}
}

int main() {
    check_operations<std::uint32_t>({});
    check_operations<Group>({});
    for (std::size_t n = 1; n <= 79; ++n) {
        std::vector<std::uint32_t> integers(n);
        std::vector<Group> groups(n);
        for (std::size_t i = 0; i < n; ++i) {
            integers[i] = std::uint32_t{0xfffffff0U} + static_cast<std::uint32_t>(i * 17);
            groups[i] = {integers[i], static_cast<std::uint32_t>(i * 31 + 7)};
        }
        check_operations(integers); check_operations(groups);
        std::vector<std::int64_t> zeros(n), gaps(n);
        for (std::size_t i = 2; i < n; i += 7) gaps[i] = 3;
        check_bounds(zeros); check_bounds(gaps);
    }
    check_bounds({});
    const auto low = std::numeric_limits<std::int64_t>::min();
    const auto high = std::numeric_limits<std::int64_t>::max();
    check_signed_limits({low}); check_signed_limits({high});
    check_signed_limits({low, 0, high, 1});
    check_signed_limits({high, 0, -high, -1});
    const std::vector<std::int64_t> cancellation{0, 0, 0, 0, -high, 0, high, 0, -2};
    check_signed_limits(cancellation);
    require(fenwick_tree<std::int64_t>(cancellation).sum(7, 9) == -2);
#ifndef SKIP_RUNTIME_CHECKS
    bool rejected = false;
    try { fenwick_tree<std::int64_t> invalid(-1); }
    catch (const std::exception&) { rejected = true; }
    require(rejected);
    fenwick_tree<std::int64_t> retained(std::vector<std::int64_t>{1, 2, 3});
    rejected = false;
    try { retained.reset(-1); }
    catch (const std::exception&) { rejected = true; }
    require(rejected); require(retained.values() == std::vector<std::int64_t>({1, 2, 3}));
#endif
    std::printf("fenwick_tree supplemental checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
