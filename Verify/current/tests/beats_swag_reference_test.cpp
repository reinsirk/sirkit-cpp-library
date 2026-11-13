// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Segment_Tree_Beats.cpp"
#include "../../../DataStructure/Sliding_Window_Aggregation.cpp"
#include <bit>
#include <cassert>
#include <deque>
#include <iostream>
#include <random>
#include <string>

using I = std::int64_t;
using W = __int128_t;
constexpr I low = std::numeric_limits<I>::min(), high = std::numeric_limits<I>::max();

void verify_range(SegmentTreeBeats& tree, const std::vector<I>& a, std::size_t l, std::size_t r) {
    W total = 0;
    for (auto i = l; i < r; ++i) total += a[i];
    bool overflow = total < low || total > high;
    try { const auto got = tree.sum(l, r); assert(!overflow && got == total); }
    catch (const std::overflow_error&) { assert(overflow); }
    if (l == r) { assert(!tree.min(l, r) && !tree.max(l, r)); }
    else {
        assert(tree.min(l, r) == *std::min_element(a.begin() + l, a.begin() + r));
        assert(tree.max(l, r) == *std::max_element(a.begin() + l, a.begin() + r));
    }
}
void verify_all(SegmentTreeBeats& tree, const std::vector<I>& a) {
    assert(tree.size() == a.size() && tree.empty() == a.empty());
    assert(tree.values() == a);
    for (std::size_t l = 0; l <= a.size(); ++l)
        for (auto r = l; r <= a.size(); ++r) verify_range(tree, a, l, r);
}
void beats_boundaries() {
    SegmentTreeBeats empty;
    verify_all(empty, {});
    empty.chmin(0, 0, low); empty.chmax(0, 0, high); empty.add(0, 0, high);
    const std::vector<std::vector<I>> cases = {{low}, {high}, {low, high}, {low, low}, {high, high}, {high, low, 0, 1, -1}, {low, high, low, high, low}};
    for (auto a : cases) {
        SegmentTreeBeats tree(a); verify_all(tree, a);
        tree.chmin(0, a.size(), 0); for (auto& v : a) v = std::min(v, I{0});
        tree.add(0, a.size(), high); for (auto& v : a) v += high;
        verify_all(tree, a);
        tree.chmax(0, a.size(), 0); for (auto& v : a) v = std::max(v, I{0});
        tree.add(0, a.size(), low); for (auto& v : a) v += low;
        verify_all(tree, a);
        tree.set(0, high); a[0] = high; verify_all(tree, a);
        auto copy = tree; copy.set(0, low); assert(tree.get(0) == high);
        SegmentTreeBeats moved(std::move(copy)); assert(copy.empty()); copy.reset({42}); assert(copy.get(0) == 42);
        copy = std::move(moved); assert(moved.empty());
        copy = copy; copy = std::move(copy); copy.clear(); verify_all(copy, {});
        tree.reset({low, high}); verify_all(tree, {low, high});
    }
#ifndef SKIP_RUNTIME_CHECKS
    try { empty.get(0); assert(false); } catch (const std::out_of_range&) {}
    try { empty.set(0, 0); assert(false); } catch (const std::out_of_range&) {}
    try { empty.min(1, 0); assert(false); } catch (const std::out_of_range&) {}
    try { empty.sum(0, 1); assert(false); } catch (const std::out_of_range&) {}
#endif
    // 同じ非一様ノードのaddを64bitの範囲外まで累積させ、再び戻す。
    SegmentTreeBeats lazy({low, low + 1, high - 1, high});
    lazy.chmin(0, 4, low + 1); lazy.add(0, 4, high);
    lazy.add(0, 4, high); verify_all(lazy, {high - 1, high, high, high});
    lazy.chmax(0, 4, high - 1); lazy.add(0, 4, low); lazy.add(0, 4, low + 2);
    verify_all(lazy, {low, low + 1, low + 1, low + 1});
}
void beats_random() {
    std::mt19937_64 rng(0xb75bea75);
    for (unsigned run = 0; run < 350; ++run) {
        const auto n = static_cast<std::size_t>(rng() % 65);
        std::vector<I> a(n);
        const bool full = (run % 3 == 0);
        for (auto& v : a) v = full ? std::bit_cast<I>(rng()) : static_cast<I>(rng() % 10001) - 5000;
        SegmentTreeBeats tree(a);
        for (unsigned step = 0; step < 1100; ++step) {
            std::size_t l = rng() % (n + 1), r = rng() % (n + 1);
            if (l > r) std::swap(l, r);
            I x = full ? std::bit_cast<I>(rng()) : static_cast<I>(rng() % 10001) - 5000;
            switch (rng() % 6) {
            case 0: tree.chmin(l, r, x); for (auto i = l; i < r; ++i) a[i] = std::min(a[i], x); break;
            case 1: tree.chmax(l, r, x); for (auto i = l; i < r; ++i) a[i] = std::max(a[i], x); break;
            case 2: {
                bool valid = true;
                for (auto i = l; i < r; ++i) valid &= W(a[i]) + x >= low && W(a[i]) + x <= high;
                try {
                    tree.add(l, r, x); assert(valid);
                    for (auto i = l; i < r; ++i) a[i] = static_cast<I>(W(a[i]) + x);
                } catch (const std::overflow_error&) { assert(!valid); assert(tree.values() == a); }
                break;
            }
            case 3: if (l < n) { tree.set(l, x); a[l] = x; } break;
            case 4: if (l < n) assert(tree.get(l) == a[l]); break;
            default: verify_range(tree, a, l, r); break;
            }
            verify_range(tree, a, l, r);
            if (step % 29 == 0) assert(tree.values() == a);
        }
        verify_all(tree, a);
    }
}
void beats_exhaustive() {
    const I v[] = {-2, -1, 0, 1, 2};
    for (I a0 : v) for (I a1 : v) {
        SegmentTreeBeats base({a0, a1});
        for (unsigned first = 0; first < 45; ++first) for (unsigned second = 0; second < 45; ++second) {
            auto tree = base; std::vector<I> a{a0, a1};
            for (auto code : {first, second}) {
                const auto range = code / 15, operation = code % 15 / 5;
                const I x = v[code % 5];
                const std::size_t l = range == 2 ? 1 : 0, r = range == 0 ? 1 : 2;
                if (operation == 0) { tree.chmin(l, r, x); for (auto i = l; i < r; ++i) a[i] = std::min(a[i], x); }
                if (operation == 1) { tree.chmax(l, r, x); for (auto i = l; i < r; ++i) a[i] = std::max(a[i], x); }
                if (operation == 2) { tree.add(l, r, x); for (auto i = l; i < r; ++i) a[i] += x; }
                verify_all(tree, a);
            }
        }
    }
}
struct Concat {
    std::string separator;
    std::string operator()(const std::string& a, const std::string& b) const {
        if (a.empty()) return b;
        if (b.empty()) return a;
        return a + separator + b;
    }
};
struct BoolOr { bool operator()(bool a, bool b) const { return a || b; } };
void swag_random() {
    SlidingWindowAggregation<std::string, Concat> q("", Concat{"/"});
    std::deque<std::string> a;
    std::mt19937_64 rng(0x5675a9);
    for (unsigned step = 0; step < 30000; ++step) {
        if (a.empty() || (rng() % 3 != 0 && a.size() < 180)) {
            const auto x = std::to_string(rng() % 100);
            q.push_back(x); a.push_back(x);
        } else { q.pop_front(); a.pop_front(); }
        std::string expected;
        for (const auto& s : a) { if (!expected.empty()) expected += '/'; expected += s; }
        assert(q.all_prod() == expected && q.size() == a.size() && q.empty() == a.empty());
        if (!a.empty()) assert(q.front() == a.front() && q.back() == a.back());
        if (step % 97 == 0) {
            auto copy = q; copy.push_back("copy"); assert(q.size() == a.size());
            auto moved = std::move(copy); assert(copy.empty()); copy.push_back("reuse"); assert(copy.all_prod() == "reuse");
            copy = moved; moved.clear(); assert(!copy.empty());
            copy = copy; copy = std::move(copy);
            copy.swap(moved); assert(copy.empty());
        }
    }
    while (!a.empty()) { q.pop_front(); a.pop_front(); }
    assert(q.all_prod().empty());
    q.push_back("alias"); q.push_back(q.front()); assert(q.all_prod() == "alias/alias");
    q.clear();
#ifndef SKIP_RUNTIME_CHECKS
    try { q.pop_front(); assert(false); } catch (const std::out_of_range&) {}
    try { q.front(); assert(false); } catch (const std::out_of_range&) {}
    try { q.back(); assert(false); } catch (const std::out_of_range&) {}
#endif
    SlidingWindowAggregation<bool, BoolOr> bits(false);
    bits.push_back(false); bits.push_back(true); assert(bits.front() == false && bits.back() == true && bits.all_prod());
    bits.pop_front(); assert(bits.front() && bits.all_prod()); bits.pop_front(); assert(!bits.all_prod());
}
int main() {
    beats_boundaries(); beats_exhaustive(); beats_random(); swag_random();
    std::cout << "beats_swag independent reference OK\n";
}
