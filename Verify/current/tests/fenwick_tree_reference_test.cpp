// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_FENWICK_TEST_SOURCE
#define SIRKIT_FENWICK_TEST_SOURCE "../../../DataStructure/Fenwick_Tree.cpp"
#endif
#include SIRKIT_FENWICK_TEST_SOURCE
#include SIRKIT_FENWICK_TEST_SOURCE
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <type_traits>

namespace {
std::uint64_t checks = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }
template<class F> void expect_range(F f) {
#ifndef SKIP_RUNTIME_CHECKS
    bool threw = false; try { f(); } catch (const std::out_of_range&) { threw = true; } check(threw);
#else
    (void)f;
#endif
}
}
int main() {
    std::mt19937_64 random(0x719a082ULL);
    for (const auto n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 65, 128, 257}) {
        std::vector<std::int64_t> a(n);
        for (auto& x : a) x = static_cast<std::int64_t>(random() % 100);
        fenwick_tree<std::int64_t> tree(a);
        check(tree.size() == a.size()); check(tree.empty() == a.empty()); check(tree.values() == a);
        for (int rep = 0; rep < 1500; ++rep) {
            if (n && random() % 3 == 0) {
                const auto p = random() % n; const auto x = static_cast<std::int64_t>(random() % 100);
                if (random() & 1) { tree.add(p, x - a[p]); a[p] = x; }
                else { tree.set(p, x); a[p] = x; }
                check(tree.get(p) == a[p]);
            }
            std::size_t l = random() % (n + 1), r = random() % (n + 1); if (l > r) std::swap(l, r);
            check(tree.sum(l, r) == std::accumulate(a.begin() + l, a.begin() + r, std::int64_t{}));
            check(tree.sum(0, r) == std::accumulate(a.begin(), a.begin() + r, std::int64_t{}));
            const auto total = std::accumulate(a.begin(), a.end(), std::int64_t{}); check(tree.sum(0, tree.size()) == total);
            const auto target = static_cast<std::int64_t>(random() % (static_cast<std::uint64_t>(total) + 4)) - 1;
            std::size_t lo = 0, hi = 0; std::int64_t value = 0;
            while (lo < a.size()) { value += a[lo]; if (value >= target) break; ++lo; }
            if (target <= 0) lo = 0;
            value = 0;
            while (hi < a.size()) { value += a[hi]; if (value > target) break; ++hi; }
            if (target < 0) hi = 0;
            check(tree.lower_bound(target) == lo); check(tree.upper_bound(target) == hi);
            if ((rep & 63) == 0) check(tree.values() == a);
        }
        auto copied = tree; auto moved = std::move(copied); check(copied.empty()); check(moved.values() == a);
        copied = moved; check(copied.values() == a); moved = std::move(copied); check(copied.empty());
        moved = std::move(moved); check(moved.values() == a);
        moved.reset(); check(moved.values() == std::vector<std::int64_t>(n));
        moved.swap(tree); check(moved.values() == a); tree.clear(); check(tree.empty()); check(tree.sum(0, 0) == 0);
        tree.reset(n); check(tree.values() == std::vector<std::int64_t>(n));
        expect_range([&] { tree.add(n, 1); }); expect_range([&] { (void)tree.get(n); });
        expect_range([&] { tree.add(-1, 1); }); expect_range([&] { (void)tree.get(-1); });
        expect_range([&] { (void)tree.sum(-1, 0); }); expect_range([&] { (void)tree.sum(0, -1); });
        expect_range([&] { (void)tree.sum(1, 0); }); expect_range([&] { (void)tree.sum(0, n + 1); });
    }
    fenwick_tree<std::uint32_t> wrapped(std::vector<std::uint32_t>{0xffffffffU, 2, 0x80000000U});
    check(wrapped.sum(0, 2) == 1U); check(wrapped.values() == std::vector<std::uint32_t>({0xffffffffU, 2, 0x80000000U}));
#ifndef SKIP_RUNTIME_CHECKS
    bool invalid = false; try { fenwick_tree<int> impossible(-1); } catch (const std::exception&) { invalid = true; } check(invalid);
#endif
    std::printf("fenwick_tree checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
