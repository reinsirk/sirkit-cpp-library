// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdexcept>
static std::ptrdiff_t allocation_countdown = -1;
static std::atomic<std::size_t> live_allocations{0};
static void allocation_check() {
    if (allocation_countdown == 0) throw std::bad_alloc();
    if (allocation_countdown > 0) --allocation_countdown;
}
void* operator new(std::size_t size) {
    allocation_check();
    if (void* p = std::malloc(size == 0 ? 1 : size)) { ++live_allocations; return p; }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { if (p) { --live_allocations; std::free(p); } }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }
void* operator new(std::size_t size, std::align_val_t alignment) {
    allocation_check();
    const auto align = static_cast<std::size_t>(alignment);
    const auto bytes = (size / align + 1) * align;
    if (void* p = std::aligned_alloc(align, bytes)) { ++live_allocations; return p; }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size, std::align_val_t alignment) { return ::operator new(size, alignment); }
void operator delete(void* p, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { ::operator delete(p); }

#ifndef PERSISTENT_SEGMENT_TREE_SOURCE
#define PERSISTENT_SEGMENT_TREE_SOURCE "../../../DataStructure/Persistent_Segment_Tree.cpp"
#endif
#ifndef PRUEFER_CODE_SOURCE
#define PRUEFER_CODE_SOURCE "../../../Graph/Pruefer_Code.cpp"
#endif
#include PERSISTENT_SEGMENT_TREE_SOURCE
#include PRUEFER_CODE_SOURCE
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

template<class F, class Check>
void sweep(F operation, Check check) {
    int failures = 0;
    for (std::ptrdiff_t at = 0; at < 1000; ++at) {
        allocation_countdown = at;
        bool failed = false;
        try { operation(); }
        catch (const std::bad_alloc&) { failed = true; ++failures; }
        allocation_countdown = -1;
        check(failed);
        if (!failed) break;
    }
    assert(failures > 0);
}

void run_allocation_checks() {
    using Tree = PersistentSegmentTree<std::int64_t, std::plus<std::int64_t>>;
    std::vector<std::int64_t> values(513); std::iota(values.begin(), values.end(), 0);
    // 毎試行作り直すので、成功済みのreserve容量が失敗経路を隠さない。
    for (int kind = 0; kind < 7; ++kind) {
        int failures = 0;
        for (std::ptrdiff_t at = 0; at < 100; ++at) {
            Tree tree(values, {}, 0);
            auto first = tree.latest_root();
            auto second = tree.set(first, 512, 999);
            if (kind == 1 || kind == 2) {
                for (;;) {
                    allocation_countdown = 0;
                    try { tree.set(first, 1, 33); }
                    catch (const std::bad_alloc&) { allocation_countdown = -1; break; }
                    allocation_countdown = -1;
                }
            }
            const auto count = tree.allocated_nodes();
            auto latest = tree.latest_root();
            allocation_countdown = at;
            bool failed = false;
            try {
                if (kind == 0) tree.build(values);
                if (kind == 1) tree.set(first, 1, 33);
                if (kind == 2) tree.transform(first, 1, [](auto x) { return x + 33; });
                if (kind == 3) tree.reserve(count * 3);
                if (kind == 4) tree.reset(5);
                if (kind == 5) { auto copied = tree; assert(copied.all_prod(copied.remap_from(tree, first)) == 131328); }
                if (kind == 6) tree.clear();
            } catch (const std::bad_alloc&) { failed = true; ++failures; }
            allocation_countdown = -1;
            if (failed || kind == 3 || kind == 5) {
                assert(tree.all_prod(first) == 131328);
                assert(tree.all_prod(second) == 131815);
            }
            if (failed) {
                assert(tree.size() == 513 && tree.allocated_nodes() == count && tree.latest_root() == latest);
            } else break;
        }
        assert(failures > 0);
    }
    // コピー代入先とコピー元の双方を維持する。
    Tree original(values, {}, 0), target(std::vector<std::int64_t>{7,8,9}, {}, 0);
    auto original_root = original.latest_root(), target_root = target.latest_root();
    sweep([&] { target = original; }, [&](bool failed) {
        assert(original.all_prod(original_root) == 131328);
        if (failed) assert(target.all_prod(target_root) == 24);
        else assert(target.all_prod(target.remap_from(original, original_root)) == 131328);
    });
    std::vector<int> code(100, 0); const auto saved_code = code;
    auto edges = pruefer_decode(code); const auto saved_edges = edges;
    auto degrees = pruefer_degrees(102, code); const auto saved_degrees = degrees;
    const auto check = [&](bool) { assert(code == saved_code && edges == saved_edges && degrees == saved_degrees); };
    sweep([&] { auto result = pruefer_decode(102, code); assert(result.size() == 101); }, check);
    sweep([&] { auto result = pruefer_encode(102, edges); assert(result == code); }, check);
    sweep([&] { auto result = pruefer_degrees(102, code); assert(result == degrees); }, check);
    sweep([&] { auto result = pruefer_from_degrees(degrees); assert(result && *result == code); }, check);
    sweep([&] { auto result = tree_from_degrees(degrees); assert(result && result->size() == 101); }, check);
    std::vector<std::pair<int,int>> forest(edges.begin(), edges.begin()+30); const auto saved_forest = forest;
    sweep([&] { auto result = complete_tree_with_degrees(degrees, forest); assert(result && result->size() == 71); },
          [&](bool failed) { check(failed); assert(forest == saved_forest); });
    std::cout << "all allocation rollback checks passed\n";
}

int main() {
    const auto before = live_allocations.load();
    run_allocation_checks();
    assert(live_allocations.load() == before);
    std::cout << "live allocation delta: " << live_allocations.load() - before << '\n';
}
