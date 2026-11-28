// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Subtree_Edge_Add_Distances.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subtree_edge_add_distances.md
//
// API一覧
// SubtreeEdgeAddDistances<T=int64_t>(n,edges,root=0): 固定の無向森。Edge{from,to,weight} を各辺1回渡す。
//   指定根のない成分は最小頂点を根とする。構築・領域 O(n+B log(B+1))、B=ceil(n/32)。
// size(): 頂点数。O(1)。
// add_subtree(v,delta): vの部分木内の全辺に加算。親からvへの辺は対象外。O(log(n+1))。
// root_distance(v): 成分の根からvまでの辺重みの総和。O(log(n+1))。
// distance(u,v): パス上の辺重みの総和。異なる成分なら nullopt。O(log(n+1))。
//   頂点は0始まり。T は bool を除く64ビット以下の整数型。加減算は型幅で周回する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LCA_SIZE_HELPERS_DEFINED
#define SIRKIT_LCA_SIZE_HELPERS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline constexpr auto kLcaMaximumVertices = std::min<std::size_t>(
    std::numeric_limits<int>::max(), std::numeric_limits<std::uint32_t>::max() / 2);

inline void CheckLcaVertexCount(std::size_t count) {
    if (count > kLcaMaximumVertices) throw std::length_error("LCA: too many vertices");
}

inline void CheckLcaCsrCapacity(std::size_t vertices, std::size_t arcs,
                               std::size_t row_capacity) {
    if (row_capacity < 2 || vertices > row_capacity - 2 ||
        arcs > static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / (2 * sizeof(int)))
        throw std::length_error("LCA: CSR capacity exceeded");
}

inline std::size_t LcaBlockTableSize(std::size_t vertices, std::size_t blocks, unsigned int levels) {
    constexpr std::size_t maximum_count =
        static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(std::uint32_t);
    if (vertices > maximum_count / 3 || blocks > (maximum_count - 3 * vertices) / levels)
        throw std::length_error("LCA: block table too large");
    return 3 * vertices + blocks * levels;
}

inline std::size_t LcaAncestorTableSize(std::size_t vertices, unsigned int levels) {
    constexpr std::size_t maximum_count =
        static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(int);
    if (vertices > maximum_count / levels) throw std::length_error("LCA: ancestor table too large");
    return vertices * levels;
}
}
#endif
#ifndef SIRKIT_LCA_CSR_GRAPH_DEFINED
#define SIRKIT_LCA_CSR_GRAPH_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
class LcaCsrGraph {
    std::unique_ptr<int[]> arcs_;
    std::vector<std::size_t> begin_;
public:
    LcaCsrGraph(int n, std::span<const std::pair<int,int>> edges) {
        if (n < 0) throw std::length_error("LCA: negative vertex count");
        const auto count = static_cast<std::size_t>(n);
        CheckLcaCsrCapacity(count, edges.size(), begin_.max_size());
        // 散布cursorを1要素ずらし、散布後の値をそのまま行のoffsetへ使う。
        begin_.assign(count + 2, 0);
        for (const auto& [u,v] : edges) {
            ++begin_[static_cast<std::size_t>(u) + 2];
            ++begin_[static_cast<std::size_t>(v) + 2];
        }
        for (std::size_t i = 2; i <= count + 1; ++i) begin_[i] += begin_[i-1];
        arcs_ = std::make_unique_for_overwrite<int[]>(std::max(std::size_t{1}, 2 * edges.size()));
        for (const auto& [u,v] : edges) {
            arcs_[begin_[static_cast<std::size_t>(u) + 1]++] = v;
            arcs_[begin_[static_cast<std::size_t>(v) + 1]++] = u;
        }
    }
    LcaCsrGraph(const LcaCsrGraph&) = delete;
    LcaCsrGraph& operator=(const LcaCsrGraph&) = delete;
    std::size_t size() const noexcept { return begin_.size() - 2; }
    std::span<const int> operator[](int v) const noexcept {
        const auto index = static_cast<std::size_t>(v);
        return {arcs_.get() + begin_[index], begin_[index+1] - begin_[index]};
    }
};
}
#endif

#ifndef SIRKIT_LCA_LADDER_DEFINED
#define SIRKIT_LCA_LADDER_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
class AncestorLadder {
    struct Position { std::uint32_t begin; std::uint32_t index; };
    std::vector<Position> positions_;
    std::vector<int> entries_;
public:
    // parentとDFS順、深さから構築。頂点数はintの範囲、根の親は-1。O(N)。
    template<class DepthAt>
    void build(std::size_t n, const int* parent, const int* order, DepthAt depth_at,
               int* height, int* heavy) {
        if (n > kLcaMaximumVertices ||
            n > static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / (2 * sizeof(int))) {
            throw std::length_error("LCA: ancestor ladders too large");
        }
        AncestorLadder replacement;
        replacement.positions_.resize(n);
        replacement.entries_.reserve(2 * n);
        std::fill_n(height, n, 1);
        std::fill_n(heavy, n, -1);
        for (std::size_t i = n; i != 0;) {
            const int v = order[--i];
            const int p = parent[v];
            if (p != -1 && height[p] < height[v] + 1) {
                height[p] = height[v] + 1;
                heavy[p] = v;
            }
        }
        for (std::size_t i = 0; i < n; ++i) {
            const int head = static_cast<int>(i);
            if (parent[head] != -1 && heavy[parent[head]] == head) continue;
            const auto begin = static_cast<std::uint32_t>(replacement.entries_.size());
            const int extension = std::min(height[head], depth_at(head));
            int ancestor = head;
            for (int j = 0; j < extension; ++j) {
                ancestor = parent[ancestor];
                replacement.entries_.push_back(ancestor);
            }
            std::reverse(replacement.entries_.begin() + begin, replacement.entries_.end());
            for (int v = head; v != -1; v = heavy[v]) {
                replacement.positions_[v] = {begin, static_cast<std::uint32_t>(replacement.entries_.size())};
                replacement.entries_.push_back(v);
            }
        }
        swap(replacement);
    }
    // kはvの深さ以下。祖先を返す。O(log(H+2))。
    int climb(int v, std::uint64_t k, const int* parent) const noexcept {
        for (;;) {
            const std::uint32_t begin = positions_[v].begin;
            const std::uint32_t position = positions_[v].index;
            const std::uint32_t available = position - begin;
            if (k <= available) return entries_[position - static_cast<std::uint32_t>(k)];
            k -= static_cast<std::uint64_t>(available) + 1;
            v = parent[entries_[begin]];
        }
    }
    void swap(AncestorLadder& other) noexcept {
        positions_.swap(other.positions_);
        entries_.swap(other.entries_);
    }
    friend void swap(AncestorLadder& a, AncestorLadder& b) noexcept { a.swap(b); }
};
}
#endif

#ifndef SIRKIT_LCA_DEFINED
#define SIRKIT_LCA_DEFINED

class LCA {
    int size_ = 0;
    int root_ = -1;
    unsigned int levels_ = 0;
    bool shallow_ = true;
#if defined(__GNUC__) && !defined(__clang__)
    int root_cut_ = 0;
    struct NestedCut { int boundary = 0; int ancestor = 0; };
    NestedCut nested_cuts_[2]{};
    bool nested_enabled_ = false;
#endif
    struct Vertex {
        int depth;
        int component;
        int entry;
        int end;
        Vertex() noexcept : depth(-1) {}
    };
    std::vector<Vertex> vertices_;
    std::vector<int> roots_;
    std::unique_ptr<int[]> up_;
    struct DeepQueryData {
        std::vector<std::uint32_t> rmq_storage;
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AncestorLadder ladder;
    };
    std::shared_ptr<const DeepQueryData> deep_;
    std::size_t block_count_ = 0;

    void check_vertex([[maybe_unused]] int v) const {
    }

    std::size_t ancestor_index(unsigned int bit, int v) const noexcept {
#if defined(__clang__)
        return static_cast<std::size_t>(v) * static_cast<std::size_t>(levels_) + bit;
#else
        return static_cast<std::size_t>(bit) * static_cast<std::size_t>(size_) + static_cast<std::size_t>(v);
#endif
    }

    int climb(int v, std::uint64_t k) const noexcept {
        if (k == 0) return v;
        if (block_count_ != 0) return deep_->ladder.climb(v, k, up_.get());
        while (k != 0) {
#if defined(__GNUC__) && !defined(__clang__)
            const unsigned int bit = static_cast<unsigned int>(std::bit_width(k) - 1);
            v = up_[ancestor_index(bit, v)];
            k ^= std::uint64_t{1} << bit;
#else
            const unsigned int bit = static_cast<unsigned int>(std::countr_zero(k));
            v = up_[ancestor_index(bit, v)];
            k &= k - 1;
#endif
        }
        return v;
    }

#if defined(__GNUC__) && !defined(__clang__)
    void choose_nested_cuts(const int* order) noexcept {
        for (unsigned int side = 0; side < 2; ++side) {
            const int child = order[side == 0 ? 1 : root_cut_];
            const int begin = vertices_[child].entry;
            const int end = vertices_[child].end;
            const int subtree_size = end - begin;
            if (subtree_size < 128) continue;
            const int middle = order[static_cast<std::size_t>(begin) + static_cast<unsigned int>(subtree_size) / 2];
            const int grandchild = climb(middle, static_cast<unsigned int>(vertices_[middle].depth - vertices_[child].depth - 1));
            const int candidates[2] = {vertices_[grandchild].entry, vertices_[grandchild].end};
            std::int64_t best_distance = std::numeric_limits<std::int64_t>::max();
            int best = 0;
            for (const int cut : candidates) {
                if (cut <= begin + 1 || cut >= end) continue;
                std::int64_t distance = std::int64_t{2} * cut - begin - end - 1;
                if (distance < 0) distance = -distance;
                if (distance < best_distance) { best_distance = distance; best = cut; }
            }
            if (best != 0 && std::min(best - begin - 1, end - best) >= subtree_size / 8) {
                nested_cuts_[side] = {best, child};
                nested_enabled_ = true;
            }
        }
    }

    void choose_root_cut(const int* order, std::size_t root_degree) noexcept {
        if (size_ < 128 || roots_.size() != 1 || root_degree <= 1) return;
        const int middle = order[static_cast<std::size_t>(size_) / 2];
        const int child = climb(middle, static_cast<unsigned int>(vertices_[middle].depth - 1));
        const int candidates[2] = {vertices_[child].entry, vertices_[child].end};
        std::int64_t best_distance = std::numeric_limits<std::int64_t>::max();
        int best = 0;
        for (const int cut : candidates) {
            if (cut <= 1 || cut >= size_) continue;
            std::int64_t distance = std::int64_t{2} * cut - size_ - 1;
            if (distance < 0) distance = -distance;
            if (distance < best_distance) { best_distance = distance; best = cut; }
        }
        if (best != 0 && std::min(best - 1, size_ - best) >= size_ / 8) root_cut_ = best;
        if (root_cut_ != 0 && root_degree == 2) choose_nested_cuts(order);
    }
#endif

    std::uint32_t block_minimum(std::size_t left, std::size_t right) const noexcept {
        const auto* keys = deep_->rmq_storage.data() + size_;
        const auto* masks = keys + size_;
        const std::uint32_t candidates = masks[right] & (~std::uint32_t{0} << (left & 31U));
        return keys[(left & ~std::size_t{31}) + std::countr_zero(candidates)];
    }

#if defined(__GNUC__) || defined(__clang__)
    [[gnu::noinline]]
#endif
    std::uint32_t range_minimum(std::size_t left, std::size_t right,
                                std::uint32_t root_entry) const noexcept {
        const std::size_t lb = left / 32;
        const std::size_t rb = right / 32;
        if (lb == rb) return block_minimum(left, right);
        std::uint32_t result = std::numeric_limits<std::uint32_t>::max();
        const std::size_t middle_count = rb - lb - 1;
        if (middle_count != 0) {
            const unsigned int level = static_cast<unsigned int>(std::bit_width(middle_count) - 1);
            const auto* table = deep_->rmq_storage.data() + 3 * static_cast<std::size_t>(size_);
            const std::size_t offset = static_cast<std::size_t>(level) * block_count_;
            result = std::min(table[offset + lb + 1],
                              table[offset + rb - (std::size_t{1} << level)]);
            // 同成分の親のDFS添字は成分根より小さくならない。
            if (result == root_entry) return result;
        }
        return std::min(result, std::min(block_minimum(left, (lb + 1) * 32 - 1),
                                         block_minimum(rb * 32, right)));
    }

#if defined(__GNUC__) || defined(__clang__)
    [[gnu::always_inline]]
#endif
    int lca(int u, int v) const noexcept {
        if (u == v) return u;
        if (vertices_[u].component != vertices_[v].component) return -1;
        if (vertices_[u].entry > vertices_[v].entry) std::swap(u, v);
        const int target = vertices_[v].entry;
        if (target < vertices_[u].end) return u;
#if defined(__GNUC__) && !defined(__clang__)
        // 根の子部分木を分けるDFS境界をまたぐとき、LCAは根。
        if ((vertices_[u].entry < root_cut_) & (target >= root_cut_)) return root_;
        if (nested_enabled_) {
            const auto& nested = nested_cuts_[static_cast<unsigned int>(vertices_[u].entry >= root_cut_)];
            if ((vertices_[u].entry < nested.boundary) & (target >= nested.boundary)) return nested.ancestor;
        }
#endif
        if (block_count_ == 0) {
            for (unsigned int remaining = std::bit_width(static_cast<unsigned int>(vertices_[u].depth - 1)); remaining != 0;) {
                const unsigned int bit = --remaining;
                const int ancestor = up_[ancestor_index(bit, u)];
                if (vertices_[ancestor].end <= target) u = ancestor;
            }
            return up_[ancestor_index(0, u)];
        }
        const std::uint32_t lowest = range_minimum(static_cast<std::size_t>(vertices_[u].entry) + 1,
            static_cast<std::size_t>(target),
            static_cast<std::uint32_t>(vertices_[roots_[vertices_[u].component]].entry));
        return static_cast<int>(deep_->rmq_storage[lowest]);
    }

    template<class GraphLike, bool endpoints_validated = false>
    void build_impl(const GraphLike& graph, int root) {
        const std::size_t n = graph.size();
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckLcaVertexCount(n);
        if (n == 0) return;
        size_ = static_cast<int>(n);
        root_ = root;
        vertices_.resize(n);
        auto temporary = std::make_unique_for_overwrite<int[]>(3 * n);
        int* const parent = temporary.get();
        int* const stack = parent + n;
        int* const order = stack + n;
        std::size_t stack_size = 0;
        std::size_t order_size = 0;
        int maximum_depth = 0;
        for (int candidate = -1; candidate < size_ && order_size != n; ++candidate) {
            const int start = candidate == -1 ? root : candidate;
            if (vertices_[start].depth != -1) continue;
            const int component = static_cast<int>(roots_.size());
            roots_.push_back(start);
            parent[start] = -1;
            vertices_[start].depth = 0;
            vertices_[start].component = component;
            stack[stack_size++] = start;
            while (stack_size != 0) {
                const int v = stack[--stack_size];
                vertices_[v].entry = static_cast<int>(order_size);
                vertices_[v].end = 1;
                order[order_size++] = v;
                for (const int to : graph[v]) {
                    if (to == parent[v]) {
                        continue;
                    }
                    if (vertices_[to].depth != -1) {
                        continue;
                    }
                    parent[to] = v;
                    vertices_[to].depth = vertices_[v].depth + 1;
                    vertices_[to].component = component;
                    if (maximum_depth < vertices_[to].depth) maximum_depth = vertices_[to].depth;
                    stack[stack_size++] = to;
                }
            }
        }
        for (std::size_t i = n; i != 0;) {
            const int v = order[--i];
            if (parent[v] != -1) vertices_[parent[v]].end += vertices_[v].end;
            vertices_[v].end += vertices_[v].entry;
        }
        shallow_ = maximum_depth <= 1;
        std::shared_ptr<DeepQueryData> construction;
        if (maximum_depth > 256) {
            construction = std::make_shared<DeepQueryData>();
            block_count_ = n / 32 + (n % 32 != 0);
            const unsigned int block_levels = static_cast<unsigned int>(std::bit_width(block_count_));
            const auto table_size = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::LcaBlockTableSize(n, block_count_, block_levels);
            construction->rmq_storage.resize(table_size);
            auto* dfs = construction->rmq_storage.data();
            auto* keys = dfs + n;
            auto* masks = keys + n;
            auto* table = masks + n;
            for (std::size_t i = 0; i < n; ++i) {
                const int v = order[i];
                dfs[i] = static_cast<std::uint32_t>(v);
                // 元の頂点番号でなく、自前のDFS順で親を比較する。
                keys[i] = static_cast<std::uint32_t>(vertices_[parent[v] == -1 ? v : parent[v]].entry);
            }
            for (std::size_t block = 0; block < block_count_; ++block) {
                const std::size_t begin = block * 32;
                const std::size_t end = std::min(n, begin + 32);
                std::uint32_t mask = 0;
                for (std::size_t i = begin; i < end; ++i) {
                    while (mask != 0) {
                        const unsigned int top = 31U - static_cast<unsigned int>(std::countl_zero(mask));
                        if (keys[begin + top] < keys[i]) break;
                        mask &= ~(std::uint32_t{1} << top);
                    }
                    mask |= std::uint32_t{1} << (i - begin);
                    masks[i] = mask;
                }
                table[block] = keys[begin + std::countr_zero(mask)];
            }
            for (unsigned int level = 1; level < block_levels; ++level) {
                const std::size_t previous = static_cast<std::size_t>(level - 1) * block_count_;
                const std::size_t current = static_cast<std::size_t>(level) * block_count_;
                const std::size_t distance = std::size_t{1} << (level - 1);
                for (std::size_t block = 0; block + (std::size_t{1} << level) <= block_count_; ++block) {
                    table[current + block] = std::min(table[previous + block],
                                                      table[previous + block + distance]);
                }
            }
        }
        if (maximum_depth > 256) {
            levels_ = 1;
            up_ = std::make_unique_for_overwrite<int[]>(n);
            std::copy_n(parent, n, up_.get());
            // 親を保存した後のDFS作業領域を高さ・長い子の計算に再利用する。
            construction->ladder.build(n, up_.get(), order,
                [this](int v) { return vertices_[v].depth; }, parent, stack);
            deep_ = std::move(construction);
#if defined(__GNUC__) && !defined(__clang__)
            choose_root_cut(order, graph[root].size());
#endif
            return;
        }
        levels_ = static_cast<unsigned int>(std::bit_width(static_cast<unsigned int>(maximum_depth)));
        if (levels_ == 0) levels_ = 1;
        const auto table_size = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::LcaAncestorTableSize(n, levels_);
        up_ = std::make_unique_for_overwrite<int[]>(table_size);
        if (levels_ == 1) {
            // 深さ1以下なら親配列だけで祖先表が完成する。
            std::copy_n(parent, n, up_.get());
            return;
        }
        // 照会・構築の実測に合わせ、Clangは頂点順、その他は段ごとの配置。
#if defined(__clang__)
        for (std::size_t i = 0; i < n; ++i) {
            const int v = order[i];
            const std::size_t base = ancestor_index(0, v);
            up_[base] = parent[v];
            const unsigned int useful_levels = std::max(1U, static_cast<unsigned int>(
                std::bit_width(static_cast<unsigned int>(vertices_[v].depth))));
            for (unsigned int bit = 1; bit < useful_levels; ++bit) {
                const int middle = up_[base + static_cast<std::size_t>(bit - 1)];
                up_[base + static_cast<std::size_t>(bit)] =
                    up_[ancestor_index(bit - 1, middle)];
            }
            // 根の0段目は-1のまま。深さを超える高段は成分根を指す。
            std::fill_n(up_.get() + base + static_cast<std::size_t>(useful_levels),
                        levels_ - useful_levels, roots_[vertices_[v].component]);
        }
#else
        std::copy_n(parent, n, up_.get());
        // 0段目の根は-1を保ち、1段目以降は根で止める。
        for (std::size_t v = 0; v < n; ++v) {
            const int par = parent[v];
            up_[n + v] = par == -1 ? static_cast<int>(v) : (parent[par] == -1 ? par : parent[par]);
        }
        // 前段の添字は非負なので、以後は深さ判定を省ける。
        for (unsigned int bit = 2; bit < levels_; ++bit) {
            const std::size_t previous = static_cast<std::size_t>(bit - 1) * n;
            const std::size_t destination = static_cast<std::size_t>(bit) * n;
            for (std::size_t v = 0; v < n; ++v) {
                const int middle = up_[previous + v];
                up_[destination + v] = up_[previous + static_cast<std::size_t>(middle)];
            }
        }
#endif
#if defined(__GNUC__) && !defined(__clang__)
        choose_root_cut(order, graph[root].size());
#endif
    }

public:
    LCA() noexcept = default;

    explicit LCA(const std::vector<std::vector<int>>& graph, int root = 0) {
        build_impl(graph, root);
    }

    LCA(int n, std::span<const std::pair<int,int>> edges, int root = 0) {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::LcaCsrGraph graph(n, edges);
        build_impl</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::LcaCsrGraph, true>(graph, root);
    }

    LCA(const LCA& other)
        : size_(other.size_), root_(other.root_), levels_(other.levels_), shallow_(other.shallow_),
#if defined(__GNUC__) && !defined(__clang__)
          root_cut_(other.root_cut_), nested_cuts_{other.nested_cuts_[0], other.nested_cuts_[1]},
          nested_enabled_(other.nested_enabled_),
#endif
          vertices_(other.vertices_), roots_(other.roots_), deep_(other.deep_),
          block_count_(other.block_count_) {
        const std::size_t count = static_cast<std::size_t>(size_) * static_cast<std::size_t>(levels_);
        if (count != 0) {
            up_ = std::make_unique_for_overwrite<int[]>(count);
            std::copy_n(other.up_.get(), count, up_.get());
        }
    }

    LCA(LCA&& other) noexcept {
        swap(other);
    }

    LCA& operator=(const LCA& other) {
        if (this != &other) {
            LCA copy(other);
            swap(copy);
        }
        return *this;
    }

    LCA& operator=(LCA&& other) noexcept {
        if (this != &other) {
            LCA moved(std::move(other));
            swap(moved);
        }
        return *this;
    }

    void swap(LCA& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(root_, other.root_);
        std::swap(levels_, other.levels_);
        std::swap(shallow_, other.shallow_);
#if defined(__GNUC__) && !defined(__clang__)
        std::swap(root_cut_, other.root_cut_);
        std::swap(nested_cuts_, other.nested_cuts_);
        std::swap(nested_enabled_, other.nested_enabled_);
#endif
        vertices_.swap(other.vertices_);
        roots_.swap(other.roots_);
        up_.swap(other.up_);
        deep_.swap(other.deep_);
        std::swap(block_count_, other.block_count_);
    }

    friend void swap(LCA& a, LCA& b) noexcept { a.swap(b); }

    void clear() noexcept {
        LCA empty;
        swap(empty);
    }

    void build(const std::vector<std::vector<int>>& graph, int root = 0) {
        LCA replacement(graph, root);
        swap(replacement);
    }

    void build(int n, std::span<const std::pair<int,int>> edges, int root = 0) {
        LCA replacement(n, edges, root);
        swap(replacement);
    }

    std::size_t size() const noexcept { return static_cast<std::size_t>(size_); }
    bool empty() const noexcept { return size_ == 0; }
    std::size_t component_count() const noexcept { return roots_.size(); }
    int root() const noexcept { return root_; }

    int parent(int v) const {
        check_vertex(v);
        return up_[ancestor_index(0, v)];
    }

    int depth_of(int v) const { check_vertex(v); return vertices_[v].depth; }
    int component_of(int v) const { check_vertex(v); return vertices_[v].component; }
    int root_of(int v) const { check_vertex(v); return roots_[vertices_[v].component]; }

    bool same_component(int u, int v) const {
        check_vertex(u);
        check_vertex(v);
        return vertices_[u].component == vertices_[v].component;
    }

    int order_index(int v) const { check_vertex(v); return vertices_[v].entry; }

    std::pair<int, int> subtree_range(int v) const {
        check_vertex(v);
        return {vertices_[v].entry, vertices_[v].end};
    }

    int subtree_size(int v) const {
        check_vertex(v);
        return vertices_[v].end - vertices_[v].entry;
    }

    bool is_ancestor(int u, int v) const {
        check_vertex(u);
        check_vertex(v);
        return vertices_[u].entry <= vertices_[v].entry && vertices_[v].entry < vertices_[u].end;
    }

#if defined(__GNUC__) || defined(__clang__)
    [[gnu::always_inline]]
#endif
    int get(int u, int v) const {
        check_vertex(u);
        check_vertex(v);
        return lca(u, v);
    }

    int operator()(int u, int v) const { return get(u, v); }

#if defined(__GNUC__) || defined(__clang__)
    [[gnu::always_inline]]
#endif
    int get(int u, int v, int new_root) const {
        check_vertex(u);
        check_vertex(v);
        check_vertex(new_root);
        if (vertices_[u].component != vertices_[v].component ||
            vertices_[u].component != vertices_[new_root].component) return -1;
        // 深さ1以下なら、異なる3頂点の根変更LCAは元の成分根。
        if (shallow_) {
            if (u == v || u == new_root) return u;
            if (v == new_root) return v;
            return roots_[vertices_[u].component];
        }
        const int a = lca(u, v);
        const int b = lca(u, new_root);
        const int c = lca(v, new_root);
        return a == b ? c : (a == c ? b : a);
    }

    int get_distance(int u, int v) const {
        check_vertex(u);
        check_vertex(v);
        const int ancestor = lca(u, v);
        if (ancestor == -1) return -1;
        return (vertices_[u].depth - vertices_[ancestor].depth) + (vertices_[v].depth - vertices_[ancestor].depth);
    }

    int kth_ancestor(int v, std::uint64_t k) const {
        check_vertex(v);
        if (k > static_cast<std::uint64_t>(vertices_[v].depth)) return -1;
        return climb(v, static_cast<unsigned int>(k));
    }

    int kth_on_path(int u, int v, std::uint64_t k) const {
        check_vertex(u);
        check_vertex(v);
        const int ancestor = lca(u, v);
        if (ancestor == -1) return -1;
        const unsigned int first = static_cast<unsigned int>(vertices_[u].depth - vertices_[ancestor].depth);
        const unsigned int second = static_cast<unsigned int>(vertices_[v].depth - vertices_[ancestor].depth);
        const std::uint64_t distance = static_cast<std::uint64_t>(first) + second;
        if (k > distance) return -1;
        if (k <= first) return climb(u, static_cast<unsigned int>(k));
        return climb(v, static_cast<unsigned int>(distance - k));
    }
};

#endif

#ifndef SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldFenwickAccess; }
#endif

#ifndef SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
#define SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T, bool>>
struct FenwickStorage { using type = T; };
template<class T>
struct FenwickStorage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct FenwickStorage<__int128> { using type = unsigned __int128; };
template<> struct FenwickStorage<unsigned __int128> { using type = unsigned __int128; };
#endif
}
#endif

#ifndef SIRKIT_FENWICK_TREE_DEFINED
#define SIRKIT_FENWICK_TREE_DEFINED
namespace detail {
inline std::size_t CheckedFenwickStorageSize(std::size_t n, std::size_t limit) {
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()) || n >= limit)
        throw std::length_error("fenwick_tree: size");
    return n == 0 ? 0 : n + 1;
}
}

template<class T>
class fenwick_tree {
    // HLD所有時の読取り専用接続。通常のFenwick操作には使わない。
    using HldFenwickTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldFenwickAccess;
    using Storage = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FenwickStorage<T>::type;
    std::vector<Storage> tree_;
    static decltype(auto) to_storage(const T& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<Storage>(value);
    }
    static decltype(auto) from_storage(Storage& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<T>(value);
    }
    static std::size_t checked_size(int n) {
        return static_cast<std::size_t>(n);
    }
    static std::size_t storage_size(std::size_t n) {
        return detail::CheckedFenwickStorageSize(n, std::vector<Storage>{}.max_size());
    }
    static std::size_t lowbit(std::size_t x) noexcept { return x & (~x + 1); }
    void check_index(int i) const {
        (void)i;
    }
    void check_range(int l, int r) const {
        (void)l; (void)r;
    }
    Storage prefix_unchecked(std::size_t r) const {
        Storage result{};
        for (; r; r -= lowbit(r)) result += tree_[r];
        return result;
    }
    Storage get_unchecked(std::size_t i) const {
        auto node = i + 1;
        Storage result = tree_[node];
        const auto begin = node - lowbit(node);
        for (--node; node > begin; node -= lowbit(node)) result -= tree_[node];
        return result;
    }
    void add_unchecked(std::size_t i, const Storage& delta) {
        const auto n = static_cast<std::size_t>(size());
        for (++i;;) {
            tree_[i] += delta;
            const auto step = lowbit(i);
            if (step > n - i) break;
            i += step;
        }
    }
public:
    fenwick_tree() = default;
    explicit fenwick_tree(int n) : tree_(storage_size(checked_size(n)), Storage{}) {}
    template<class Values> requires std::is_same_v<Values, std::vector<T>>
    explicit fenwick_tree(const Values& values) : tree_(storage_size(values.size()), Storage{}) {
        const auto n = values.size();
        for (std::size_t i = 1; i <= n; ++i) {
            tree_[i] += to_storage(values[i - 1]);
            const auto step = lowbit(i);
            if (step <= n - i) tree_[i + step] += tree_[i];
        }
    }
    fenwick_tree(const fenwick_tree&) = default;
    fenwick_tree& operator=(const fenwick_tree&) = default;
    fenwick_tree(fenwick_tree&& other) noexcept : tree_(std::move(other.tree_)) { other.tree_.clear(); }
    fenwick_tree& operator=(fenwick_tree&& other) noexcept {
        if (this != &other) { tree_ = std::move(other.tree_); other.tree_.clear(); }
        return *this;
    }
    int size() const noexcept { return tree_.empty() ? 0 : static_cast<int>(tree_.size() - 1); }
    bool empty() const noexcept { return size() == 0; }
    void add(int i, T delta) {
        check_index(i);
        add_unchecked(static_cast<std::size_t>(i), to_storage(delta));
    }
    T get(int i) const { check_index(i); return static_cast<T>(get_unchecked(static_cast<std::size_t>(i))); }
    void set(int i, T value) {
        check_index(i);
        const Storage delta = to_storage(value) - get_unchecked(static_cast<std::size_t>(i));
        add_unchecked(static_cast<std::size_t>(i), delta);
    }
    T sum(int l, int r) {
        check_range(l, r);
        return static_cast<T>(prefix_unchecked(static_cast<std::size_t>(r)) - prefix_unchecked(static_cast<std::size_t>(l)));
    }
    int lower_bound(const T& target) const {
        if (!(T{} < target)) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (from_storage(candidate) < target) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    int upper_bound(const T& target) const {
        if (target < T{}) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (!(target < from_storage(candidate))) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    std::vector<T> values() const {
        if (tree_.empty()) return {};
        std::vector<T> result(tree_.begin() + 1, tree_.end());
        const auto n = static_cast<std::size_t>(size());
        for (auto i = n; i; --i) {
            const auto step = lowbit(i);
            if (step <= n - i) {
                if constexpr (std::is_same_v<T, Storage>) result[i + step - 1] -= result[i - 1];
                else {
                    const Storage difference = static_cast<Storage>(result[i + step - 1]) - static_cast<Storage>(result[i - 1]);
                    result[i + step - 1] = static_cast<T>(difference);
                }
            }
        }
        return result;
    }
    void reset(int n) {
        std::vector<Storage> replacement(storage_size(checked_size(n)), Storage{});
        tree_.swap(replacement);
    }
    void reset() { std::fill(tree_.begin(), tree_.end(), Storage{}); }
    void clear() noexcept { tree_.clear(); }
    void swap(fenwick_tree& other) noexcept { tree_.swap(other.tree_); }
};
#endif

#ifndef SIRKIT_SUBTREE_EDGE_ADD_DISTANCES_DEFINED
#define SIRKIT_SUBTREE_EDGE_ADD_DISTANCES_DEFINED

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class SubtreeEdgeAddDistances
{
public:
    struct Edge
    {
        int from;
        int to;
        T weight{};
    };

    SubtreeEdgeAddDistances(int n, std::span<const Edge> edges, int root = 0)
    {
        std::vector<std::pair<int, int>> endpoints;
        endpoints.reserve(edges.size());
        for (const auto &[u, v, weight] : edges)
        {
            endpoints.emplace_back(u, v);
        }
        lca_.build(n, endpoints, root);
        std::vector<Coefficients> initial(n);
        for (const auto &[u, v, weight] : edges)
        {
            const int child = lca_.parent(v) == u ? v : u;
            initial[lca_.order_index(child)].offset = static_cast<std::uint64_t>(weight);
        }
        std::vector<int> parent_position(n, -1);
        for (int v = 0; v < n; ++v)
        {
            if (const int p = lca_.parent(v); p != -1)
            {
                parent_position[lca_.order_index(v)] = lca_.order_index(p);
            }
        }
        for (int i = 0; i < n; ++i)
        {
            if (const int p = parent_position[i]; p != -1)
            {
                initial[i].offset += initial[p].offset;
            }
        }
        for (int i = n - 1; i > 0; --i)
        {
            initial[i].offset -= initial[i - 1].offset;
        }
        differences_ = fenwick_tree<Coefficients>(initial);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lca_.size();
    }

    void add_subtree(int vertex, T delta)
    {
        const auto [first, last] = lca_.subtree_range(vertex);
        const auto slope = static_cast<std::uint64_t>(delta);
        const auto product = slope * static_cast<std::uint64_t>(lca_.depth_of(vertex));
        // 子孫 w の根距離には delta * (depth(w) - depth(vertex)) を足す。
        differences_.add(first, {slope, 0 - product});
        if (last < differences_.size())
        {
            differences_.add(last, {0 - slope, product});
        }
    }

    [[nodiscard]] T root_distance(int vertex)
    {
        return static_cast<T>(root_sum(vertex));
    }

    [[nodiscard]] std::optional<T> distance(int u, int v)
    {
        const int common = lca_.get(u, v);
        if (common == -1)
        {
            return std::nullopt;
        }
        return static_cast<T>(root_sum(u) + root_sum(v) - 2 * root_sum(common));
    }

private:
    struct Coefficients
    {
        std::uint64_t slope = 0;
        std::uint64_t offset = 0;

        void operator+=(Coefficients other)
        {
            slope += other.slope;
            offset += other.offset;
        }

        friend Coefficients operator-(Coefficients left, Coefficients right)
        {
            return {left.slope - right.slope, left.offset - right.offset};
        }
    };

    [[nodiscard]] std::uint64_t root_sum(int vertex)
    {
        const auto sum = differences_.sum(0, lca_.order_index(vertex) + 1);
        return sum.offset + sum.slope * static_cast<std::uint64_t>(lca_.depth_of(vertex));
    }

    LCA lca_;
    fenwick_tree<Coefficients> differences_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
