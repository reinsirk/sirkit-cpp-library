// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <new>
#include <string>
#include <vector>
namespace {
std::atomic<std::ptrdiff_t> budget{-1};
std::atomic<std::size_t> live{0}, requests{0};
std::uint64_t checks = 0, failures = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }
}
void* operator new(std::size_t size) {
    ++requests;
    if (budget == 0) throw std::bad_alloc();
    if (budget > 0) --budget;
    if (void* p = std::malloc(size == 0 ? 1 : size)) { ++live; return p; }
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { if (p) { --live; std::free(p); } }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }
#ifndef SIRKIT_FENWICK_TEST_SOURCE
#define SIRKIT_FENWICK_TEST_SOURCE "../../../DataStructure/Fenwick_Tree.cpp"
#endif
#ifndef SIRKIT_SEGMENT_TEST_SOURCE
#define SIRKIT_SEGMENT_TEST_SOURCE "../../../DataStructure/Segment_Tree.cpp"
#endif
#ifndef SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#define SIRKIT_LAZY_SEGMENT_TEST_SOURCE "../../../DataStructure/Lazy_Segment_Tree.cpp"
#endif
#ifndef SIRKIT_SPARSE_TEST_SOURCE
#define SIRKIT_SPARSE_TEST_SOURCE "../../../DataStructure/Sparse_Table.cpp"
#endif
#ifndef SIRKIT_DISJOINT_TEST_SOURCE
#define SIRKIT_DISJOINT_TEST_SOURCE "../../../DataStructure/Disjoint_Sparse_Table.cpp"
#endif
#include SIRKIT_FENWICK_TEST_SOURCE
#include SIRKIT_SEGMENT_TEST_SOURCE
#include SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#include SIRKIT_SPARSE_TEST_SOURCE
#include SIRKIT_DISJOINT_TEST_SOURCE
namespace {
std::string empty_string() { return {}; }
struct Join { std::string operator()(const std::string& a, const std::string& b) const { return a + b; } };
struct First {
    std::string operator()(const std::string& a, const std::string& b) const { return a.empty() ? b : a; }
};
struct Assign { bool active = false; char value = ' '; };
struct Act {
    std::string operator()(const Assign& f, const std::string& value, std::size_t length) const {
        return f.active ? std::string(length, f.value) : value;
    }
};
Assign assign_identity() { return {}; }
struct Compose { Assign operator()(const Assign& newer, const Assign& older) const { return newer.active ? newer : older; } };
template<class Maker, class Verify> void construction(Maker maker, Verify verify) {
    const auto owned = live.load(); const auto start = requests.load();
    std::size_t allocations;
    { auto object = maker(); allocations = requests.load() - start; verify(object); }
    check(live == owned);
    for (std::size_t limit = 0; limit <= allocations; ++limit) {
        budget = static_cast<std::ptrdiff_t>(limit);
        try { auto object = maker(); budget = -1; verify(object); }
        catch (const std::bad_alloc&) { budget = -1; ++failures; }
        check(live == owned);
    }
}
template<class Tree, class Operation, class Before, class After>
void replacement(const Tree& original, Operation operation, Before before, After after) {
    const auto baseline = live.load(); std::size_t allocations;
    { Tree object = original; const auto start = requests.load(); operation(object); allocations = requests.load() - start; after(object); }
    check(live == baseline);
    for (std::size_t limit = 0; limit <= allocations; ++limit) {
        {
            Tree object = original;
            budget = static_cast<std::ptrdiff_t>(limit);
            try { operation(object); budget = -1; after(object); }
            catch (const std::bad_alloc&) { budget = -1; ++failures; before(object); }
        }
        check(live == baseline);
    }
}
struct CombineFailure {};
struct ThrowingIntegerAdd {
    std::size_t* calls;
    std::size_t fail_at;
    std::int64_t operator()(std::int64_t a, std::int64_t b) const {
        if ((*calls)++ == fail_at) throw CombineFailure{};
        return a + b;
    }
};
void integer_operation_failures() {
    for (const auto n : {3, 33, 65, 129}) {
        const std::vector<std::int64_t> input(static_cast<std::size_t>(n), 3);
        const auto owned = live.load();
        std::size_t calls = 0, construction_calls;
        {
            DisjointSparseTable<std::int64_t, ThrowingIntegerAdd> tree(
                input, 0, ThrowingIntegerAdd{&calls, std::numeric_limits<std::size_t>::max()});
            construction_calls = calls;
            check(tree.all_prod() == std::int64_t{n} * 3);
        }
        check(live == owned);
        for (std::size_t fail_at = 0; fail_at < construction_calls; ++fail_at) {
            calls = 0;
            bool threw = false;
            try {
                DisjointSparseTable<std::int64_t, ThrowingIntegerAdd> tree(
                    input, 0, ThrowingIntegerAdd{&calls, fail_at});
            } catch (const CombineFailure&) { threw = true; ++failures; }
            check(threw && calls == fail_at + 1);
            check(live == owned);
        }
    }
}
}
int main() {
    const auto global_before = live.load();
    {
        const std::vector<std::int64_t> numbers(33, 2);
        construction([&] { return fenwick_tree<std::int64_t>(numbers); }, [&](auto& t) { check(t.values() == numbers); });
        construction([&] { return DisjointSparseTable<std::int64_t, std::plus<std::int64_t>>(numbers); },
                     [&](auto& t) { check(t.values() == numbers); check(t.all_prod() == 66); });
        DisjointSparseTable<std::int64_t, std::plus<std::int64_t>> integer_disjoint(numbers);
        const std::vector<std::int64_t> replacement_numbers(65, 3);
        replacement(integer_disjoint, [&](auto& t) { t.assign(replacement_numbers); },
                    [&](auto& t) { check(t.values() == numbers); },
                    [&](auto& t) { check(t.values() == replacement_numbers); check(t.all_prod() == 195); });
        fenwick_tree<std::int64_t> bit(numbers);
        replacement(bit, [](auto& t) { t.reset(67); }, [&](auto& t) { check(t.values() == numbers); }, [](auto& t) { check(t.values() == std::vector<std::int64_t>(67)); });
        std::vector<std::string> words(33, std::string(48, 'x'));
        construction([&] { return segtree<std::string, Join{}, empty_string>(words); }, [&](auto& t) { check(t.values() == words); });
        construction([&] { return SparseTable<std::string, First>(words, ""); }, [&](auto& t) { check(t.values() == words); check(t.all_prod() == words[0]); });
        construction([&] { return DisjointSparseTable<std::string, Join>(words, ""); }, [&](auto& t) { check(t.values() == words); });
        using Lazy = lazy_segtree<std::string, Join{}, empty_string, Assign, Act{}, Compose{}, assign_identity>;
        construction([&] { return Lazy(words); }, [&](auto& t) { check(t.values() == words); });
        segtree<std::string, Join{}, empty_string> segment(words);
        SparseTable<std::string, First> sparse(words, "");
        DisjointSparseTable<std::string, Join> disjoint(words, "");
        Lazy lazy(words);
        std::vector<std::string> replacement_words(65, std::string(65, 'y'));
        auto before = [&](auto& t) { check(t.values() == words); };
        auto after = [&](auto& t) { check(t.values() == replacement_words); };
        replacement(segment, [&](auto& t) { t.assign(replacement_words); }, before, after);
        replacement(sparse, [&](auto& t) { t.assign(replacement_words); }, before, after);
        replacement(disjoint, [&](auto& t) { t.assign(replacement_words); }, before, after);
        replacement(lazy, [&](auto& t) { t.assign(replacement_words); }, before, after);
        construction([&] { return segment; }, before);
        construction([&] { return sparse; }, before);
        construction([&] { return disjoint; }, before);
        construction([&] { return lazy; }, before);
        integer_operation_failures();
    }
    check(live == global_before);
    std::printf("Range allocation checks=%ju failures=%ju live_delta=%zu\n", static_cast<std::uintmax_t>(checks), static_cast<std::uintmax_t>(failures), live.load() - global_before);
}
