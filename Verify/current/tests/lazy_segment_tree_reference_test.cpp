// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#define SIRKIT_LAZY_SEGMENT_TEST_SOURCE "../../../DataStructure/Lazy_Segment_Tree.cpp"
#endif
#include SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#include SIRKIT_LAZY_SEGMENT_TEST_SOURCE
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
struct Affine { std::int64_t a = 1, b = 0; };
struct Mapping {
    std::int64_t operator()(const Affine& f, std::int64_t value, std::size_t length) const {
        return f.a * value + f.b * static_cast<std::int64_t>(length);
    }
};
struct Compose {
    Affine operator()(const Affine& newer, const Affine& older) const {
        return {newer.a * older.a, newer.a * older.b + newer.b};
    }
};
struct Assign { bool active = false; char value = ' '; };
struct AssignMapping {
    std::string operator()(const Assign& f, const std::string& value, std::size_t length) const {
        return f.active ? std::string(length, f.value) : value;
    }
};
struct AssignCompose { Assign operator()(const Assign& newer, const Assign& older) const { return newer.active ? newer : older; } };
struct Xor { bool operator()(bool a, bool b) const { return a != b; } };
struct XorMapping { bool operator()(bool f, bool value, std::size_t length) const { return value != (f && (length & 1)); } };
std::int64_t zero() { return 0; }
Affine affine_identity() { return {}; }
std::string empty_string() { return {}; }
Assign assign_identity() { return {}; }
bool false_value() { return false; }
using Tree = lazy_segtree<std::int64_t, std::plus<std::int64_t>{}, zero,
                          Affine, Mapping{}, Compose{}, affine_identity>;
}
int main() {
    std::mt19937_64 random(0x19609441ULL);
    for (const auto n : {0, 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 128, 129, 257}) {
        std::vector<std::int64_t> a(n); for (auto& x : a) x = static_cast<std::int64_t>(random() % 100);
        Tree tree(a); check(tree.size() == a.size()); check(tree.values() == a);
        for (int rep = 0; rep < 2500; ++rep) {
            std::size_t l = random() % (n + 1), r = random() % (n + 1); if (l > r) std::swap(l, r);
            const Affine f{static_cast<std::int64_t>(random() % 2), static_cast<std::int64_t>(random() % 10)};
            switch (random() % 7) {
            case 0: case 1:
                tree.apply(l, r, f); for (auto i = l; i < r; ++i) a[i] = f.a * a[i] + f.b; break;
            case 2:
                if (n) { const auto p = random() % n; tree.apply(p, f); a[p] = f.a * a[p] + f.b; }
                break;
            case 3:
                if (n) { const auto p = random() % n; const auto value = static_cast<std::int64_t>(random() % 100); tree.set(p, value); a[p] = value; }
                break;
            case 4:
                tree.apply(0, tree.size(), f); for (auto& x : a) x = f.a * x + f.b; break;
            default: break;
            }
            check(tree.prod(l, r) == std::accumulate(a.begin() + l, a.begin() + r, std::int64_t{}));
            check(tree.all_prod() == std::accumulate(a.begin(), a.end(), std::int64_t{}));
            if (n) { const auto p = random() % n; check(tree.get(p) == a[p]); }
            const auto limit = static_cast<std::int64_t>(random() % 1000); std::size_t mr = l, ml = r; std::int64_t current = 0;
            while (mr < a.size() && current + a[mr] <= limit) current += a[mr++];
            current = 0; while (ml > 0 && a[ml - 1] + current <= limit) current += a[--ml];
            check(tree.max_right(l, [&](auto value) { return value <= limit; }) == mr);
            check(tree.min_left(r, [&](auto value) { return value <= limit; }) == ml);
            if ((rep & 63) == 0) check(tree.values() == a);
        }
        tree.apply(0, tree.size(), Affine{0, 7}); std::fill(a.begin(), a.end(), 7);
        auto copied = tree; auto moved = std::move(copied); check(copied.empty()); check(copied.all_prod() == 0); check(moved.values() == a); check(tree.values() == a);
        copied = tree; moved = std::move(copied); check(copied.empty()); moved = std::move(moved); check(moved.values() == a);
        moved.reset(); check(moved.values() == std::vector<std::int64_t>(n)); moved.swap(tree); check(moved.values() == a);
        tree.clear(); check(tree.empty()); check(tree.prod(0, 0) == 0); check(tree.all_prod() == 0);
        tree.assign(a); check(tree.values() == a); tree.reset(n + 1); check(tree.values() == std::vector<std::int64_t>(n + 1));
        expect<std::out_of_range>([&] { tree.apply(n + 1, Affine{}); }); expect<std::out_of_range>([&] { (void)tree.prod(1, 0); });
        expect<std::out_of_range>([&] { tree.set(-1, 0); });
        expect<std::out_of_range>([&] { (void)tree.get(-1); });
        expect<std::out_of_range>([&] { tree.apply(-1, Affine{}); });
        expect<std::out_of_range>([&] { tree.apply(-1, 0, Affine{}); });
        expect<std::out_of_range>([&] { (void)tree.prod(0, -1); });
        expect<std::out_of_range>([&] { (void)tree.max_right(-1, [](auto) { return true; }); });
        expect<std::out_of_range>([&] { (void)tree.min_left(-1, [](auto) { return true; }); });
        expect<std::out_of_range>([&] { (void)tree.max_right(n + 2, [](auto) { return true; }); });
        expect<std::invalid_argument>([&] { (void)tree.max_right(0, [](auto) { return false; }); });
        expect<std::invalid_argument>([&] { (void)tree.min_left(0, [](auto) { return false; }); });
    }
    for (const std::size_t n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 65}) {
        std::vector<std::string> words(n); for (auto& s : words) s = std::string(1, static_cast<char>('a' + random() % 26));
        lazy_segtree<std::string, std::plus<std::string>{}, empty_string, Assign, AssignMapping{}, AssignCompose{}, assign_identity> tree(words);
        for (int rep = 0; rep < 700; ++rep) {
            std::size_t l = random() % (n + 1), r = random() % (n + 1); if (l > r) std::swap(l, r);
            if (random() % 2 == 0) {
                const char c = static_cast<char>('a' + random() % 26); tree.apply(l, r, Assign{true, c});
                for (auto i = l; i < r; ++i) words[i] = std::string(1, c);
            }
            std::string expected; for (auto i = l; i < r; ++i) expected += words[i]; check(tree.prod(l, r) == expected);
            check(tree.max_right(l, [&](const std::string& s) { return expected.starts_with(s); }) == r);
            check(tree.min_left(r, [&](const std::string& s) { return expected.ends_with(s); }) == l);
            if ((rep & 31) == 0) check(tree.values() == words);
        }
    }
    std::vector<bool> bits{true, false, true, false, false};
    lazy_segtree<bool, Xor{}, false_value, bool, XorMapping{}, Xor{}, false_value> binary(bits);
    for (std::size_t l = 0; l <= bits.size(); ++l) for (auto r = l; r <= bits.size(); ++r) {
        binary.apply(l, r, true); for (auto i = l; i < r; ++i) bits[i] = !bits[i];
        bool expected = false; for (const auto bit : bits) expected = expected != bit;
        check(binary.all_prod() == expected); check(binary.values() == bits);
    }
    expect<std::invalid_argument>([] { Tree invalid(-1); });
    std::printf("lazy_segtree checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
