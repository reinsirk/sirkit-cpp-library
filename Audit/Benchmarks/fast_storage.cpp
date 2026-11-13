// SPDX-License-Identifier: CC0-1.0
// ベンチ用。STORAGE_LIBRARYとSTORAGE_KINDをビルド時に指定する。
#include <cstdint>
inline std::uint64_t benchmark_treap_seed = 1;
#include STORAGE_LIBRARY
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <time.h>
#include <vector>

using I = std::int64_t;
I range_sum_op(I a, I b) { return a + b; }
I range_sum_identity() { return 0; }
struct Stamp { std::uint64_t wall, cpu; };
Stamp stamp() {
    timespec t{};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
    return {static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count()),
            static_cast<std::uint64_t>(t.tv_sec) * 1000000000u + static_cast<std::uint64_t>(t.tv_nsec)};
}
template<class T> void escape(const T& value) {
    asm volatile("" : : "g"(std::addressof(value)) : "memory");
}
struct Generator {
    std::uint64_t state;
    std::uint64_t next() { state ^= state << 13; state ^= state >> 7; state ^= state << 17; return state; }
};
struct Query { std::size_t left, right, point; I value; unsigned kind; };
struct AddAction {
    I operator()(I value, I sum, std::size_t length) const { return sum + value * static_cast<I>(length); }
};
struct AddCompose { I operator()(I a, I b) const { return a + b; } };

int main(int argc, char** argv) {
    if (argc != 5) return 2;
    const auto n = static_cast<std::size_t>(std::strtoull(argv[1], nullptr, 10));
    const auto q = static_cast<std::size_t>(std::strtoull(argv[2], nullptr, 10));
    Generator random{std::strtoull(argv[3], nullptr, 10)};
    benchmark_treap_seed = random.state;
    const unsigned operation = static_cast<unsigned>(std::strtoul(argv[4], nullptr, 10));
    if (n == 0 || random.state == 0) return 2;
    std::vector<I> values(n);
    for (auto& x : values) x = static_cast<I>(random.next() & 1023);
    std::vector<Query> queries(q);
    for (auto& x : queries) {
        x.left = static_cast<std::size_t>(random.next() % (n + 1));
        x.right = static_cast<std::size_t>(random.next() % (n + 1));
        if (x.left > x.right) std::swap(x.left, x.right);
        x.point = static_cast<std::size_t>(random.next() % n);
        x.value = static_cast<I>(random.next() & 7);
        x.kind = static_cast<unsigned>(random.next() % 4);
    }
    std::uint64_t checksum = 0;

#if STORAGE_KIND == 0
#ifdef SIRKIT_LEGACY_RANGE_API
    using Tree = FenwickTree<I>;
#else
    using Tree = fenwick_tree<I>;
#endif
#elif STORAGE_KIND == 1
#ifdef SIRKIT_LEGACY_RANGE_API
    using Tree = SegmentTree<I, std::plus<I>>;
#else
    using Tree = segtree<I, range_sum_op, range_sum_identity>;
#endif
#elif STORAGE_KIND == 2
#ifdef SIRKIT_LEGACY_RANGE_API
    using Tree = LazySegmentTree<I, I, std::plus<I>, AddAction, AddCompose>;
#else
    using Tree = lazy_segtree<I, range_sum_op, range_sum_identity, I, AddAction{}, AddCompose{}, range_sum_identity>;
#endif
#elif STORAGE_KIND == 3
    using Tree = PersistentSegmentTree<I, std::plus<I>>;
#elif STORAGE_KIND == 4
    using Tree = WaveletMatrix<I>;
#elif STORAGE_KIND == 5
    using Tree = AhoCorasick<6, 'a'>;
#elif STORAGE_KIND == 6
    using Tree = lazy_implicit_treap<RangeAddSum::S, RangeAddSum::op, RangeAddSum::e,
        RangeAddSum::F, RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;
#endif
    std::optional<Tree> storage;
#if STORAGE_KIND == 5
    std::vector<std::string> patterns(n);
    for (auto& p : patterns) {
        p.resize(8 + random.next() % 16);
        for (auto& c : p) c = static_cast<char>('a' + random.next() % 6);
    }
    std::string text(q, 'a');
    for (auto& c : text) c = static_cast<char>('a' + random.next() % 6);
#elif STORAGE_KIND == 6
    std::vector<RangeAddSum::S> sequence;
    sequence.reserve(n);
    for (auto x : values) sequence.push_back({x, 1});
#elif STORAGE_KIND == 3
    std::vector<Tree::Root> roots;
    roots.reserve(q + 1);
#endif
    const auto begin = stamp();
#if STORAGE_KIND == 0 || STORAGE_KIND == 4
    storage.emplace(values);
#elif STORAGE_KIND == 1
#ifdef SIRKIT_LEGACY_RANGE_API
    storage.emplace(values, 0);
#else
    storage.emplace(values);
#endif
#elif STORAGE_KIND == 2
#ifdef SIRKIT_LEGACY_RANGE_API
    storage.emplace(values, 0, 0);
#else
    storage.emplace(values);
#endif
#elif STORAGE_KIND == 3
    storage.emplace(values, std::plus<I>{}, 0);
    roots.push_back(storage->latest_root());
#elif STORAGE_KIND == 5
    storage.emplace();
    for (const auto& p : patterns) storage->add(p);
    storage->build();
#elif STORAGE_KIND == 6
    storage.emplace(sequence);
#endif
    escape(*storage);
    const auto built = stamp();
    auto& tree = *storage;
#if STORAGE_KIND == 5
    for (unsigned repeat = 0; repeat < 12; ++repeat) {
        if (operation == 0) checksum += tree.count_total(text);
        else {
            const auto counts = tree.match_counts(text);
            for (auto count : counts) checksum += count;
        }
        escape(tree);
    }
#else
    for (const auto& x : queries) {
#if STORAGE_KIND == 0
        if (operation == 0 && x.kind == 0) tree.add(x.point, x.value);
        else checksum += static_cast<std::uint64_t>(tree.sum(x.left, x.right));
#elif STORAGE_KIND == 1
        if (operation == 0 && x.kind == 0) tree.set(x.point, x.value);
        else checksum += static_cast<std::uint64_t>(tree.prod(x.left, x.right));
#elif STORAGE_KIND == 2
        if (operation == 0 && x.kind == 0) tree.apply(x.left, x.right, x.value);
        else checksum += static_cast<std::uint64_t>(tree.prod(x.left, x.right));
#elif STORAGE_KIND == 3
        const auto version = x.point % roots.size();
        if (operation == 0 && x.kind == 0) roots.push_back(tree.set(roots[version], x.point, x.value));
        else checksum += static_cast<std::uint64_t>(tree.prod(roots[version], x.left, x.right));
#elif STORAGE_KIND == 4
        if (operation == 0 && x.left != x.right)
            checksum += static_cast<std::uint64_t>(tree.kth_smallest(x.left, x.right, x.point % (x.right - x.left)));
        else checksum += tree.count_less(x.left, x.right, static_cast<I>(x.point & 1023));
#elif STORAGE_KIND == 6
        if (operation == 0 && x.kind == 0) tree.apply(x.left, x.right, x.value);
        else if (operation == 0 && x.kind == 1) tree.reverse(x.left, x.right);
        else checksum += static_cast<std::uint64_t>(tree.prod(x.left, x.right).sum);
#endif
    }
#endif
    escape(tree);
    const auto operated = stamp();
    storage.reset();
    const auto destroyed = stamp();
    std::printf("{\"checksum\":%ju,\"build_cpu_ns\":%ju,\"operation_cpu_ns\":%ju,\"destroy_cpu_ns\":%ju,"
                "\"build_wall_ns\":%ju,\"operation_wall_ns\":%ju,\"destroy_wall_ns\":%ju}\n",
        static_cast<std::uintmax_t>(checksum), static_cast<std::uintmax_t>(built.cpu-begin.cpu),
        static_cast<std::uintmax_t>(operated.cpu-built.cpu), static_cast<std::uintmax_t>(destroyed.cpu-operated.cpu),
        static_cast<std::uintmax_t>(built.wall-begin.wall), static_cast<std::uintmax_t>(operated.wall-built.wall),
        static_cast<std::uintmax_t>(destroyed.wall-operated.wall));
}
