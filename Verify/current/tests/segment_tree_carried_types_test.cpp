// SPDX-License-Identifier: CC0-1.0
// 点更新の値持ち越しによる型要件の増加と、左右の演算順を検証する。
#ifndef SIRKIT_SEGMENT_TEST_SOURCE
#define SIRKIT_SEGMENT_TEST_SOURCE "../../../DataStructure/Segment_Tree.cpp"
#endif
#include SIRKIT_SEGMENT_TEST_SOURCE
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
std::uint64_t checks = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }

// いずれも旧方式の長さ指定構築・点更新・値取得・全積が使える型。
template<int Case>
struct Value {
    int x;
    explicit Value(int v) : x(v) {}
    Value(Value&) requires (Case == 3) = delete;
    Value(const Value&) = default;
    Value(Value&&) = default;
    Value& operator=(Value&) requires (Case == 2) = delete;
    Value& operator=(const Value&) requires (Case != 1) = default;
    Value& operator=(const Value&) requires (Case == 1) = delete;
    Value& operator=(Value&&) = default;
};
template<int Case>
struct Add {
    Value<Case> operator()(const Value<Case>& a, const Value<Case>& b) const
        requires (Case != 4) { return Value<Case>(a.x + b.x); }
    Value<Case> operator()(Value<Case>& a, Value<Case>& b) const
        requires (Case == 4) { return Value<Case>(a.x + b.x); }
    Value<Case> operator()(Value<Case>&& a, Value<Case>&& b) const
        requires (Case == 4) { return Value<Case>(a.x + b.x); }
    Value<Case> operator()(const Value<Case>&, const Value<Case>&) const
        requires (Case == 4) = delete;
};
template<int Case> Value<Case> zero() { return Value<Case>(0); }

template<int Case>
void deleted_overloads() {
    using S = Value<Case>;
    static_assert(std::is_trivially_copyable_v<S>);
    static_assert(!std::is_default_constructible_v<S>);
    if constexpr (Case == 1) static_assert(!std::is_copy_assignable_v<S>);
    if constexpr (Case == 2) static_assert(!std::is_assignable_v<S&, S&>);
    if constexpr (Case == 3) static_assert(!std::is_constructible_v<S, S&>);
    if constexpr (Case == 4)
        static_assert(!std::is_invocable_v<Add<Case>, const S&, const S&>);
    std::mt19937 random(19003 + Case);
    for (const int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 127, 128, 129, 257}) {
        segtree<S, Add<Case>{}, zero<Case>> tree(n);
        std::vector<int> values(n, 0);
        check(tree.size() == n); check(tree.all_prod().x == 0);
        if (!n) continue;
        for (int rep = 0; rep < 200; ++rep) {
            const int p = random() % n;
            if (rep & 1) {
                values[p] = static_cast<int>(random() % 1000);
                tree.set(p, S(values[p]));
            } else {
                tree.transform(p, [](const S& v) { return S(v.x + 1); });
                ++values[p];
            }
            int sum = 0; for (const int v : values) sum += v;
            check(tree.get(p).x == values[p]); check(tree.all_prod().x == sum);
            if constexpr (Case != 4) {
                int l = random() % (n + 1), r = random() % (n + 1);
                if (l > r) std::swap(l, r);
                int expected = 0; for (int i = l; i < r; ++i) expected += values[i];
                check(tree.prod(l, r).x == expected);
            }
        }
    }
}

constexpr std::uint64_t modulus = 1000000007;
struct Affine {
    std::uint64_t a, b;
    Affine() = delete;
    Affine(std::uint64_t x, std::uint64_t y) : a(x), b(y) {}
    bool operator==(const Affine&) const = default;
};
static_assert(std::is_trivially_copyable_v<Affine> && sizeof(Affine) == 16);
Affine compose(Affine l, Affine r) {
    return {(l.a * r.a) % modulus, (l.b * r.a + r.b) % modulus};
}
Affine identity() { return {1, 0}; }
void noncommutative() {
    std::mt19937 random(81769);
    for (const int n : {1, 2, 3, 7, 8, 9, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257}) {
        std::vector<Affine> values(n, identity());
        segtree<Affine, compose, identity> tree(values);
        for (int rep = 0; rep < 200; ++rep) {
            const int p = random() % n;
            const Affine v(random() % modulus, random() % modulus);
            values[p] = v;
            if (rep & 1) tree.set(p, v);
            else tree.transform(p, [&](const Affine&) { return v; });
            int l = random() % (n + 1), r = random() % (n + 1);
            if (l > r) std::swap(l, r);
            Affine expected = identity();
            for (int i = l; i < r; ++i) {
                expected.b = (expected.b * values[i].a + values[i].b) % modulus;
                expected.a = (expected.a * values[i].a) % modulus;
            }
            check(tree.prod(l, r) == expected);
            expected = identity();
            for (const auto& x : values) {
                expected.b = (expected.b * x.a + x.b) % modulus;
                expected.a = (expected.a * x.a) % modulus;
            }
            check(tree.all_prod() == expected);
        }
    }
}

bool true_value() { return true; }
std::string join(const std::string& a, const std::string& b) { return a + b; }
std::string empty_word() { return {}; }
void bool_and_owning_type() {
    std::mt19937 random(17057);
    for (const int n : {1, 2, 3, 7, 8, 9, 33, 65}) {
        std::vector<bool> bits(n, true);
        segtree<bool, std::logical_and<bool>{}, true_value> boolean_tree(bits);
        std::vector<std::string> words(n);
        segtree<std::string, join, empty_word> word_tree(words);
        for (int rep = 0; rep < 150; ++rep) {
            const int p = random() % n;
            bits[p] = !bits[p];
            words[p] = std::string(1 + random() % 3, static_cast<char>('a' + random() % 26));
            if (rep & 1) {
                boolean_tree.set(p, bits[p]); word_tree.set(p, words[p]);
            } else {
                boolean_tree.transform(p, [](bool v) { return !v; });
                word_tree.transform(p, [&](const auto&) { return words[p]; });
            }
            bool expected_bool = true; std::string expected_word;
            for (int i = 0; i < n; ++i) { expected_bool &= bits[i]; expected_word += words[i]; }
            check(boolean_tree.all_prod() == expected_bool); check(word_tree.all_prod() == expected_word);
            check(boolean_tree.get(p) == bits[p]); check(word_tree.get(p) == words[p]);
        }
        auto copied = word_tree;
        word_tree.set(0, "independent");
        check(copied.get(0) == words[0]);
    }
}
}
int main() {
    deleted_overloads<1>(); deleted_overloads<2>();
    deleted_overloads<3>(); deleted_overloads<4>();
    noncommutative(); bool_and_owning_type();
    std::printf("segment_tree_carried_types_test: PASS (%" PRIu64 " checks)\n", checks);
}
