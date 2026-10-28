// SPDX-License-Identifier: CC0-1.0
#include <cstdlib>
#include <new>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include "../../../DataStructure/implicit_treap.cpp"

static int allocations_before_failure = -1;
void* operator new(std::size_t n) {
    if (allocations_before_failure == 0) throw std::bad_alloc();
    if (allocations_before_failure > 0) --allocations_before_failure;
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

using Tree = indexed_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;
using Snapshot = std::vector<std::pair<int, std::int64_t>>;

static Snapshot snapshot(Tree& tree) {
    Snapshot result;
    for (int i = 0; i < tree.size(); ++i)
        result.emplace_back(tree.get_id(i), tree.get(i).sum);
    return result;
}
static void check(Tree& tree, const Snapshot& expected) {
    assert(snapshot(tree) == expected);
    for (int label = 0; label < 200; ++label) {
        int position = -1;
        for (int i = 0; i < static_cast<int>(expected.size()); ++i)
            if (expected[i].first == label) position = i;
        assert(tree.position_of(label) == position);
        assert(tree.contains_id(label) == (position >= 0));
    }
}

int main() {
    int failed_copies = 0, successful_copies = 0, failed_inserts = 0;
    Tree source;
    for (int i = 0; i < 48; ++i) source.insert_at_with_id(i, 100 + i, {i, 1});
    source.apply(4, 40, 17);
    source.reverse(2, 45);
    const Snapshot expected_source = snapshot(source);
    for (bool reuse_node_capacity : {false, true}) {
        for (int budget = 0; budget < 6; ++budget) {
            Tree target;
            target.insert_at_with_id(0, 1, {100, 1});
            target.insert_after(1, 2, {200, 1});
            if (reuse_node_capacity) target.reserve(128);
            const Snapshot previous = snapshot(target);
            bool failed = false;
            allocations_before_failure = budget;
            try { target = source; } catch (const std::bad_alloc&) { failed = true; }
            allocations_before_failure = -1;
            check(target, failed ? previous : expected_source);
            check(source, expected_source);
            if (failed) ++failed_copies;
            else ++successful_copies;
            target.insert_at_with_id(0, 190, {19, 1});
            assert(target.position_of(190) == 0);
        }
    }
    for (int budget = 0; budget < 6; ++budget) {
        Tree tree;
        tree.reserve(1);
        tree.reserve_ids(1);
        tree.insert_at_with_id(0, 0, {7, 1});
        bool failed = false;
        allocations_before_failure = budget;
        try { tree.insert_after(0, 100, {9, 1}); }
        catch (const std::bad_alloc&) { failed = true; }
        allocations_before_failure = -1;
        if (failed) {
            ++failed_inserts;
            check(tree, {{0, 7}});
        } else check(tree, {{0, 7}, {100, 9}});
        tree.insert_after(0, 1, {11, 1});
        assert(tree.position_of(1) == 1);
    }
    bool capacity_rejected = false;
    try { source.reserve_ids(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 2); }
    catch (const std::length_error&) { capacity_rejected = true; }
    assert(capacity_rejected && failed_copies > 0 && successful_copies > 0 && failed_inserts > 0);
    std::cout << failed_copies << " copy failures, " << failed_inserts
              << " insertion failures: contents and IDs preserved\n";
}
