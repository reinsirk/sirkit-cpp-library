// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SEGMENT_TEST_SOURCE
#define SIRKIT_SEGMENT_TEST_SOURCE "../../../DataStructure/Segment_Tree.cpp"
#endif
#include SIRKIT_SEGMENT_TEST_SOURCE
#include SIRKIT_SEGMENT_TEST_SOURCE
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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
std::int64_t zero() { return 0; }
std::int64_t seven() { return 7; }
std::string empty_string() { return {}; }
bool true_value() { return true; }
struct Join { std::string operator()(const std::string& a, const std::string& b) const { return a + b; } };
struct First {
    int bias = 0;
    std::int64_t operator()(std::int64_t a, std::int64_t b) const { return a + b - bias; }
};
}
int main() {
    std::mt19937_64 random(0x9221733ULL);
    for (const auto n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 65, 128, 257}) {
        std::vector<std::int64_t> a(n);
        for (auto& x : a) x = static_cast<std::int64_t>(random() % 100);
        segtree<std::int64_t, std::plus<std::int64_t>{}, zero> tree(a);
        check(tree.size() == a.size()); check(tree.empty() == a.empty()); check(tree.values() == a);
        for (int rep = 0; rep < 1500; ++rep) {
            if (n && random() % 3 == 0) {
                const auto p = random() % n, x = random() % 100;
                if (random() & 1) tree.set(p, static_cast<std::int64_t>(x));
                else tree.transform(p, [&](auto) { return static_cast<std::int64_t>(x); });
                a[p] = static_cast<std::int64_t>(x); check(tree.get(p) == a[p]);
            }
            std::size_t l = random() % (n + 1), r = random() % (n + 1); if (l > r) std::swap(l, r);
            check(tree.prod(l, r) == std::accumulate(a.begin() + l, a.begin() + r, std::int64_t{}));
            check(tree.all_prod() == std::accumulate(a.begin(), a.end(), std::int64_t{}));
            const auto limit = static_cast<std::int64_t>(random() % 1000);
            std::size_t mr = l, ml = r; std::int64_t acc = 0;
            while (mr < a.size() && acc + a[mr] <= limit) acc += a[mr++];
            acc = 0; while (ml > 0 && a[ml - 1] + acc <= limit) acc += a[--ml];
            const auto pred = [&](auto value) { return value <= limit; };
            check(tree.max_right(l, pred) == mr); check(tree.min_left(r, pred) == ml);
            if ((rep & 63) == 0) check(tree.values() == a);
        }
        auto copied = tree; auto moved = std::move(copied); check(copied.empty()); check(moved.values() == a);
        copied = moved; moved = std::move(copied); check(copied.empty()); moved = std::move(moved); check(moved.values() == a);
        moved.reset(); check(moved.values() == std::vector<std::int64_t>(n)); moved.swap(tree); check(moved.values() == a);
        tree.clear(); check(tree.empty()); check(tree.prod(0, 0) == 0); check(tree.all_prod() == 0);
        tree.assign(a); check(tree.values() == a); tree.reset(n + 1); check(tree.values() == std::vector<std::int64_t>(n + 1));
        expect<std::out_of_range>([&] { tree.set(n + 1, 1); });
        expect<std::out_of_range>([&] { tree.set(-1, 1); });
        expect<std::out_of_range>([&] { (void)tree.get(-1); });
        expect<std::out_of_range>([&] { (void)tree.prod(-1, 0); });
        expect<std::out_of_range>([&] { (void)tree.prod(0, -1); });
        expect<std::out_of_range>([&] { (void)tree.max_right(-1, [](auto) { return true; }); });
        expect<std::out_of_range>([&] { (void)tree.min_left(-1, [](auto) { return true; }); });
        expect<std::out_of_range>([&] { (void)tree.prod(1, 0); });
        expect<std::out_of_range>([&] { (void)tree.max_right(n + 2, [](auto) { return true; }); });
        expect<std::invalid_argument>([&] { (void)tree.max_right(0, [](auto) { return false; }); });
        expect<std::invalid_argument>([&] { (void)tree.min_left(0, [](auto) { return false; }); });
    }
    for (int n = 0; n <= 67; ++n) {
        std::vector<std::string> words(n); for (auto& s : words) s = std::string(1, static_cast<char>('a' + random() % 26));
        segtree<std::string, Join{}, empty_string> ordered(words);
        for (std::size_t l = 0; l <= words.size(); ++l) for (auto r = l; r <= words.size(); ++r) {
            std::string expected; for (auto i = l; i < r; ++i) expected += words[i]; check(ordered.prod(l, r) == expected);
            auto valid = [&](const std::string& s) { return s.size() <= 5 && expected.starts_with(s); };
            std::size_t mr = l; std::string joined;
            while (mr < words.size() && valid(joined + words[mr])) joined += words[mr++];
            check(ordered.max_right(l, valid) == mr);
            auto suffix = [&](const std::string& s) { return s.size() <= 5 && expected.ends_with(s); };
            std::size_t ml = r; joined.clear();
            while (ml > 0 && suffix(words[ml - 1] + joined)) joined = words[--ml] + joined;
            check(ordered.min_left(r, suffix) == ml);
        }
    }
    segtree<bool, std::logical_and<bool>{}, true_value> bits(std::vector<bool>{true, false, true});
    check(!bits.prod(0, 3)); check(bits.get(0)); bits.set(1, true); check(bits.all_prod());
    segtree<std::int64_t, First{7}, seven> stateful(std::vector<std::int64_t>{8, 9, 10});
    check(stateful.prod(0, 3) == 13); auto moved = std::move(stateful); check(stateful.all_prod() == 7); stateful.reset(2); check(stateful.all_prod() == 7); check(moved.all_prod() == 13);
    expect<std::invalid_argument>([] { segtree<std::int64_t, std::plus<std::int64_t>{}, zero> invalid(-1); });
    std::printf("segtree checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
