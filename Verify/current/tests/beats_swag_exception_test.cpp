// SPDX-License-Identifier: CC0-1.0
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <cinttypes>
#include <stdexcept>
#include "../../../DataStructure/Segment_Tree_Beats.cpp"
#include "../../../DataStructure/Sliding_Window_Aggregation.cpp"
#include "../../../Math/Xor_Basis.cpp"

namespace {
std::int64_t allocations_live = 0;
std::int64_t fail_after = -1;
}
void* operator new(std::size_t size) {
    if (fail_after == 0) throw std::bad_alloc();
    if (fail_after > 0) --fail_after;
    if (void* p = std::malloc(size == 0 ? 1 : size)) { ++allocations_live; return p; }
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { if (p) { --allocations_live; std::free(p); } }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }

struct Tracked {
    static inline int live = 0, copies_before_throw = -1;
    int value;
    explicit Tracked(int x = 0) : value(x) { ++live; }
    Tracked(const Tracked& other) : value(other.value) {
        if (copies_before_throw == 0) throw std::runtime_error("copy");
        if (copies_before_throw > 0) --copies_before_throw;
        ++live;
    }
    Tracked(Tracked&& other) noexcept(false) : Tracked(static_cast<const Tracked&>(other)) {}
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) = default;
    ~Tracked() { --live; }
};
struct Plus {
    static inline int calls_before_throw = -1;
    Tracked operator()(const Tracked& a, const Tracked& b) const {
        if (calls_before_throw == 0) throw std::runtime_error("op");
        if (calls_before_throw > 0) --calls_before_throw;
        return Tracked(a.value + b.value);
    }
};
void verify(const SlidingWindowAggregation<Tracked, Plus>& q, int n) {
    assert(q.size() == static_cast<unsigned>(n));
    assert(q.all_prod().value == n * (n + 1) / 2);
    if (n != 0) assert(q.front().value == 1 && q.back().value == n);
}
void swag_failures() {
    using Queue = SlidingWindowAggregation<Tracked, Plus>;
    for (int budget = 0; budget < 30; ++budget) {
        Queue q(Tracked(0)); for (int i = 1; i <= 8; ++i) q.push_back(Tracked(i));
        Tracked::copies_before_throw = budget;
        bool threw = false;
        try { q.push_back(q.front()); } catch (const std::runtime_error&) { threw = true; }
        Tracked::copies_before_throw = -1;
        if (threw) verify(q, 8);
        else assert(q.size() == 9 && q.back().value == 1 && q.all_prod().value == 37);
    }
    for (int budget = 0; budget < 40; ++budget) {
        Queue q(Tracked(0)); for (int i = 1; i <= 8; ++i) q.push_back(Tracked(i));
        Tracked::copies_before_throw = budget;
        bool threw = false;
        try { q.pop_front(); } catch (const std::runtime_error&) { threw = true; }
        Tracked::copies_before_throw = -1;
        if (threw) verify(q, 8);
        else assert(q.size() == 7 && q.front().value == 2 && q.back().value == 8 && q.all_prod().value == 35);
    }
    for (int budget = 0; budget < 10; ++budget) {
        Queue q(Tracked(0)); for (int i = 1; i <= 8; ++i) q.push_back(Tracked(i));
        Plus::calls_before_throw = budget;
        bool threw = false;
        try { q.pop_front(); } catch (const std::runtime_error&) { threw = true; }
        Plus::calls_before_throw = -1;
        if (threw) verify(q, 8);
        else assert(q.size() == 7 && q.all_prod().value == 35);
    }
    Queue q(Tracked(0)); for (int i = 1; i <= 8; ++i) q.push_back(Tracked(i));
    fail_after = 0;
    try { q.pop_front(); assert(false); } catch (const std::bad_alloc&) {}
    fail_after = -1; verify(q, 8);
    fail_after = 0;
    try { Queue copy(q); assert(false); } catch (const std::bad_alloc&) {}
    fail_after = -1; verify(q, 8);
    for (int budget = 0; budget < 15; ++budget) {
        Tracked::copies_before_throw = budget;
        try { Queue copy(q); } catch (const std::runtime_error&) {}
        Tracked::copies_before_throw = -1; verify(q, 8);
    }
    q.pop_front(); q.push_back(Tracked(9));
    Plus::calls_before_throw = 0;
    try { q.all_prod(); assert(false); } catch (const std::runtime_error&) {}
    Plus::calls_before_throw = -1;
    assert(q.size() == 8 && q.front().value == 2 && q.back().value == 9 && q.all_prod().value == 44);
}
void xor_no_allocations() {
    fail_after = 0;
    XorBasis a, b;
    for (unsigned bit = 0; bit < 64; ++bit) a.insert(std::uint64_t{1} << bit);
    b.merge(a); a.swap(b); a.merge(a);
    assert(a.rank() == 64 && a.has_nonempty_zero());
    assert(a.kth(123) == 123 && a.contains(123) && a.contains_nonempty(0));
    assert(a.max_xor(123) == std::numeric_limits<std::uint64_t>::max() && a.min_xor(123) == 0);
    a.clear(); assert(a.rank() == 0);
    fail_after = -1;
}
void beats_failures() {
    SegmentTreeBeats tree({1, 2, 3, 4});
    const std::vector<std::int64_t> old{1, 2, 3, 4}, next(1000, 42);
    fail_after = 0;
    try { tree.reset(next); assert(false); } catch (const std::bad_alloc&) {}
    fail_after = -1; assert(tree.values() == old);
    SegmentTreeBeats source(next);
    fail_after = 0;
    try { tree = source; assert(false); } catch (const std::bad_alloc&) {}
    fail_after = -1; assert(tree.values() == old && source.values() == next);
    fail_after = 0;
    try { tree.values(); assert(false); } catch (const std::bad_alloc&) {}
    fail_after = -1; assert(tree.values() == old);
    // 更新・範囲取得は確保しない。
    fail_after = 0;
    tree.chmin(0, 4, 3); tree.chmax(0, 2, 2); tree.add(1, 4, 5); tree.set(2, -8);
    assert(tree.sum(0, 4) == 9 && tree.min(0, 4) == -8 && tree.max(0, 4) == 8 && tree.get(2) == -8);
    fail_after = -1;
}
int main() {
    assert(allocations_live == 0);
    { beats_failures(); swag_failures(); xor_no_allocations(); }
    assert(Tracked::live == 0);
    assert(allocations_live == 0);
    std::printf("normal_live0=%" PRId64 " tracked_live0=%d; allocation/copy/op failure OK\n", allocations_live, Tracked::live);
}
