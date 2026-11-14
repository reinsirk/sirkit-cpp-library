// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
// ACL本体は独立参照としてのみ使用する。自作3実装との公開API・結果の照合。
#include "../../../DataStructure/Fenwick_Tree.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include "../../../DataStructure/Lazy_Segment_Tree.cpp"
#include "../vendor/atcoder/fenwicktree.hpp"
#include "../vendor/atcoder/segtree.hpp"
#include "../vendor/atcoder/lazysegtree.hpp"
#include "../vendor/atcoder/modint.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace {
std::uint64_t checks = 0;
void check(bool condition) { ++checks; if (!condition) std::abort(); }
std::mt19937_64 random(0x4815039a217ULL);
int bound = 0;
std::int64_t numeric_bound = 0;

std::int64_t add(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t zero() { return 0; }
bool sum_fits(std::int64_t value) { return value <= numeric_bound; }
std::string join(std::string a, std::string b) { return a + b; }
std::string empty_word() { return {}; }
bool length_fits(std::string value) { return value.size() <= static_cast<std::size_t>(bound); }

struct Sum {
    std::int64_t value = 0;
    int length = 0;
    friend bool operator==(const Sum&, const Sum&) = default;
};
struct Affine { std::int64_t a = 1, b = 0; };
Sum sum_op(Sum a, Sum b) { return {a.value + b.value, a.length + b.length}; }
Sum sum_e() { return {}; }
Sum map_affine(Affine f, Sum value) { return {f.a * value.value + f.b * value.length, value.length}; }
Affine compose(Affine newer, Affine older) { return {newer.a * older.a, newer.a * older.b + newer.b}; }
Affine identity() { return {}; }
bool aggregate_fits(Sum value) { return value.value <= numeric_bound; }

using OwnSum = ::sirkit::segtree<std::int64_t, add, zero>;
using RefSum = atcoder::segtree<std::int64_t, add, zero>;
using OwnWords = ::sirkit::segtree<std::string, join, empty_word>;
using RefWords = atcoder::segtree<std::string, join, empty_word>;
using OwnLazy = ::sirkit::lazy_segtree<Sum, sum_op, sum_e, Affine, map_affine, compose, identity>;
using RefLazy = atcoder::lazy_segtree<Sum, sum_op, sum_e, Affine, map_affine, compose, identity>;

template<class Mint>
void modular_fenwick_case() {
    std::vector<Mint> values(17);
    for (auto& value : values) value = Mint(random());
    ::sirkit::fenwick_tree<Mint> ours(values);
    atcoder::fenwick_tree<Mint> reference(17);
    for (int i = 0; i < 17; ++i) reference.add(i, values[i]);
    for (int repeat = 0; repeat < 1000; ++repeat) {
        const int p = static_cast<int>(random() % 17);
        const Mint value(random());
        if (repeat & 1) {
            ours.add(p, value); reference.add(p, value); values[p] += value;
        } else {
            ours.set(p, value); reference.add(p, value - values[p]); values[p] = value;
        }
        int l = static_cast<int>(random() % 18), r = static_cast<int>(random() % 18);
        if (l > r) std::swap(l, r);
        check(ours.sum(l, r) == reference.sum(l, r));
        check(ours.get(p) == values[p]);
        if (repeat % 31 == 0) check(ours.values() == values);
    }
}

// signed型も型幅を超える更新・部分和を含め、ACLのmod 2^bitの意味を照合する。
template<class T>
void fenwick_case(int n) {
    ::sirkit::fenwick_tree<T> ours(n);
    atcoder::fenwick_tree<T> reference(n);
    check(ours.sum(0, n) == reference.sum(0, n));
    for (int repeat = 0; repeat < 1800; ++repeat) {
        if (n && (random() & 1)) {
            const auto position = static_cast<int>(random() % static_cast<unsigned>(n));
            const T delta = static_cast<T>(random());
            ours.add(position, delta); reference.add(position, delta);
        }
        int l = static_cast<int>(random() % static_cast<unsigned>(n + 1));
        int r = static_cast<int>(random() % static_cast<unsigned>(n + 1));
        if (l > r) std::swap(l, r);
        check(ours.sum(l, r) == reference.sum(l, r));
        check(ours.sum(0, r) == reference.sum(0, r));
        check(ours.sum(l, n) == reference.sum(l, n));
    }
}

void segment_case(int n) {
    std::vector<std::int64_t> values(n);
    std::vector<std::string> words(n);
    for (int i = 0; i < n; ++i) {
        values[i] = static_cast<std::int64_t>(random() % 32);
        words[i] = std::string(random() % 4, static_cast<char>('a' + random() % 4));
    }
    OwnSum ours(values); RefSum reference(values);
    OwnWords own_words(words); RefWords ref_words(words);
    for (int repeat = 0; repeat < 1200; ++repeat) {
        if (n && random() % 3 == 0) {
            const int position = static_cast<int>(random() % static_cast<unsigned>(n));
            const auto value = static_cast<std::int64_t>(random() % 32);
            const auto word = std::string(random() % 4, static_cast<char>('a' + random() % 4));
            ours.set(position, value); reference.set(position, value);
            own_words.set(position, word); ref_words.set(position, word);
            check(ours.get(position) == reference.get(position));
            check(own_words.get(position) == ref_words.get(position));
        }
        int l = static_cast<int>(random() % static_cast<unsigned>(n + 1));
        int r = static_cast<int>(random() % static_cast<unsigned>(n + 1));
        if (l > r) std::swap(l, r);
        check(ours.prod(l, r) == reference.prod(l, r));
        check(ours.all_prod() == reference.all_prod());
        check(own_words.prod(l, r) == ref_words.prod(l, r));
        check(own_words.all_prod() == ref_words.all_prod());
        numeric_bound = static_cast<std::int64_t>(random() % static_cast<std::uint64_t>(ours.all_prod() + 1));
        check(ours.max_right<sum_fits>(l) == reference.max_right<sum_fits>(l));
        check(ours.min_left<sum_fits>(r) == reference.min_left<sum_fits>(r));
        const auto numeric_predicate = [limit = numeric_bound](std::int64_t value) { return value <= limit; };
        check(ours.max_right(l, numeric_predicate) == reference.max_right(l, numeric_predicate));
        check(ours.min_left(r, numeric_predicate) == reference.min_left(r, numeric_predicate));
        bound = static_cast<int>(random() % (own_words.all_prod().size() + 1));
        check(own_words.max_right<length_fits>(l) == ref_words.max_right<length_fits>(l));
        check(own_words.min_left<length_fits>(r) == ref_words.min_left<length_fits>(r));
        const auto word_predicate = [limit = bound](const std::string& value) {
            return value.size() <= static_cast<std::size_t>(limit);
        };
        check(own_words.max_right(l, word_predicate) == ref_words.max_right(l, word_predicate));
        check(own_words.min_left(r, word_predicate) == ref_words.min_left(r, word_predicate));
    }
}

void lazy_case(int n) {
    std::vector<Sum> values(n);
    for (auto& value : values) value = {static_cast<std::int64_t>(random() % 32), 1};
    OwnLazy ours(values); RefLazy reference(values);
    for (int repeat = 0; repeat < 1800; ++repeat) {
        int l = static_cast<int>(random() % static_cast<unsigned>(n + 1));
        int r = static_cast<int>(random() % static_cast<unsigned>(n + 1));
        if (l > r) std::swap(l, r);
        const auto kind = random() % 4;
        const Affine action{static_cast<std::int64_t>(random() % 2), static_cast<std::int64_t>(random() % 6)};
        if (kind == 0) { ours.apply(l, r, action); reference.apply(l, r, action); }
        else if (kind == 1 && n) {
            const int position = static_cast<int>(random() % static_cast<unsigned>(n));
            ours.apply(position, action); reference.apply(position, action);
        } else if (kind == 2 && n) {
            const int position = static_cast<int>(random() % static_cast<unsigned>(n));
            const Sum value{static_cast<std::int64_t>(random() % 32), 1};
            ours.set(position, value); reference.set(position, value);
        }
        check(ours.prod(l, r) == reference.prod(l, r));
        check(ours.all_prod() == reference.all_prod());
        if (n) {
            const int position = static_cast<int>(random() % static_cast<unsigned>(n));
            check(ours.get(position) == reference.get(position));
        }
        numeric_bound = static_cast<std::int64_t>(random() % static_cast<std::uint64_t>(ours.all_prod().value + 1));
        check(ours.max_right<aggregate_fits>(l) == reference.max_right<aggregate_fits>(l));
        check(ours.min_left<aggregate_fits>(r) == reference.min_left<aggregate_fits>(r));
        const auto predicate = [limit = numeric_bound](Sum value) { return value.value <= limit; };
        check(ours.max_right(l, predicate) == reference.max_right(l, predicate));
        check(ours.min_left(r, predicate) == reference.min_left(r, predicate));
    }
}

void empty_and_size_constructors() {
    ::sirkit::fenwick_tree<std::int64_t> own_fenwick;
    atcoder::fenwick_tree<std::int64_t> ref_fenwick;
    check(own_fenwick.sum(0, 0) == ref_fenwick.sum(0, 0));
    OwnSum own_segment; RefSum ref_segment;
    OwnLazy own_lazy; RefLazy ref_lazy;
    check(own_segment.all_prod() == ref_segment.all_prod());
    check(own_segment.prod(0, 0) == ref_segment.prod(0, 0));
    check(own_lazy.all_prod() == ref_lazy.all_prod());
    check(own_lazy.prod(0, 0) == ref_lazy.prod(0, 0));
    numeric_bound = 0;
    check(own_segment.max_right<sum_fits>(0) == ref_segment.max_right<sum_fits>(0));
    check(own_segment.min_left<sum_fits>(0) == ref_segment.min_left<sum_fits>(0));
    check(own_lazy.max_right<aggregate_fits>(0) == ref_lazy.max_right<aggregate_fits>(0));
    check(own_lazy.min_left<aggregate_fits>(0) == ref_lazy.min_left<aggregate_fits>(0));
    for (const int n : {1, 3, 8, 17}) {
        OwnSum ours(n); RefSum reference(n);
        OwnLazy lazy(n); RefLazy lazy_reference(n);
        check(ours.all_prod() == reference.all_prod());
        lazy.apply(0, n, Affine{0, 3}); lazy_reference.apply(0, n, Affine{0, 3});
        check(lazy.all_prod() == lazy_reference.all_prod());
        for (int i = 0; i < n; ++i) {
            const Sum value{i + 1, 1};
            ours.set(i, i + 1); reference.set(i, i + 1);
            lazy.set(i, value); lazy_reference.set(i, value);
        }
        lazy.apply(0, n, Affine{1, 2}); lazy_reference.apply(0, n, Affine{1, 2});
        check(ours.prod(0, n) == reference.prod(0, n));
        check(lazy.prod(0, n) == lazy_reference.prod(0, n));
    }
}
}

int main() {
    empty_and_size_constructors();
    modular_fenwick_case<atcoder::modint998244353>();
    atcoder::dynamic_modint<7>::set_mod(1000000007);
    modular_fenwick_case<atcoder::dynamic_modint<7>>();
    for (const int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 127, 128, 129, 257}) {
        fenwick_case<std::int32_t>(n); fenwick_case<std::uint32_t>(n);
        fenwick_case<std::int64_t>(n); fenwick_case<std::uint64_t>(n);
        segment_case(n); lazy_case(n);
    }
    std::printf("ACL core API cross-checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
