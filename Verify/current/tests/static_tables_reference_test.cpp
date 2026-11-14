// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_SPARSE_TEST_SOURCE
#define SIRKIT_SPARSE_TEST_SOURCE "../../../DataStructure/Sparse_Table.cpp"
#endif
#ifndef SIRKIT_DISJOINT_TEST_SOURCE
#define SIRKIT_DISJOINT_TEST_SOURCE "../../../DataStructure/Disjoint_Sparse_Table.cpp"
#endif
#include SIRKIT_SPARSE_TEST_SOURCE
#include SIRKIT_DISJOINT_TEST_SOURCE
#include SIRKIT_SPARSE_TEST_SOURCE
#include SIRKIT_DISJOINT_TEST_SOURCE
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <string>
namespace {
std::uint64_t checks = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }
template<class Error, class F> void expect(F f) {
#ifndef SKIP_RUNTIME_CHECKS
    bool threw = false; try { f(); } catch (const Error&) { threw = true; } check(threw);
#else
    (void)f;
#endif
}
struct Minimum { std::int64_t operator()(std::int64_t a, std::int64_t b) const { return std::min(a, b); } };
struct Endpoints {
    int first = 0, last = 0; bool empty = true;
    friend bool operator==(const Endpoints&, const Endpoints&) = default;
};
struct OrderedIdempotent {
    Endpoints operator()(const Endpoints& a, const Endpoints& b) const {
        if (a.empty) return b;
        if (b.empty) return a;
        return {a.first, b.last, false};
    }
};
struct Affine {
    std::uint64_t a = 1, b = 0;
    friend bool operator==(const Affine&, const Affine&) = default;
};
struct Composition {
    static constexpr std::uint64_t modulus = 998244353;
    Affine operator()(const Affine& left, const Affine& right) const {
        return {left.a * right.a % modulus, (left.b * right.a + right.b) % modulus};
    }
};
struct Join { std::string operator()(const std::string& a, const std::string& b) const { return a + b; } };
}
int main() {
    std::mt19937_64 random(0x72012588ULL);
    const auto inf = std::numeric_limits<std::int64_t>::max();
    for (const auto n : {0, 1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257}) {
        std::vector<std::int64_t> a(n); for (auto& x : a) x = static_cast<std::int64_t>(random() % 100);
        SparseTable<std::int64_t, Minimum> sparse(a, inf);
        DisjointSparseTable<std::int64_t, std::plus<std::int64_t>> disjoint(a);
        check(sparse.size() == a.size()); check(disjoint.size() == a.size()); check(sparse.empty() == a.empty()); check(disjoint.empty() == a.empty());
        check(sparse.values() == a); check(disjoint.values() == a);
        for (std::size_t l = 0; l <= a.size(); ++l) {
            auto expected_min = inf; std::int64_t expected_sum = 0;
            for (auto r = l; r <= a.size(); ++r) {
                check(sparse.prod(l, r) == expected_min); check(disjoint.prod(l, r) == expected_sum);
                if (r < a.size()) { expected_min = std::min(expected_min, a[r]); expected_sum += a[r]; }
            }
            for (int rep = 0; rep < 12; ++rep) {
                const auto limit = static_cast<std::int64_t>(random() % 1500); std::size_t mr = l, ml = l; std::int64_t acc = 0;
                while (mr < a.size() && acc + a[mr] <= limit) acc += a[mr++];
                acc = 0; while (ml > 0 && a[ml - 1] + acc <= limit) acc += a[--ml];
                check(disjoint.max_right(l, [&](auto x) { return x <= limit; }) == mr);
                check(disjoint.min_left(l, [&](auto x) { return x <= limit; }) == ml);
                const auto minimum = static_cast<std::int64_t>(random() % 100); mr = l; ml = l;
                while (mr < a.size() && a[mr] >= minimum) ++mr;
                while (ml > 0 && a[ml - 1] >= minimum) --ml;
                check(sparse.max_right(l, [&](auto x) { return x >= minimum; }) == mr);
                check(sparse.min_left(l, [&](auto x) { return x >= minimum; }) == ml);
            }
        }
        if (n) { check(sparse.get(n - 1) == a.back()); check(disjoint.get(n - 1) == a.back()); }
        check(sparse.all_prod() == (n ? *std::min_element(a.begin(), a.end()) : inf));
        check(disjoint.all_prod() == std::accumulate(a.begin(), a.end(), std::int64_t{}));
        auto cs = sparse; auto ms = std::move(cs); check(cs.empty()); check(cs.all_prod() == inf); check(ms.values() == a); cs = ms; ms = std::move(cs); check(cs.empty()); ms = std::move(ms); check(ms.values() == a);
        auto cd = disjoint; auto md = std::move(cd); check(cd.empty()); check(cd.all_prod() == 0); check(md.values() == a); cd = md; md = std::move(cd); check(cd.empty()); md = std::move(md); check(md.values() == a);
        ms.clear(); md.clear(); check(ms.empty()); check(md.empty()); ms.assign(a); md.assign(a); check(ms.values() == a); check(md.values() == a);
        expect<std::out_of_range>([&] { (void)sparse.get(n); }); expect<std::out_of_range>([&] { (void)disjoint.prod(0, n + 1); });
        expect<std::out_of_range>([&] { (void)sparse.prod(1, 0); }); expect<std::out_of_range>([&] { (void)disjoint.min_left(n + 1, [](auto) { return true; }); });
        expect<std::invalid_argument>([&] { (void)sparse.max_right(0, [](auto) { return false; }); });
        expect<std::invalid_argument>([&] { (void)disjoint.min_left(0, [](auto) { return false; }); });
    }
    for (std::size_t n = 0; n <= 97; ++n) {
        std::vector<Endpoints> endpoints(n); std::vector<Affine> functions(n); std::vector<std::string> words(n);
        for (std::size_t i = 0; i < n; ++i) {
            endpoints[i] = {static_cast<int>(i), static_cast<int>(i + 10), false};
            functions[i] = {random() % Composition::modulus, random() % Composition::modulus};
            words[i] = std::string(1, static_cast<char>('a' + random() % 26));
        }
        SparseTable<Endpoints, OrderedIdempotent> a(endpoints);
        DisjointSparseTable<Endpoints, OrderedIdempotent> b(endpoints);
        DisjointSparseTable<Affine, Composition> affine(functions);
        DisjointSparseTable<std::string, Join> text(words);
        for (std::size_t l = 0; l <= n; ++l) {
            Affine f{}; Endpoints e{}; std::string s;
            for (auto r = l; r <= n; ++r) {
                check(a.prod(l, r) == e); check(b.prod(l, r) == e); check(affine.prod(l, r) == f); check(text.prod(l, r) == s);
                if (r != n) { e = OrderedIdempotent{}(e, endpoints[r]); f = Composition{}(f, functions[r]); s += words[r]; }
            }
        }
    }
    SparseTable<bool, std::logical_and<bool>> bits(std::vector<bool>{true, false, true}, true);
    DisjointSparseTable<bool, std::logical_and<bool>> other(std::vector<bool>{true, false, true}, true);
    check(!bits.prod(0, 3)); check(bits.get(0)); check(!other.all_prod()); check(other.get(2));
    std::printf("Static tables checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
