// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HLD_SOURCE
#define HLD_SOURCE "../../../Graph/Heavy_Light_Decomposition.cpp"
#endif
#ifdef SIRKIT_HLD_HELPERS_RANGES_FIRST
#include "../../../DataStructure/Fenwick_Tree.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include "../../../DataStructure/Lazy_Segment_Tree.cpp"
#include "../vendor/atcoder/fenwicktree.hpp"
#include "../vendor/atcoder/segtree.hpp"
#include "../vendor/atcoder/lazysegtree.hpp"
#include HLD_SOURCE
#else
#include HLD_SOURCE
#include "../../../DataStructure/Fenwick_Tree.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include "../../../DataStructure/Lazy_Segment_Tree.cpp"
#include "../vendor/atcoder/fenwicktree.hpp"
#include "../vendor/atcoder/segtree.hpp"
#include "../vendor/atcoder/lazysegtree.hpp"
#endif
#include HLD_SOURCE
#include <algorithm>
#include <concepts>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <source_location>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using I = std::int64_t;
std::uint64_t checks = 0, forests = 0;
void check(bool condition, const std::source_location where = std::source_location::current()) {
    ++checks;
    if (!condition) {
        std::fprintf(stderr, "HLD helper check failed at %s:%u\n", where.file_name(), where.line());
        std::abort();
    }
}
template<class Exception, class Fn> void expect(Fn&& fn) {
    bool caught = false;
    try { fn(); } catch (const Exception&) { caught = true; }
    check(caught);
}
I sum_op(I a, I b) { return a + b; }
I sum_e() { return 0; }
I min_op(I a, I b) { return std::min(a, b); }
I min_e() { return I{1} << 60; }
constexpr auto max_op = [](I a, I b) { return std::max(a, b); };
constexpr auto max_e = [] { return -(I{1} << 60); };

struct Total {
    I sum;
    int length;
    bool operator==(const Total&) const = default;
};
Total total_op(Total a, Total b) { return {a.sum + b.sum, a.length + b.length}; }
Total total_e() { return {0, 0}; }
struct Action { bool assign; I value; }; // equalityを要求しない作用。
Total act(Action f, Total x) {
    return {f.assign ? f.value * x.length : x.sum + f.value * x.length, x.length};
}
Action compose(Action newer, Action older) {
    if (newer.assign) return newer;
    return {older.assign, older.value + newer.value};
}
Action action_e() { return {false, 0}; }
using OwnLazy = lazy_segtree<Total, total_op, total_e, Action, act, compose, action_e>;
using AclLazy = atcoder::lazy_segtree<Total, total_op, total_e, Action, act, compose, action_e>;

template<class H, class R> concept InferredProduct = requires(const H& h, R& range) {
    h.path_prod(0, 0, range);
};
using OwnSum = segtree<I, sum_op, sum_e>;
using AclSum = atcoder::segtree<I, sum_op, sum_e>;
static_assert(InferredProduct<HeavyLightDecomposition, const OwnSum>);
static_assert(InferredProduct<HeavyLightDecomposition, const AclSum>);
static_assert(!InferredProduct<HeavyLightDecomposition, const OwnLazy>);
static_assert(!InferredProduct<HeavyLightDecomposition, const AclLazy>);
static_assert(!InferredProduct<HeavyLightDecomposition, const fenwick_tree<I>>);
static_assert(!InferredProduct<HeavyLightDecomposition, const atcoder::fenwick_tree<I>>);
static_assert(!InferredProduct<HeavyLightDecomposition, std::vector<I>>);

// BFSで根・親・経路を独立に求める。頂点番号やHLDのparentには依存しない。
struct Reference {
    const Graph& graph;
    std::vector<int> parent, depth;
    Reference(const Graph& g, int root) : graph(g), parent(g.size(), -2), depth(g.size()) {
        const auto discover = [&](int start) {
            std::queue<int> pending; pending.push(start); parent[start] = -1;
            while (!pending.empty()) {
                const int v = pending.front(); pending.pop();
                for (int u : graph[v]) if (parent[u] == -2) {
                    parent[u] = v; depth[u] = depth[v] + 1; pending.push(u);
                }
            }
        };
        if (!g.empty()) discover(root);
        for (int v = 0; v < int(g.size()); ++v) if (parent[v] == -2) discover(v);
    }
    std::vector<int> path(int u, int v, bool edges) const {
        std::vector<int> from(graph.size(), -1); std::queue<int> pending;
        from[u] = u; pending.push(u);
        while (!pending.empty()) {
            const int x = pending.front(); pending.pop();
            for (int y : graph[x]) if (from[y] == -1) { from[y] = x; pending.push(y); }
        }
        if (from[v] == -1) return {};
        std::vector<int> result;
        for (int x = v;; x = from[x]) { result.push_back(x); if (x == u) break; }
        std::reverse(result.begin(), result.end());
        if (edges) {
            const auto ancestor = std::min_element(result.begin(), result.end(),
                [&](int a, int b) { return depth[a] < depth[b]; });
            result.erase(ancestor);
        }
        return result;
    }
    bool connected(int u, int v) const { return !path(u, v, false).empty(); }
    std::vector<int> subtree(int v, bool edges) const {
        std::vector<int> result;
        for (int u = 0; u < int(graph.size()); ++u) {
            if (edges && u == v) continue;
            for (int x = u; x != -1; x = parent[x])
                if (x == v) { result.push_back(u); break; }
        }
        return result;
    }
};
I sum_at(const std::vector<I>& values, const std::vector<int>& vertices) {
    I result = 0; for (int v : vertices) result += values[v]; return result;
}
struct NonDefault {
    int value;
    NonDefault() = delete;
    explicit NonDefault(int x) : value(x) {}
    NonDefault(const NonDefault&) = default;
    NonDefault& operator=(const NonDefault&) = default;
    NonDefault(NonDefault&&) = delete;
    NonDefault& operator=(NonDefault&&) = delete;
    bool operator==(const NonDefault&) const = default;
};
void permutation_checks(const HeavyLightDecomposition& hld, std::mt19937_64& random) {
    const int n = hld.size();
    std::vector<I> values(n); std::vector<bool> bits(n); std::vector<NonDefault> objects;
    objects.reserve(n);
    for (int v = 0; v < n; ++v) {
        values[v] = I(random() % 10000); bits[v] = (random() & 1) != 0; objects.emplace_back(v + 100);
    }
    auto ordered = hld.reorder(values);
    for (int v = 0; v < n; ++v) check(ordered[hld.index(v)] == values[v]);
    check(hld.restore(ordered) == values);
    check(hld.restore(hld.reorder(bits)) == bits);
    check(hld.restore(hld.reorder(objects)) == objects);
    if (n) {
        const I before = values[hld.vertex_at(0)]; ordered[0] += 77;
        check(values[hld.vertex_at(0)] == before);
    }
}
void commutative_checks(const Graph& graph, int root, std::mt19937_64& random) {
    HeavyLightDecomposition hld(graph, root); Reference reference(graph, root); ++forests;
    permutation_checks(hld, random);
    const int n = int(graph.size()); if (!n) return;
    std::vector<I> values(n);
    for (auto& value : values) value = I(random() % 1000) - 500;
    const auto ordered = hld.reorder(values);
    fenwick_tree<I> own_bit(ordered); atcoder::fenwick_tree<I> acl_bit(n);
    for (int v = 0; v < n; ++v) acl_bit.add(hld.index(v), values[v]);
    const segtree<I, sum_op, sum_e> own_sum(ordered);
    const atcoder::segtree<I, sum_op, sum_e> acl_sum(ordered);
    const segtree<I, min_op, min_e> own_min(ordered);
    const atcoder::segtree<I, min_op, min_e> acl_min(ordered);
    const segtree<I, max_op, max_e> own_max(ordered);
    const atcoder::segtree<I, max_op, max_e> acl_max(ordered);
    const auto query = [&](int l, int r) -> I { return own_bit.sum(l, r); };
    static_assert(std::is_same_v<decltype(hld.path_prod(0, 0, 0, query, sum_op)), I>);
    const int rounds = n <= 7 ? n * n : 100;
    for (int q = 0; q < rounds; ++q) {
        const int u = n <= 7 ? q / n : int(random() % n);
        const int v = n <= 7 ? q % n : int(random() % n);
        if (!reference.connected(u, v)) continue;
        for (bool edges : {false, true}) {
            const auto vertices = reference.path(u, v, edges);
            const I want = sum_at(values, vertices);
            check(hld.path_prod(u, v, own_bit, edges) == want);
            check(hld.path_prod(u, v, acl_bit, edges) == want);
            check(hld.path_prod(u, v, own_sum, edges) == want);
            check(hld.path_prod(u, v, acl_sum, edges) == want);
            check(hld.path_prod(u, v, 0, query, sum_op, edges) == want);
            check(hld.path_prod(u, v, 0, own_bit, sum_op, edges) == want);
            check(hld.path_prod(u, v, I{0}, own_sum, sum_op, edges) == want);
            I minimum = min_e(), maximum = max_e();
            for (int x : vertices) { minimum = std::min(minimum, values[x]); maximum = std::max(maximum, values[x]); }
            check(hld.path_prod(u, v, own_min, edges) == minimum);
            check(hld.path_prod(u, v, acl_min, edges) == minimum);
            check(hld.path_prod(u, v, own_max, edges) == maximum);
            check(hld.path_prod(u, v, acl_max, edges) == maximum);
        }
    }
    for (int v = 0; v < n; ++v) for (bool edges : {false, true}) {
        const I want = sum_at(values, reference.subtree(v, edges));
        check(hld.subtree_prod(v, own_bit, edges) == want);
        check(hld.subtree_prod(v, acl_bit, edges) == want);
        check(hld.subtree_prod(v, own_sum, edges) == want);
        check(hld.subtree_prod(v, query, edges) == want);
    }
    own_bit.add(hld.index(root), 9); acl_bit.add(hld.index(root), 9); values[root] += 9;
    check(hld.path_prod(root, root, own_bit) == values[root]);
    check(hld.path_prod(root, root, acl_bit) == values[root]);
}
void lazy_checks(const Graph& graph, int root, std::mt19937_64& random) {
    const int n = int(graph.size()); if (!n) return;
    HeavyLightDecomposition hld(graph, root); Reference reference(graph, root);
    std::vector<I> values(n);
    for (auto& x : values) x = I(random() % 100) - 50;
    std::vector<Total> atoms; atoms.reserve(n);
    for (I x : values) atoms.push_back({x, 1});
    OwnLazy own(hld.reorder(atoms)); AclLazy acl(hld.reorder(atoms));
    for (int q = 0; q < 100; ++q) {
        const int u = int(random() % n), v = int(random() % n);
        const bool edges = (random() & 1) != 0;
        Action f{(random() & 1) != 0, I(random() % 21) - 10};
        if (q % 3 == 0) {
            hld.subtree_apply(v, own, f, edges); hld.subtree_apply(v, acl, f, edges);
            for (int x : reference.subtree(v, edges)) values[x] = f.assign ? f.value : values[x] + f.value;
        } else if (reference.connected(u, v)) {
            if (q % 3 == 1) {
                hld.path_apply(u, v, own, f, edges); hld.path_apply(u, v, acl, f, edges);
                for (int x : reference.path(u, v, edges)) values[x] = f.assign ? f.value : values[x] + f.value;
            } else {
                const auto path = reference.path(u, v, edges);
                const Total want{sum_at(values, path), int(path.size())};
                check(hld.path_prod(u, v, own, edges) == want);
                check(hld.path_prod(u, v, acl, edges) == want);
                const auto reverse = [](Total x) { return x; };
                check(hld.path_prod(u, v, own, reverse, edges) == want);
                check(hld.path_prod(u, v, acl, reverse, edges) == want);
            }
        }
        const auto sub = reference.subtree(v, edges);
        const Total want{sum_at(values, sub), int(sub.size())};
        check(hld.subtree_prod(v, own, edges) == want);
        check(hld.subtree_prod(v, acl, edges) == want);
    }
    for (int v = 0; v < n; ++v) {
        check(own.get(hld.index(v)) == Total{values[v], 1});
        check(acl.get(hld.index(v)) == Total{values[v], 1});
    }
}
std::string concat(std::string a, std::string b) { return a + b; }
std::string concat_reverse(std::string a, std::string b) { return b + a; }
std::string string_e() { return {}; }
struct BothArityQuery {
    std::string operator()(int, int, bool = false) const { return {}; }
};
struct ReverseString {
    std::string operator()(std::string x) const { std::reverse(x.begin(), x.end()); return x; }
};
template<class H, class Q, class Reverse> concept ExplicitReverseProduct = requires(const H& h, Q& query, Reverse& reverse) {
    h.path_prod(0, 0, std::string{}, query, concat, reverse);
};
static_assert(!ExplicitReverseProduct<HeavyLightDecomposition, BothArityQuery, ReverseString>);
static_assert(ExplicitReverseProduct<HeavyLightDecomposition, BothArityQuery, std::identity>);

struct Directions {
    std::string forward, backward;
    bool operator==(const Directions&) const = default;
};
Directions directions_op(Directions a, Directions b) { return {a.forward + b.forward, b.backward + a.backward}; }
Directions directions_e() { return {}; }
struct ReverseDirections {
    Directions operator()(Directions x) const { std::swap(x.forward, x.backward); return x; }
};
void noncommutative_checks(const Graph& graph, int root, std::mt19937_64& random) {
    const int n = int(graph.size()); if (!n) return;
    HeavyLightDecomposition hld(graph, root); Reference reference(graph, root);
    std::vector<std::string> values(n); std::vector<Directions> atoms;
    for (int v = 0; v < n; ++v) { values[v] = "(" + std::to_string(v) + ")"; atoms.push_back({values[v], values[v]}); }
    const segtree<Directions, directions_op, directions_e> own(hld.reorder(atoms));
    const atcoder::segtree<Directions, directions_op, directions_e> acl(hld.reorder(atoms));
    const segtree<std::string, concat, string_e> forward(hld.reorder(values));
    const segtree<std::string, concat_reverse, string_e> backward(hld.reorder(values));
    // 両方のarityで呼べるが、3引数が選ばれなければ逆向きの答えが壊れる。
    const auto directional = [&](int l, int r, bool reverse = false) {
        return reverse ? backward.prod(l, r) : forward.prod(l, r);
    };
    const auto paired = [&](int l, int r) { return own.prod(l, r); };
    for (int q = 0; q < 90; ++q) {
        const int u = int(random() % n), v = int(random() % n);
        if (!reference.connected(u, v)) continue;
        for (bool edges : {false, true}) {
            auto path = reference.path(u, v, edges); Directions want;
            for (int x : path) want.forward += values[x];
            for (auto it = path.rbegin(); it != path.rend(); ++it) want.backward += values[*it];
            check(hld.path_prod(u, v, own, ReverseDirections{}, edges) == want);
            check(hld.path_prod(u, v, acl, ReverseDirections{}, edges) == want);
            check(hld.path_prod(u, v, directions_e(), own, directions_op, ReverseDirections{}, edges) == want);
            check(hld.path_prod(u, v, directions_e(), paired, directions_op, ReverseDirections{}, edges) == want);
            check(hld.path_prod(u, v, std::string{}, directional, concat, edges) == want.forward);
        }
        auto nodes = reference.subtree(v, false);
        std::sort(nodes.begin(), nodes.end(), [&](int a, int b) { return hld.index(a) < hld.index(b); });
        std::string expected; for (int x : nodes) expected += values[x];
        check(hld.subtree_prod(v, own).forward == expected);
        check(hld.subtree_prod(v, acl).forward == expected);
        check(hld.subtree_prod(v, directional) == expected);
    }
}
struct Affine {
    std::uint64_t a, b;
    bool operator==(const Affine&) const = default;
};
Affine append_affine(Affine f, Affine g) { return {f.a * g.a, f.b * g.a + g.b}; }
struct AffineDirections { Affine forward, backward; bool operator==(const AffineDirections&) const = default; };
AffineDirections affine_op(AffineDirections x, AffineDirections y) {
    return {append_affine(x.forward, y.forward), append_affine(y.backward, x.backward)};
}
AffineDirections affine_e() { return {{1, 0}, {1, 0}}; }
void affine_checks() {
    Graph graph{{1, 2}, {0, 3, 4}, {0}, {1}, {1}};
    HeavyLightDecomposition hld(graph, 4); Reference reference(graph, 4);
    std::vector<AffineDirections> values;
    for (std::uint64_t i = 0; i < graph.size(); ++i) values.push_back({{i + 2, i + 7}, {i + 2, i + 7}});
    const segtree<AffineDirections, affine_op, affine_e> seg(hld.reorder(values));
    const auto reverse = [](AffineDirections x) { std::swap(x.forward, x.backward); return x; };
    for (int u = 0; u < hld.size(); ++u) for (int v = 0; v < hld.size(); ++v) for (bool edges : {false, true}) {
        const auto path = reference.path(u, v, edges); Affine want{1, 0};
        for (int x : path) want = append_affine(want, values[x].forward);
        check(hld.path_prod(u, v, seg, reverse, edges).forward == want);
    }
}
template<class T, class U> void wrap_checks() {
    Graph graph{{1, 2, 3}, {0}, {0}, {0}}; HeavyLightDecomposition hld(graph);
    for (T value : {std::numeric_limits<T>::max(), std::numeric_limits<T>::lowest()}) {
        std::vector<T> values(4); values[1] = value; values[2] = value;
        fenwick_tree<T> own(hld.reorder(values)); atcoder::fenwick_tree<T> acl(4);
        for (int v = 0; v < 4; ++v) acl.add(hld.index(v), values[v]);
        const T want = static_cast<T>(static_cast<U>(static_cast<U>(value) + static_cast<U>(value)));
        check(hld.path_prod(1, 2, own) == want);
        check(hld.path_prod(1, 2, acl) == want);
        check(hld.path_prod(2, 1, own) == want);
        check(hld.path_prod(2, 1, acl) == want);
    }
}
// actionがbackend自身の要素でも、一回の操作では最初の値を全対象へ適用する。
template<bool ReferenceAction> struct AliasingRange {
    std::vector<I> data;
    void apply(int l, int r, std::conditional_t<ReferenceAction, const I&, I> action) {
        for (int i = l; i < r; ++i) data[i] += action;
    }
};
void aliased_action_checks() {
    Graph graph{{1, 2}, {0}, {0}};
    HeavyLightDecomposition hld(graph); Reference reference(graph, 0);
    const std::vector<I> initial{1, 2, 3};
    const auto paths = [&]<bool ReferenceAction>() {
        for (auto [u, v] : {std::pair{1, 2}, std::pair{2, 1}})
            for (bool edges : {false, true}) {
                AliasingRange<ReferenceAction> range{hld.reorder(initial)};
                const I original_action = range.data[hld.index(u)];
                auto expected = initial;
                for (int x : reference.path(u, v, edges)) expected[x] += original_action;
                hld.path_apply(u, v, range, range.data[hld.index(u)], edges);
                check(hld.restore(range.data) == expected);
            }
        for (bool edges : {false, true}) {
            AliasingRange<ReferenceAction> range{hld.reorder(initial)};
            const int source = edges ? hld.vertex_at(hld.index(0) + 1) : 0;
            const I original_action = range.data[hld.index(source)];
            auto expected = initial;
            for (int x : reference.subtree(0, edges)) expected[x] += original_action;
            hld.subtree_apply(0, range, range.data[hld.index(source)], edges);
            check(hld.restore(range.data) == expected);
        }
    };
    paths.template operator()<false>();
    paths.template operator()<true>();
}
void large_result_type_checks() {
    Graph graph{{1, 2}, {0}, {0}}; HeavyLightDecomposition hld(graph);
    const std::vector<I> values{0, 3000000000, 4000000000}; fenwick_tree<I> bit(hld.reorder(values));
    const auto query = [&](int l, int r) -> I { return bit.sum(l, r); };
    auto value = hld.path_prod(1, 2, 0, query, sum_op);
    static_assert(std::same_as<decltype(value), I>); check(value == 7000000000);
}
#ifndef SKIP_RUNTIME_CHECKS
void invalid_checks() {
    Graph graph{{1}, {0}, {3}, {2}, {}}; HeavyLightDecomposition hld(graph, 3);
    std::vector<I> values(5, 1); fenwick_tree<I> bit(hld.reorder(values));
    OwnLazy lazy(std::vector<Total>(5, Total{1, 1})); const auto before = lazy.values();
    expect<std::invalid_argument>([&] { (void)hld.reorder(std::vector<I>(4)); });
    expect<std::invalid_argument>([&] { (void)hld.reorder(std::vector<I>(6)); });
    expect<std::invalid_argument>([&] { (void)hld.restore(std::vector<I>(4)); });
    expect<std::invalid_argument>([&] { (void)hld.restore(std::vector<I>(6)); });
    int calls = 0;
    const auto query = [&](int l, int r) -> I { ++calls; return bit.sum(l, r); };
    expect<std::invalid_argument>([&] { (void)hld.path_prod(0, 2, bit); });
    expect<std::invalid_argument>([&] { (void)hld.path_prod(0, 2, I{0}, query, sum_op); });
    check(calls == 0);
    expect<std::invalid_argument>([&] { hld.path_apply(0, 2, lazy, Action{true, 100}); });
    check(lazy.values() == before);
    for (int bad : {-1, 5}) {
        expect<std::out_of_range>([&] { (void)hld.path_prod(bad, 0, bit); });
        expect<std::out_of_range>([&] { (void)hld.path_prod(0, bad, bit, true); });
        expect<std::out_of_range>([&] { (void)hld.subtree_prod(bad, query); });
        expect<std::out_of_range>([&] { hld.path_apply(bad, 0, lazy, Action{false, 3}); });
        expect<std::out_of_range>([&] { hld.subtree_apply(bad, lazy, Action{false, 3}); });
    }
    check(calls == 0); check(lazy.values() == before);
}
#endif
}
int main() {
    std::mt19937_64 random(0x484c445f48454c50ULL);
    for (int n : {0, 1, 2, 7, 19, 65, 257}) for (int trial = 0; trial < 3; ++trial) {
        std::vector<int> labels(n); std::iota(labels.begin(), labels.end(), 0); std::shuffle(labels.begin(), labels.end(), random);
        Graph graph(n);
        for (int v = 1; v < n; ++v) if (trial == 0 || random() % 4) {
            const int p = int(random() % v); graph[labels[v]].push_back(labels[p]); graph[labels[p]].push_back(labels[v]);
        }
        for (auto& row : graph) std::shuffle(row.begin(), row.end(), random);
        const int root = n ? int(random() % n) : 0;
        commutative_checks(graph, root, random); lazy_checks(graph, root, random);
        if (n <= 19) noncommutative_checks(graph, root, random);
    }
    affine_checks(); large_result_type_checks(); aliased_action_checks();
    wrap_checks<std::int8_t, std::uint8_t>(); wrap_checks<std::int16_t, std::uint16_t>();
    wrap_checks<std::int32_t, std::uint32_t>(); wrap_checks<std::int64_t, std::uint64_t>();
    wrap_checks<std::uint32_t, std::uint32_t>(); wrap_checks<std::uint64_t, std::uint64_t>();
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
    wrap_checks<__int128, unsigned __int128>(); wrap_checks<unsigned __int128, unsigned __int128>();
#endif
#ifndef SKIP_RUNTIME_CHECKS
    invalid_checks();
#endif
    std::printf("HLD helpers forests=%" PRIu64 " checks=%" PRIu64 "\n", forests, checks);
}
