// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/implicit_treap.cpp"
#include "../../../DataStructure/dynamic-li-chao-tree.cpp"
#include "../../../DataStructure/line-container-2d.cpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

using namespace sirkit;
using I = std::int64_t;
static std::size_t checks = 0;
static void require(bool value) { ++checks; assert(value); }

static void constructor_test() {
    using Tree = lazy_implicit_treap<>;
    Tree empty{}, nested_empty{{}}, nested_argument({{}});
    Tree one({7}), nested_one{{7}}, two({7, 9}), nested_two{{7, 9}};
    for (auto* tree : {&empty, &nested_empty})
        require(tree->empty() && tree->identity() == RangeAddMin::e() && tree->all_prod() == RangeAddMin::e());
    require(nested_argument.to_vector() == std::vector<I>{0} && nested_argument.identity() == RangeAddMin::e());
    for (auto* tree : {&one, &nested_one})
        require(tree->to_vector() == std::vector<I>{7} && tree->identity() == RangeAddMin::e() && tree->all_prod() == 7);
    for (auto* tree : {&two, &nested_two})
        require(tree->to_vector() == std::vector<I>({7, 9}) && tree->identity() == RangeAddMin::e() && tree->all_prod() == 7);
    const std::vector<I> values{7, 9};
    Tree from_vector(values), configured(values, 1000), custom(1000);
    require(from_vector.to_vector() == values && from_vector.identity() == RangeAddMin::e());
    require(configured.to_vector() == values && configured.identity() == 1000 && configured.all_prod() == 7);
    require(custom.empty() && custom.identity() == 1000 && custom.all_prod() == 1000);
    Tree copy{configured}, moved{std::move(copy)};
    require(copy.empty() && copy.identity() == 1000 && moved.to_vector() == values && moved.identity() == 1000);
    static_assert(std::is_nothrow_move_constructible_v<Tree> && std::is_nothrow_move_assignable_v<Tree>);

    using S = RangeAddSum::S;
    using Sum = lazy_implicit_treap<S, RangeAddSum::op, RangeAddSum::e, I,
                                  RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;
    Sum aggregate_empty{}, aggregate_list_empty(std::vector<S>{}), aggregate_one({{7, 1}}), aggregate_two({{7, 1}, {9, 1}});
    Sum aggregate_custom(S{0, 0}), aggregate_configured(std::vector<S>{{7, 1}}, S{0, 0});
    require(aggregate_empty.empty() && aggregate_list_empty.empty() && aggregate_list_empty.identity().size == 0);
    require(aggregate_one.size() == 1 && aggregate_one.all_prod().sum == 7 && aggregate_one.all_prod().size == 1);
    require(aggregate_two.size() == 2 && aggregate_two.all_prod().sum == 16 && aggregate_two.all_prod().size == 2);
    require(aggregate_custom.empty() && aggregate_custom.identity().sum == 0 && aggregate_custom.identity().size == 0);
    Sum aggregate_copy{aggregate_configured}, aggregate_moved{std::move(aggregate_copy)};
    require(aggregate_copy.empty() && aggregate_copy.identity().size == 0 && aggregate_moved.all_prod().sum == 7);
    static_assert(std::is_nothrow_move_constructible_v<Sum> && std::is_nothrow_move_assignable_v<Sum>);
}

static void treap_test() {
    std::mt19937 rng(71827);
    for (I inf : {I{1}, I{1000}, I{1} << 60, I{1} << 62, std::numeric_limits<I>::max()}) {
        lazy_implicit_treap<> tree(inf);
        std::vector<I> values;
        for (int iteration = 0; iteration < 4000; ++iteration) {
            const int n = static_cast<int>(values.size());
            int l = static_cast<int>(rng() % (n + 1)), r = static_cast<int>(rng() % (n + 1));
            if (l > r) std::swap(l, r);
            const int mode = rng() % 7;
            if (mode == 0 && n < 80) {
                const I value = -I(rng() % 500);
                tree.insert_at(l, value); values.insert(values.begin() + l, value);
            } else if (mode == 1) {
                I expected = inf; for (int i = l; i < r; ++i) expected = std::min(expected, values[i]);
                require(tree.erase_prod(l, r) == expected); values.erase(values.begin() + l, values.begin() + r);
            } else if (mode == 2) {
                tree.apply(l, r, -3); for (int i = l; i < r; ++i) values[i] -= 3;
            } else if (mode == 3) {
                tree.reverse(l, r); std::reverse(values.begin() + l, values.begin() + r);
            } else if (mode == 4 && n) {
                const I value = -I(rng() % 500); tree.set(l % n, value); values[l % n] = value;
            } else if (mode == 5) {
                auto copy = tree; lazy_implicit_treap<> assigned(17); assigned.reserve(100); assigned = tree;
                require(copy.to_vector() == values && assigned.to_vector() == values && assigned.identity() == inf);
                auto moved = std::move(copy); require(copy.empty() && copy.all_prod() == inf);
                copy = std::move(moved); require(moved.empty() && moved.identity() == inf);
                lazy_implicit_treap<> other(19); swap(other, copy);
                require(other.identity() == inf && copy.identity() == 19 && other.to_vector() == values);
                other.clear(); require(other.prod(0, 0) == inf);
            }
            require(tree.to_vector() == values && tree.identity() == inf);
            const int size = int(values.size()); l = rng() % (size + 1); r = rng() % (size + 1); if (l > r) std::swap(l, r);
            I expected = inf; for (int i = l; i < r; ++i) expected = std::min(expected, values[i]);
            require(tree.prod(l, r) == expected);
            int right = l; while (right < size && values[right] >= -250) ++right;
            int left = r; while (left && values[left - 1] >= -250) --left;
            require(tree.max_right(l, [](I x) { return x >= -250; }) == right);
            require(tree.min_left(r, [](I x) { return x >= -250; }) == left);
        }
        lazy_implicit_treap<> high(std::vector<I>{inf, inf - 1}, inf);
        require(high.prod(0, 1) == inf && high.prod(0, 2) == inf - 1);
        indexed_implicit_treap<> indexed(inf); indexed.insert_at_with_id(0, 2, inf - 1); indexed.insert_after(2, 8, -1);
        require(indexed.prod_between(2, 8) == inf && indexed.erase_between(2, 8) == inf);
        auto moved = std::move(indexed); require(indexed.identity() == inf && moved.get_by_id(2) == inf - 1);
    }
}

template<class T> static void li_chao_test() {
    using Tree = DynamicLiChaoTree<T>;
    std::mt19937 rng(71928 + sizeof(T));
    for (T inf : {T{1}, T{100}, std::numeric_limits<T>::max()}) {
        Tree tree(-7, 7, inf);
        struct Segment { I a, b; int l, r; };
        std::vector<Segment> lines;
        for (int i = 0; i < 250; ++i) {
            if (i % 50 == 49) { tree.clear(); lines.clear(); }
            const I a = I(rng() % 9) - 4, b = -40 - I(rng() % 20);
            int l = int(rng() % 15) - 7, r = int(rng() % 15) - 7; if (l > r) std::swap(l, r);
            tree.add_segment(T(a), T(b), T(l), T(r)); lines.push_back({a, b, l, r});
            for (int x = -7; x <= 7; ++x) {
                I expected = inf; for (auto line : lines) if (line.l <= x && x <= line.r) expected = std::min(expected, line.a * x + line.b);
                require(tree.query(T(x)) == expected);
                const auto line = tree.query_line(T(x)); require(bool(line) == (expected != inf));
                if (line) require(line->get(T(x)) == expected);
            }
            if (i % 13 == 0) {
                auto copy = tree; Tree assigned(0, 0, 7); assigned = tree;
                require(assigned.infinity() == inf && assigned.domain_min() == -7);
                auto moved = std::move(copy); require(copy.empty() && copy.query(0) == inf);
                assigned = std::move(moved); require(moved.empty() && moved.infinity() == inf);
                require(assigned.query(0) == tree.query(0)); assigned.clear(); require(assigned.query(0) == inf);
            }
        }
        Tree single(1, 1, inf); single.add_line(-1, inf); require(single.query(1) == inf - 1);
        Tree bound(0, 0, inf); bound.add_line(0, std::numeric_limits<T>::min()); require(bound.query(0) == std::numeric_limits<T>::min());
#ifndef SKIP_RUNTIME_CHECKS
        bool caught = false; try { Tree invalid(0, 0, 0); } catch (const std::invalid_argument&) { caught = true; } require(caught);
        caught = false; try { single.add_line(0, inf); } catch (const std::overflow_error&) { caught = true; } require(caught);
#endif
    }
    Tree defaults(0, 0); require(defaults.query(0) == (sizeof(T) < 8 ? I(std::numeric_limits<T>::max()) : (I{1} << 60)));
    DynamicLiChaoTree<T, T{99}> legacy(0, 0); require(legacy.infinity() == 99 && legacy.query(0) == 99);
}

static I integer_reference(long double x, I inf, bool rounded) {
    const long double integer = rounded ? std::round(x) : std::trunc(x);
    if (integer >= static_cast<long double>(inf)) return inf;
    if (integer <= -static_cast<long double>(inf)) return -inf;
    return static_cast<I>(integer);
}
static void line2d_test() {
    std::mt19937 rng(99218);
    for (I inf : {I{1}, I{17}, I{1} << 60, I{4100000000000000000}, std::numeric_limits<I>::max()}) {
        LineContainer2D tree(inf); std::vector<std::pair<I, I>> points;
        for (int i = 0; i < 300; ++i) {
            if (i % 71 == 0) { tree.clear(); points.clear(); require(tree.infinity() == inf && !tree.max_optional(1, 1)); }
            const I x = I(rng() % 2001) - 1000, y = I(rng() % 2001) - 1000;
            tree.add(x / 2.0L, y / 2.0L); points.emplace_back(x, y);
            const I a = I(rng() % 31) - 15, b = I(rng() % 31) - 15;
            I low = std::numeric_limits<I>::max(), high = std::numeric_limits<I>::min();
            for (auto [px, py] : points) { const I value = a * px + b * py; low = std::min(low, value); high = std::max(high, value); }
            require(tree.min_ld(a, b) == low / 2.0L && tree.max_ld(a, b) == high / 2.0L);
            require(tree.min_ll(a, b) == integer_reference(low / 2.0L, inf, false));
            require(tree.max_ll(a, b) == integer_reference(high / 2.0L, inf, false));
            require(tree.min_rounded_ll(a, b) == integer_reference(low / 2.0L, inf, true));
            require(tree.max_rounded_ll(a, b) == integer_reference(high / 2.0L, inf, true));
            if (i % 17 == 0) {
                auto copy = tree; LineContainer2D assigned(9); assigned = tree; auto moved = std::move(copy);
                require(copy.empty() && copy.infinity() == inf && moved.infinity() == inf);
                assigned = std::move(moved); require(moved.empty() && moved.infinity() == inf);
                LineContainer2D other(7); assigned.swap(other);
                require(assigned.infinity() == 7 && other.infinity() == inf && other.max_ll(a, b) == tree.max_ll(a, b));
            }
        }
        for (long double x : {1e30L, -1e30L, static_cast<long double>(inf), -static_cast<long double>(inf), std::nextafter(static_cast<long double>(inf), 0.0L), std::nextafter(-static_cast<long double>(inf), 0.0L)}) {
            LineContainer2D single(inf); single.add(x, 0);
            require(single.max_ll(1, 0) == integer_reference(x, inf, false));
            require(single.min_rounded_ll(1, 0) == integer_reference(x, inf, true));
        }
    }
    LineContainer2D defaults; require(defaults.infinity() == (I{1} << 60));
#ifndef SKIP_RUNTIME_CHECKS
    for (I inf : {I{0}, I{-1}, std::numeric_limits<I>::min()}) {
        bool caught = false; try { LineContainer2D invalid(inf); } catch (const std::invalid_argument&) { caught = true; } require(caught);
    }
#endif
}
int main() {
    constructor_test(); treap_test(); li_chao_test<std::int8_t>(); li_chao_test<std::int16_t>(); li_chao_test<std::int32_t>(); li_chao_test<I>(); line2d_test();
    std::cout << checks << " custom identity/inf reference checks passed\n";
}
