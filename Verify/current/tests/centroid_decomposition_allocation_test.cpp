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

#ifndef CENTROID_DECOMPOSITION_SOURCE
#define CENTROID_DECOMPOSITION_SOURCE "../../../Graph/Centroid_Decomposition.cpp"
#endif
#include CENTROID_DECOMPOSITION_SOURCE
#include <iostream>
#include <utility>
#include <vector>

Graph make_graph(int n) {
    Graph graph(static_cast<std::size_t>(n));
    for (int vertex = 1; vertex < n; ++vertex) {
        const int parent = vertex / 3;
        graph[static_cast<std::size_t>(vertex)].push_back(parent);
        graph[static_cast<std::size_t>(parent)].push_back(vertex);
    }
    return graph;
}
void run_allocation_checks() {
    Graph graph = make_graph(130), small = make_graph(7);
    const auto saved_graph = graph;
    std::size_t failures = 0;
    for (int operation = 0; operation < 4; ++operation) {
        bool success = false;
        for (std::ptrdiff_t at = 0; at < 600; ++at) {
            CentroidDecomposition target(small), original(graph);
            const auto parents = target.parents(), source_parents = original.parents();
            const auto live_before = live_allocations.load();
            allocation_countdown = at;
            bool failed = false;
            try {
                if (operation == 0) target.build(graph);
                if (operation == 1) target.build(graph, false);
                if (operation == 2) target = original;
                if (operation == 3) { auto copy = original; assert(copy.parents() == source_parents); }
            } catch (const std::bad_alloc&) { failed = true; ++failures; }
            allocation_countdown = -1;
            assert(graph == saved_graph && original.parents() == source_parents);
            if (failed) {
                assert(target.parents() == parents && target.distance(0,6) == 2);
                assert(live_allocations.load() == live_before);
            } else {
                assert(target.parents() == (operation == 3 ? parents : source_parents));
                success = true; break;
            }
        }
        assert(success);
    }
    // 結果の所有値を作る途中の失敗も元の重心分解を変更しない。
    CentroidDecomposition cd(graph);
    auto parents = cd.parents(); bool success = false;
    for (std::ptrdiff_t at = 0; at < 400; ++at) {
        const auto before = live_allocations.load();
        allocation_countdown = at; bool failed = false;
        try { auto tree = cd.centroid_tree(); assert(tree.size() == graph.size()); }
        catch (const std::bad_alloc&) { failed = true; ++failures; }
        allocation_countdown = -1;
        assert(cd.parents() == parents && live_allocations.load() == before);
        if (!failed) { success = true; break; }
    }
    assert(success && failures > 0);
    std::cout << "centroid allocation failures: " << failures << '\n';
}
int main() {
    const auto before = live_allocations.load();
    run_allocation_checks();
    assert(live_allocations.load() == before);
    std::cout << "live allocation delta: " << live_allocations.load() - before << '\n';
}
