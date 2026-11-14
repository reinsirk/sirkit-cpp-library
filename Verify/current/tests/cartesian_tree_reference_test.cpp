// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_CARTESIAN_TREE_SOURCE
#define SIRKIT_CARTESIAN_TREE_SOURCE "../../../Graph/Cartesian_Tree.cpp"
#endif
#include SIRKIT_CARTESIAN_TREE_SOURCE
#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <tuple>
static_assert(!std::is_constructible_v<CartesianTree, const std::vector<bool>&>);
static std::uint64_t checks = 0;
void require(bool value) { ++checks; if (!value) { std::cerr << "Cartesian check " << checks << '\n'; std::abort(); } }
template<class F> void rejected(F f) { bool caught = false; try { f(); } catch (const std::exception&) { caught = true; } require(caught); }
template<class T> std::vector<T> saved(std::span<const T> values) { return {values.begin(), values.end()}; }
template<class T, class Compare> void check(std::span<T> values, Compare compare) {
    CartesianTree tree(values, compare);
    const auto n = values.size(); require(tree.size() == n); require(tree.empty() == (n == 0));
    std::vector<std::int32_t> parents(n, -1), left(n, -1), right(n, -1);
    std::vector<std::tuple<std::size_t, std::size_t, std::int32_t, bool>> ranges;
    if (n) ranges.emplace_back(0, n, -1, false); else require(tree.root() == -1);
    // 各区間の優先元を走査で選ぶ。単調stackを使わない独立参照。
    while (!ranges.empty()) {
        const auto [l,r,p,is_right] = ranges.back(); ranges.pop_back(); auto best = l;
        for (auto i = l + 1; i < r; ++i) if (std::invoke(compare, values[i], values[best])) best = i;
        const auto vertex = static_cast<std::int32_t>(best); parents[best] = p;
        if (p == -1) require(tree.root() == vertex);
        else (is_right ? right : left)[static_cast<std::size_t>(p)] = vertex;
        if (l < best) ranges.emplace_back(l, best, vertex, false);
        if (best + 1 < r) ranges.emplace_back(best + 1, r, vertex, true);
    }
    require(saved(tree.parents()) == parents); require(saved(tree.left_children()) == left); require(saved(tree.right_children()) == right);
    for (std::size_t i = 0; i < n; ++i) {
        auto v = static_cast<std::int32_t>(i); require(tree.parent(v) == parents[i]); require(tree.left_child(v) == left[i]);
        require(tree.right_child(v) == right[i]); require(tree.children(v) == std::pair{left[i],right[i]});
    }
    // 各区間の両端LCAが左優先RMQに一致することを別に確認。
    for (std::size_t l = 0; l < n; ++l) for (std::size_t r = l; r < n; ++r) {
        auto best = l; for (auto i = l + 1; i <= r; ++i) if (std::invoke(compare, values[i], values[best])) best = i;
        std::vector<bool> ancestor(n); for (auto v = static_cast<std::int32_t>(l); v != -1; v = tree.parent(v)) ancestor[static_cast<std::size_t>(v)] = true;
        auto v = static_cast<std::int32_t>(r); while (!ancestor[static_cast<std::size_t>(v)]) v = tree.parent(v);
        require(v == static_cast<std::int32_t>(best));
    }
    CartesianTree vector_copy(tree); require(saved(vector_copy.parents()) == parents);
}
struct Payload { int key, label; Payload(int k, int l):key(k),label(l){} Payload(const Payload&) = delete; Payload(Payload&&) = default; bool earlier(const Payload& other) const { return key < other.key; } };
struct MoveOnlyCompare {
    std::unique_ptr<int> calls = std::make_unique<int>(0);
    bool operator()(const int& a, const int& b) { ++*calls; return a < b; }
};
void ownership() {
    std::vector<int> values{4,2,2,5,1,8,1}; const auto before = values; CartesianTree tree(values);
    require(values == before); auto parents = saved(tree.parents());
    CartesianTree copy(tree); copy.build(std::vector<int>{9,4,7}); require(saved(tree.parents()) == parents);
    copy = tree; require(saved(copy.parents()) == parents); auto* tree_self = &tree; tree = *tree_self; require(saved(tree.parents()) == parents);
    CartesianTree moved(std::move(copy)); require(copy.empty() && copy.root() == -1); require(saved(moved.parents()) == parents);
    copy = std::move(moved); require(moved.empty() && moved.root() == -1); require(saved(copy.parents()) == parents);
    auto* self = &copy; copy = std::move(*self); require(saved(copy.parents()) == parents);
    swap(copy, moved); require(copy.empty()); require(saved(moved.parents()) == parents); moved.clear(); require(moved.empty() && moved.root() == -1);
    tree.build(tree.parents()); require(tree.size() == parents.size()); // 旧index配列を入力にできる。
    CartesianTree empty; require(empty.root() == -1); empty.build(std::span<const int>{}); require(empty.empty());
    const std::array<bool, 4> boolean{true,false,false,true}; CartesianTree booltree{std::span(boolean)}; require(booltree.root() == 1);
    Payload payload[]{ {3,8},{1,9},{1,1},{4,2},{0,3} };
    auto cmp = [](const Payload& a,const Payload& b){return a.key < b.key;}; check(std::span<Payload>(payload),cmp);
    MoveOnlyCompare move_only; CartesianTree by_ref(values, std::ref(move_only)); require(*move_only.calls > 0); by_ref.build(values, move_only); by_ref.build(values, MoveOnlyCompare{});
    auto member = &Payload::key; CartesianTree membercmp(std::span<Payload>(payload),[&](const Payload& a,const Payload& b){return std::invoke(member,a)<std::invoke(member,b);});require(membercmp.root() == 4); CartesianTree member_function(std::span<Payload>(payload), &Payload::earlier); require(member_function.root() == 4);
#ifndef SKIP_RUNTIME_CHECKS
    rejected([&]{empty.parent(0);}); rejected([&]{tree.parent(-1);}); rejected([&]{tree.left_child(static_cast<std::int32_t>(tree.size()));});
    rejected([&]{tree.right_child(-2);}); rejected([&]{tree.children(-1);});
#endif
}
void chains() {
    constexpr std::int32_t n = 300000; std::vector<int> a(static_cast<std::size_t>(n)); std::iota(a.begin(),a.end(),0);
    for (int mode = 0; mode < 3; ++mode) {
        if (mode == 1) std::reverse(a.begin(),a.end());
        if (mode == 2) std::fill(a.begin(),a.end(),7);
        std::size_t calls = 0; CartesianTree tree(a,[&](int x,int y){++calls;return x < y;}); require(calls <= 2 * a.size());
        require(tree.root() == (mode == 1 ? n - 1 : 0));
        for (std::int32_t i = 0; i < n; ++i) {
            require(tree.parent(i) == (mode == 1 ? (i + 1 == n ? -1 : i + 1) : (i == 0 ? -1 : i - 1)));
            require(tree.left_child(i) == (mode == 1 && i > 0 ? i - 1 : -1));
            require(tree.right_child(i) == (mode != 1 && i + 1 < n ? i + 1 : -1));
        }
    }
}
int main() {
    for (std::size_t n = 0, possibilities = 1; n <= 8; ++n, possibilities *= 3) {
        std::vector<int> values(n);
        for (std::size_t code = 0; code < possibilities; ++code) {
            auto remaining = code; for (auto& value : values) { value = static_cast<int>(remaining % 3) - 1; remaining /= 3; }
            check(std::span<int>(values), std::less<>{}); check(std::span<int>(values), std::greater<>{});
        }
    }
    std::mt19937_64 random(6754596615ULL);
    for (int trial = 0; trial < 250; ++trial) {
        std::vector<std::int64_t> values(static_cast<std::size_t>(random() % 45)); for (auto& v : values) v = static_cast<std::int64_t>(random() % 17) - 8;
        check(std::span<std::int64_t>(values),std::less<>{}); check(std::span<std::int64_t>(values),std::greater<>{});
        CartesianTree from_vector(values); CartesianTree from_span{std::span<const std::int64_t>(values)}; require(saved(from_vector.parents()) == saved(from_span.parents()));
    }
    ownership(); chains(); std::cout << "Cartesian independent checks=" << checks << '\n';
}
