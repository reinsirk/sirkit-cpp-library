// SPDX-License-Identifier: CC0-1.0
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

#ifndef SIRKIT_CARTESIAN_TREE_SOURCE
#define SIRKIT_CARTESIAN_TREE_SOURCE "../../../Graph/Cartesian_Tree.cpp"
#endif
#include SIRKIT_CARTESIAN_TREE_SOURCE
#include <iostream>
template<class T> std::vector<T> saved(std::span<const T> values) { return {values.begin(), values.end()}; }
struct ComparisonFailure {};
void run_checks() {
    std::size_t failures = 0, comparison_failures = 0;
    const std::vector<int> values{8,3,6,1,9,2,4,0,7};
    for (int operation = 0; operation < 5; ++operation) {
        bool success = false;
        for (std::ptrdiff_t budget = 0; budget < 12; ++budget) {
            CartesianTree target(std::vector<int>{4,2,8,1,5}), source(values);
            const auto parents = saved(target.parents()), left = saved(target.left_children()), right = saved(target.right_children());
            const auto input = values; const auto root = target.root();
            const auto* pp = target.parents().data(); const auto* lp = target.left_children().data(); const auto* rp = target.right_children().data();
            const auto before = live_allocations.load(); int comparisons = 0; bool failed = false;
            auto compare = [&](int a,int b) { ++comparisons; return a < b; };
            allocation_countdown = budget;
            try {
                if (operation == 0) target.build(values, compare);
                if (operation == 1) target.build(std::span<const int>(values), compare);
                if (operation == 2) { auto copy = target; assert(copy.root() == root); }
                if (operation == 3) target = source;
                if (operation == 4) target.build(target.parents(), compare);
            } catch (const std::bad_alloc&) { failed = true; ++failures; }
            allocation_countdown = -1;
            if (failed) {
                assert(saved(target.parents()) == parents && saved(target.left_children()) == left && saved(target.right_children()) == right && target.root() == root);
                assert(target.parents().data() == pp && target.left_children().data() == lp && target.right_children().data() == rp);
                assert(comparisons == 0 && live_allocations.load() == before && values == input);
            } else { success = true; break; }
        }
        assert(success);
    }
    for (bool alias : {false,true}) {
        bool success = false;
        for (int budget = 0; budget < 40; ++budget) {
            CartesianTree target(std::vector<int>{4,2,8,1,5});
            const auto parents = saved(target.parents()), left = saved(target.left_children()), right = saved(target.right_children());
            const auto root = target.root(); const auto* pp = target.parents().data(); const auto* lp = target.left_children().data(); const auto* rp = target.right_children().data();
            const auto before = live_allocations.load(); int remaining = budget; bool failed = false;
            auto compare = [&](int a,int b) { if (remaining-- == 0) throw ComparisonFailure{}; return a < b; };
            try { if (alias) target.build(target.parents(), compare); else target.build(values, compare); }
            catch (const ComparisonFailure&) { failed = true; ++comparison_failures; }
            if (failed) {
                assert(saved(target.parents()) == parents && saved(target.left_children()) == left && saved(target.right_children()) == right && target.root() == root);
                assert(target.parents().data() == pp && target.left_children().data() == lp && target.right_children().data() == rp);
                assert(live_allocations.load() == before);
            } else { success = true; break; }
        }
        assert(success);
    }
    {
        CartesianTree tree(values), empty; auto* self = &tree;
        allocation_countdown = 0;
        tree = *self; tree = std::move(*self);
        CartesianTree moved(std::move(tree)); assert(tree.empty() && tree.root() == -1);
        tree = std::move(moved); assert(moved.empty() && moved.root() == -1);
        swap(tree, empty); empty.clear(); tree.build(std::span<const int>{}, [](int,int) { std::abort(); return false; });
        allocation_countdown = -1; assert(tree.empty() && tree.root() == -1);
    }
    std::cout << "Cartesian allocation failures=" << failures << " comparison failures=" << comparison_failures << '\n';
}
int main() {
    const auto before = live_allocations.load(); run_checks();
    assert(live_allocations.load() == before);
    std::cout << "live allocation delta=0\n";
}
