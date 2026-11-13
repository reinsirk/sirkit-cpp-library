// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_FENWICK_TEST_SOURCE
#define SIRKIT_FENWICK_TEST_SOURCE "../../../DataStructure/Fenwick_Tree.cpp"
#endif
#ifndef SIRKIT_SEGMENT_TEST_SOURCE
#define SIRKIT_SEGMENT_TEST_SOURCE "../../../DataStructure/Segment_Tree.cpp"
#endif
#ifndef SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#define SIRKIT_LAZY_SEGMENT_TEST_SOURCE "../../../DataStructure/Lazy_Segment_Tree.cpp"
#endif
#include SIRKIT_FENWICK_TEST_SOURCE
#include SIRKIT_SEGMENT_TEST_SOURCE
#include SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
std::uint64_t checks = 0;
void require(bool condition) { ++checks; if (!condition) std::abort(); }
using I = std::int64_t;
I op(I left, I right) { return left + right; }
I e() { return 0; }
bool at_most_ten(I value) { return value <= 10; }
struct Sum { I sum = 0; int length = 0; friend bool operator==(Sum, Sum) = default; };
struct Affine { I a = 1, b = 0; };
Sum lazy_op(Sum left, Sum right) { return {left.sum + right.sum, left.length + right.length}; }
Sum lazy_e() { return {}; }
Sum mapping(Affine f, Sum value) { return {f.a * value.sum + f.b * value.length, value.length}; }
Affine composition(Affine newer, Affine older) { return {newer.a * older.a, newer.a * older.b + newer.b}; }
Affine id() { return {}; }
bool lazy_at_most_ten(Sum value) { return value.sum <= 10; }
struct NonDefault {
    I value;
    NonDefault() = delete;
    explicit NonDefault(I x) : value(x) {}
};
struct NonDefaultAction {
    I delta;
    NonDefaultAction() = delete;
    explicit NonDefaultAction(I x) : delta(x) {}
};
NonDefault nondefault_op(NonDefault a, NonDefault b) { return NonDefault(a.value < b.value ? b.value : a.value); }
NonDefault nondefault_e() { return NonDefault(std::numeric_limits<I>::min()); }
NonDefault nondefault_mapping(NonDefaultAction f, NonDefault a) { return a.value == std::numeric_limits<I>::min() ? a : NonDefault(a.value + f.delta); }
NonDefaultAction nondefault_composition(NonDefaultAction newer, NonDefaultAction older) { return NonDefaultAction(newer.delta + older.delta); }
NonDefaultAction nondefault_id() { return NonDefaultAction(0); }
using Seg = segtree<I, op, e>;
using Lazy = lazy_segtree<Sum, lazy_op, lazy_e, Affine, mapping, composition, id>;
using Bit = fenwick_tree<I>;
static_assert(std::default_initializable<Seg> && std::constructible_from<Seg, int>);
static_assert(std::constructible_from<Seg, const std::vector<I>&>);
static_assert(std::default_initializable<Lazy> && std::constructible_from<Lazy, int>);
static_assert(std::constructible_from<Lazy, const std::vector<Sum>&>);
static_assert(std::default_initializable<Bit> && std::constructible_from<Bit, int>);
static_assert(std::same_as<decltype(std::declval<Seg&>().set(0, I{})), void>);
static_assert(std::same_as<decltype(std::declval<const Seg&>().get(0)), I>);
static_assert(std::same_as<decltype(std::declval<const Seg&>().prod(0, 0)), I>);
static_assert(std::same_as<decltype(std::declval<const Seg&>().all_prod()), I>);
static_assert(std::same_as<decltype(std::declval<Seg&>().max_right<at_most_ten>(0)), int>);
static_assert(std::same_as<decltype(std::declval<Seg&>().min_left<at_most_ten>(0)), int>);
static_assert(std::same_as<decltype(std::declval<Lazy&>().get(0)), Sum>);
static_assert(std::same_as<decltype(std::declval<Lazy&>().all_prod()), Sum>);
static_assert(std::same_as<decltype(std::declval<Lazy&>().prod(0, 0)), Sum>);
static_assert(std::same_as<decltype(std::declval<Lazy&>().apply(0, Affine{})), void>);
static_assert(std::same_as<decltype(std::declval<Lazy&>().apply(0, 0, Affine{})), void>);
static_assert(std::same_as<decltype(std::declval<Lazy&>().max_right<lazy_at_most_ten>(0)), int>);
static_assert(std::same_as<decltype(std::declval<Lazy&>().min_left<lazy_at_most_ten>(0)), int>);
static_assert(std::same_as<decltype(std::declval<Bit&>().add(0, I{})), void>);
static_assert(std::same_as<decltype(std::declval<Bit&>().sum(0, 0)), I>);
static_assert(std::same_as<decltype(&Seg::set), void (Seg::*)(int, I)>);
static_assert(std::same_as<decltype(&Lazy::set), void (Lazy::*)(int, Sum)>);
static_assert(std::same_as<decltype(&Bit::add), void (Bit::*)(int, I)>);
static_assert(std::same_as<decltype(&Bit::sum), I (Bit::*)(int, int)>);
static_assert(std::same_as<decltype(&Lazy::all_prod), Sum (Lazy::*)()>);

void basic_calls() {
    Seg empty;
    require(empty.prod(0, 0) == e()); require(empty.all_prod() == e());
    require(empty.max_right<at_most_ten>(0) == 0); require(empty.min_left<at_most_ten>(0) == 0);
    Lazy empty_lazy;
    require(empty_lazy.prod(0, 0) == lazy_e()); require(empty_lazy.all_prod() == lazy_e());
    empty_lazy.apply(0, 0, Affine{0, 3});
    require(empty_lazy.max_right<lazy_at_most_ten>(0) == 0);
    require(empty_lazy.min_left<lazy_at_most_ten>(0) == 0);
    Bit empty_bit; require(empty_bit.sum(0, 0) == 0);
    Bit brace_empty({}); require(brace_empty.size() == 0);
    Bit brace_size({4}); require(brace_size.size() == 4); require(brace_size.sum(0, 4) == 0);
    Seg tree(std::vector<I>{1, 2, 3, 4, 5});
    require(tree.max_right<at_most_ten>(0) == 4);
    require(tree.min_left<at_most_ten>(5) == 3);
    require(tree.max_right(1, [](I value) { return value <= 5; }) == 3);
    require(tree.min_left(4, [](I value) { return value <= 7; }) == 2);
    tree.set(2, 7); require(tree.get(2) == 7); require(tree.prod(1, 4) == 13);
    Lazy lazy(std::vector<Sum>{{1,1},{2,1},{3,1},{4,1},{5,1}});
    require(lazy.max_right<lazy_at_most_ten>(0) == 4);
    require(lazy.min_left<lazy_at_most_ten>(5) == 3);
    lazy.apply(1, 4, Affine{2, 1});
    require(lazy.prod(1, 4) == Sum{21, 3});
    lazy.apply(2, Affine{0, 4}); lazy.set(4, Sum{2, 1});
    require(lazy.get(2) == Sum{4, 1}); require(lazy.all_prod() == Sum{21, 5});
    require(lazy.max_right(0, [](Sum value) { return value.sum <= 10; }) == 3);
    require(lazy.min_left(5, [](Sum value) { return value.sum <= 11; }) == 3);
    constexpr auto lambda_op = [](I a, I b) { return a + b; };
    constexpr auto lambda_e = [] { return I{}; };
    segtree<I, lambda_op, lambda_e> lambda_tree(3);
    lambda_tree.set(1, 5); require(lambda_tree.all_prod() == 5);
    segtree<NonDefault, nondefault_op, nondefault_e> nondefault(std::vector<NonDefault>{NonDefault(2), NonDefault(5)});
    require(nondefault.prod(0, 2).value == 5);
    lazy_segtree<NonDefault, nondefault_op, nondefault_e, NonDefaultAction,
                 nondefault_mapping, nondefault_composition, nondefault_id> nondefault_lazy(
                     std::vector<NonDefault>{NonDefault(2), NonDefault(5)});
    nondefault_lazy.apply(0, 2, NonDefaultAction(3));
    require(nondefault_lazy.get(0).value == 5); require(nondefault_lazy.all_prod().value == 8);
}

void random_reference() {
    std::mt19937_64 random(0xd1621b49);
    for (const int n : {0,1,2,3,7,8,9,31,32,33,127}) {
        Bit bit(n); Seg tree(n);
        std::vector<I> values(n);
        std::vector<Sum> nodes(n, Sum{0, 1}); Lazy lazy(nodes);
        for (int repeat = 0; repeat < 800; ++repeat) {
            if (n && (random() & 1)) {
                const int p = static_cast<int>(random() % n);
                const I delta = static_cast<I>(random() % 8);
                bit.add(p, delta); tree.set(p, tree.get(p) + delta); lazy.apply(p, Affine{1, delta}); values[p] += delta;
            }
            int l = static_cast<int>(random() % (n + 1)), r = static_cast<int>(random() % (n + 1));
            if (l > r) std::swap(l, r);
            I expected = 0; for (int p = l; p < r; ++p) expected += values[p];
            require(bit.sum(l,r) == expected); require(tree.prod(l,r) == expected); require(lazy.prod(l,r) == Sum{expected,r-l});
        }
    }
}
void integral_wrap() {
    fenwick_tree<std::uint32_t> unsigned_tree(3);
    unsigned_tree.add(0, 0xffffffffU); unsigned_tree.add(1, 2);
    require(unsigned_tree.sum(0, 2) == 1U);
    unsigned_tree.add(2, 0x80000000U); require(unsigned_tree.sum(1, 3) == 0x80000002U);
    fenwick_tree<std::int32_t> signed_tree(2);
    signed_tree.add(0, std::numeric_limits<std::int32_t>::max()); signed_tree.add(1, 1);
    require(signed_tree.sum(0, 2) == std::numeric_limits<std::int32_t>::min());
    signed_tree.add(0, 1); require(signed_tree.sum(0, 1) == std::numeric_limits<std::int32_t>::min());
    require(signed_tree.sum(1, 2) == 1);
    signed_tree.set(0, std::numeric_limits<std::int32_t>::max());
    signed_tree.set(0, std::numeric_limits<std::int32_t>::min());
    require(signed_tree.get(0) == std::numeric_limits<std::int32_t>::min());
    require(signed_tree.values() == std::vector<std::int32_t>({std::numeric_limits<std::int32_t>::min(), 1}));
    fenwick_tree<std::int64_t> wide(2);
    wide.add(0, std::numeric_limits<std::int64_t>::max()); wide.add(1, 1);
    require(wide.sum(0, 2) == std::numeric_limits<std::int64_t>::min());
#if defined(__SIZEOF_INT128__)
    fenwick_tree<__int128> extended(2);
    extended.add(0, std::numeric_limits<__int128>::max()); extended.add(1, 1);
    require(extended.sum(0, 2) == std::numeric_limits<__int128>::min());
    fenwick_tree<unsigned __int128> extended_unsigned(2);
    extended_unsigned.add(0, ~static_cast<unsigned __int128>(0)); extended_unsigned.add(1, 1);
    require(extended_unsigned.sum(0, 2) == 0);
#endif
}
}
int main() {
    basic_calls(); random_reference(); integral_wrap();
    std::printf("ACL range API checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
