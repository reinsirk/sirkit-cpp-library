#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/awc0100/tasks/awc0100_j

// Library: Graph/Tree_Path_Difference.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Path_Difference.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_path_difference.md
//
// API一覧
// TreePathDifference<T=int64_t>(n,edges,root=0): 無向森を用意。各辺を pair<int,int> で1回渡す。
//   頂点は0始まり。構築・領域 O(n+B log(B+1))、B=ceil(n/32)。
// add_vertex_path(u,v,delta): 両端を含むパス上の頂点に delta を加算。O(1)。
// add_edge_path(u,v,delta): パス上の辺に delta を加算。O(1)。
//   どちらも別成分なら加算せず false。同一成分なら true。
// vertex_values()/edge_values(): 頂点番号順/入力辺順の累計。取得後も加算できる。O(n) 時間・追加領域。
//   Tはboolを除く64ビット以下の整数型。加減算は型幅で周回する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
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
    int root_cut_ = 0;
    struct NestedCut { int boundary = 0; int ancestor = 0; };
    NestedCut nested_cuts_[2]{};
    bool nested_enabled_ = false;
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
        return static_cast<std::size_t>(bit) * static_cast<std::size_t>(size_) + static_cast<std::size_t>(v);
    }

    int climb(int v, std::uint64_t k) const noexcept {
        if (k == 0) return v;
        if (block_count_ != 0) return deep_->ladder.climb(v, k, up_.get());
        while (k != 0) {
            const unsigned int bit = static_cast<unsigned int>(std::bit_width(k) - 1);
            v = up_[ancestor_index(bit, v)];
            k ^= std::uint64_t{1} << bit;
        }
        return v;
    }

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

    std::uint32_t block_minimum(std::size_t left, std::size_t right) const noexcept {
        const auto* keys = deep_->rmq_storage.data() + size_;
        const auto* masks = keys + size_;
        const std::uint32_t candidates = masks[right] & (~std::uint32_t{0} << (left & 31U));
        return keys[(left & ~std::size_t{31}) + std::countr_zero(candidates)];
    }

    [[gnu::noinline]]
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

    [[gnu::always_inline]]
    int lca(int u, int v) const noexcept {
        if (u == v) return u;
        if (vertices_[u].component != vertices_[v].component) return -1;
        if (vertices_[u].entry > vertices_[v].entry) std::swap(u, v);
        const int target = vertices_[v].entry;
        if (target < vertices_[u].end) return u;
        // 根の子部分木を分けるDFS境界をまたぐとき、LCAは根。
        if ((vertices_[u].entry < root_cut_) & (target >= root_cut_)) return root_;
        if (nested_enabled_) {
            const auto& nested = nested_cuts_[static_cast<unsigned int>(vertices_[u].entry >= root_cut_)];
            if ((vertices_[u].entry < nested.boundary) & (target >= nested.boundary)) return nested.ancestor;
        }
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
            choose_root_cut(order, graph[root].size());
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
        choose_root_cut(order, graph[root].size());
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
          root_cut_(other.root_cut_), nested_cuts_{other.nested_cuts_[0], other.nested_cuts_[1]},
          nested_enabled_(other.nested_enabled_),
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
        std::swap(root_cut_, other.root_cut_);
        std::swap(nested_cuts_, other.nested_cuts_);
        std::swap(nested_enabled_, other.nested_enabled_);
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

    [[gnu::always_inline]]
    int get(int u, int v) const {
        check_vertex(u);
        check_vertex(v);
        return lca(u, v);
    }

    int operator()(int u, int v) const { return get(u, v); }

    [[gnu::always_inline]]
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

#ifndef SIRKIT_TREE_PATH_DIFFERENCE_DEFINED
#define SIRKIT_TREE_PATH_DIFFERENCE_DEFINED

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class TreePathDifference
{
public:
    TreePathDifference(int n, std::span<const std::pair<int, int>> edges, int root = 0)
        : lca_(n, edges, root),
          vertices_(n),
          edges_(n),
          child_(edges.size()),
          order_(n)
    {
        for (int v = 0; v < n; ++v)
        {
            order_[lca_.order_index(v)] = v;
        }
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [u, v] = edges[i];
            child_[i] = lca_.parent(u) == v ? u : v;
        }
    }

    bool add_vertex_path(int u, int v, T delta)
    {
        const int common = lca_.get(u, v);
        if (common == -1)
        {
            return false;
        }
        const auto value = static_cast<std::uint64_t>(delta);
        vertices_[u] += value;
        vertices_[v] += value;
        vertices_[common] -= value;
        if (const int parent = lca_.parent(common); parent != -1)
        {
            vertices_[parent] -= value;
        }
        return true;
    }

    bool add_edge_path(int u, int v, T delta)
    {
        const int common = lca_.get(u, v);
        if (common == -1)
        {
            return false;
        }
        const auto value = static_cast<std::uint64_t>(delta);
        edges_[u] += value;
        edges_[v] += value;
        edges_[common] -= 2 * value;
        return true;
    }

    [[nodiscard]] std::vector<T> vertex_values() const
    {
        const auto totals = accumulate(vertices_);
        return std::vector<T>(totals.begin(), totals.end());
    }

    [[nodiscard]] std::vector<T> edge_values() const
    {
        const auto totals = accumulate(edges_);
        std::vector<T> result;
        result.reserve(child_.size());
        for (int v : child_)
        {
            result.push_back(static_cast<T>(totals[v]));
        }
        return result;
    }

private:
    [[nodiscard]] std::vector<std::uint64_t> accumulate(std::vector<std::uint64_t> difference) const
    {
        for (auto it = order_.rbegin(); it != order_.rend(); ++it)
        {
            const int v = *it;
            if (const int parent = lca_.parent(v); parent != -1)
            {
                difference[parent] += difference[v];
            }
        }
        return difference;
    }

    LCA lca_;
    std::vector<std::uint64_t> vertices_;
    std::vector<std::uint64_t> edges_;
    std::vector<int> child_;
    std::vector<int> order_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


// Library: Graph/Heavy_Light_Decomposition.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Heavy_Light_Decomposition.cpp
// 参考（API設計・CC0）: https://github.com/NyaanNyaan/library/blob/2e713880f0d5041d794e0f09c7f31c1826ba0ccf/tree/heavy-light-decomposition.hpp
// 参考元の提供条件: https://github.com/NyaanNyaan/library/blob/2e713880f0d5041d794e0f09c7f31c1826ba0ccf/LICENSE
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/heavy_light_decomposition.md
//
// API一覧
// [共通型] Graphはvector<vector<int>>。HeavyLightDecomposition::Intervalは{int left,right;bool reversed;}。
// [値を持つHLD: HeavyLightValues<Range,Ordered=false>]
// value_typeはBITのT/Segtree・LazyのS。HeavyLightValues()は空。コピー構築/代入は独立、移動構築はO(1)。
// operator=(other)は自身への参照を返す。代入/buildは旧状態の破棄費用も加わる。
// with_values<Range,Ordered>(values): values[v]を頂点vの値とする長さNのvectorから、分解と集計器を独立所有するHLDを返す。
// 元の値配列とは更新を共有しない。get/set/add/applyのvは頂点番号で、index(v)への変換は不要。
// 一時HLDなら分解を移動、既存HLDならコピー。RangeはBIT/ACL形式Segtree/Lazy。値の型はRangeから推論。
// HeavyLightValues(g,values,root=0)/build(g,values,root=0): 分解と値を同時に構築/再構築。失敗時は元を保持。
// HeavyLightValues(layout,values): 分解をコピー、std::move(layout)なら移動。移動後の失敗では元分解が空の場合あり。
// 構築は分解O(N+M)+集計器のvector構築。vector構築なしのBITはN回add。所有状態を1回確保する。
// get(v)/set(v,x)/add(v,x): 頂点番号で取得/代入/加算。addは対応BITのみ。通常の集計器ではO(log N)。
// apply(v,f)/path_apply(u,v,f,edges=false)/subtree_apply(v,f,edges=false): 点/経路/部分木へ作用。
// path_prod(u,v,edges=false)/subtree_prod(v,edges=false): 所有集計器で経路/部分木を集計。通常O(log²N)/O(log N)。
// Ordered=falseは和・最小値などの可換演算用。trueは文字列連結などをu→vの順に集計。領域と更新は集計器2本分。
// trueは値の並び順を守る設定で、個々の辺の値を逆元へ変える機能ではない。部分木の積はDFS添字昇順。
// 辺モードは子vに親辺の値を置き、各根には0/e()を置く。初期値はN個必要。edges=trueでLCA/部分木の指定頂点を除く。
// path_prod(v,v)は頂点の値、path_prod(v,v,true)や葉の辺部分木は空区間の単位元。空区間の更新は何もしない。
// values(): 元の頂点順へ復元。Range.valuesがあればO(N)の並べ直し、なければN回点照会。
// clear()/swap(other): 空へ/交換。コピーは独立、move元は空。build/コピー失敗時は元を保持。
// 分解情報・reorder/restore/for_each_path/pathも同名で取得可能。対応applyを持つRangeでのみ作用APIを使用できる。
// 所有型にInterval型別名・with_values・外部集計器を受けるオーバーロードはない。pathの要素型は上記Interval。
// 値の照会も非constの所有HLDから呼ぶ。根を変えて辺の値をbuildする場合は、新しい親子対応で値を配置し直す。
// 値の更新中の例外では変更は巻き戻さない。Ordered=trueでは例外後にclear/buildで再構築すること。
// [分解だけを持つHLD: HeavyLightDecomposition]
// HeavyLightDecomposition()は空、コピー構築はO(N)、移動構築はO(1)。operator=(other)は自身への参照を返す。
// コピー・移動代入とbuildは旧状態の破棄費用も加わる。
// HeavyLightDecomposition(g,root=0)/build(g,root=0): 無向森を反復DFSで分解。O(n+m)時間・領域。
// 頂点番号[0,n)。隣接リストの対称性は前提。rootの木から、残りは最小未訪問頂点から構築する。
// 失敗時は元を保持。空のgにはroot=0。閉路・自己辺・多重辺は検査有効時に例外。
// size()/empty()/root()/component_count(): 頂点数/空か/指定根（空なら-1）/木の数。O(1)。
// parent(v)/depth(v)/component(v)/root_of(v): 親（根は-1）/深さ/木の番号/木の根。O(1)。
// index(v)/vertex_at(i)/order(): heavy優先DFS添字/対応頂点/順序配列へのconst参照。O(1)。
// same_component(u,v)/is_ancestor(u,v): 同じ木か/祖先か（自身を含む）。O(1)。
// subtree_range(v,edges=false): 部分木の半開区間。edges=trueは根vを除き子側の頂点で辺を表す。O(1)。
// subtree_size(v): 部分木の頂点数。O(1)。lca(u,v)/distance(u,v): LCA/辺数、異なる木は-1。O(log n)。
// kth_ancestor(v,k): k個上の頂点。k<0/根を越すと-1。jump(u,v,k): u→vのk番目、範囲外/異なる木は-1。O(log n)。
// for_each_path(u,v,fn,edges=false): 通行順にfn(left,right,reversed)を呼ぶ。fnの戻り値は無視。接続ならtrue、異なる木はfalse。
// reversed=trueはright-1からleftへ読む。区間は半開。edges=trueはLCAを除く。O(log n)、ヒープ確保なし。
// path(u,v,edges=false): 同じ順序のInterval{left,right,reversed}配列を返す。異なる木は空。O(log n)時間・領域。
// clear()/swap(other): 空に戻す/O(1)交換。コピーは独立所有、ムーブ元は空。検査はSKIP_RUNTIME_CHECKSで除く。
// reorder(values)/restore(ordered): 長さNのvectorをDFS添字順/元の頂点順へコピー。O(N)時間・領域。
// path_prod(u,v,bit,edges=false): sumを持つBITの和。単位元はT{}、整数は型幅で周回。O(log N)回照会。
// path_prod(u,v,seg,edges=false): ACL形式のSegtreeからop/eを推論した可換積。Segはconst可、Lazyは非const。
// path_prod(u,v,identity,query,op,edges=false): queryはprod/sum、2引数関数なら可換、3引数(l,r,reversed)なら向き対応。
// path_prod(u,v,seg,reverse,edges=false)/汎用版のop直後にreverse指定: 正しい逆順集約を返す非可換積。
// subtree_prod(v,query,edges=false): 部分木のDFS添字昇順の積。区間照会1回。非可換も可。
// path_apply(u,v,range,f,edges=false)/subtree_apply(v,range,f,edges=false): range.apply(l,r,f)で更新。O(log N)/1回。
// prod/applyのrangeは同じDFS添字順で用意する。path_prod/path_applyは同じ木が前提、検査時は別木を例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_HLD_QUERY_HELPERS_DEFINED
#define SIRKIT_HLD_QUERY_HELPERS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline int HldVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("HLD: int size overflow");
    return static_cast<int>(vertices);
}

template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T,bool>>
struct HldArithmetic {
    static T add(const T& a,const T& b) { return a+b; }
    static T subtract(const T& a,const T& b) { return a-b; }
};
template<class T> struct HldArithmetic<T,true> {
    using U=std::make_unsigned_t<T>;
    static T add(T a,T b) { return static_cast<T>(static_cast<U>(a)+static_cast<U>(b)); }
    static T subtract(T a,T b) { return static_cast<T>(static_cast<U>(a)-static_cast<U>(b)); }
};
#if defined(__SIZEOF_INT128__)
template<> struct HldArithmetic<__int128,false> {
    using U=unsigned __int128;
    static __int128 add(__int128 a,__int128 b) { return static_cast<__int128>(static_cast<U>(a)+static_cast<U>(b)); }
    static __int128 subtract(__int128 a,__int128 b) { return static_cast<__int128>(static_cast<U>(a)-static_cast<U>(b)); }
};
template<> struct HldArithmetic<unsigned __int128,false> {
    using U=unsigned __int128;
    static U add(U a,U b) { return a+b; }
    static U subtract(U a,U b) { return a-b; }
};
#endif
template<class Q> concept HldQuery = requires(Q& q) { q.prod(0,0); } ||
    requires(Q& q) { q.sum(0,0); } || std::invocable<Q&,int,int,bool> || std::invocable<Q&,int,int>;
template<class Q> decltype(auto) hld_query(Q& q,int l,int r,bool reverse=false) {
    if constexpr (requires { q.prod(l,r); }) return q.prod(l,r);
    else if constexpr (requires { q.sum(l,r); }) return q.sum(l,r);
    else if constexpr (std::invocable<Q&,int,int,bool>) return std::invoke(q,l,r,reverse);
    else return std::invoke(q,l,r);
}
template<class Q> using HldQueryValue=std::remove_cvref_t<decltype(hld_query(std::declval<Q&>(),0,0))>;
template<class Q> inline constexpr bool hld_directional_query =
    !requires(Q& q) { q.prod(0,0); } && !requires(Q& q) { q.sum(0,0); } && std::invocable<Q&,int,int,bool>;
template<class I,class Q,class Op> concept HldProduct = HldQuery<Q> &&
    std::constructible_from<HldQueryValue<Q>,I> &&
    std::is_invocable_r_v<HldQueryValue<Q>,Op&,HldQueryValue<Q>,HldQueryValue<Q>>;
template<class I,class Q,class Op,class Reverse> concept HldReversedProduct = HldProduct<I,Q,Op> &&
    std::is_invocable_r_v<HldQueryValue<Q>,Reverse&,HldQueryValue<Q>> &&
    (!hld_directional_query<Q> || std::is_same_v<std::remove_cvref_t<Reverse>,std::identity>);
}
#endif

#ifndef SIRKIT_HEAVY_LIGHT_VALUES_FORWARD_DEFINED
#define SIRKIT_HEAVY_LIGHT_VALUES_FORWARD_DEFINED
template<class Range,bool Ordered=false> class HeavyLightValues;
#endif

#ifndef SIRKIT_HEAVY_LIGHT_DECOMPOSITION_DEFINED
#define SIRKIT_HEAVY_LIGHT_DECOMPOSITION_DEFINED
class HeavyLightDecomposition {
    std::vector<int> parent_,depth_,component_,roots_,index_,order_,subtree_,head_;
    int root_=-1;
    void check_vertex(int v) const {
        (void)v;
    }
    void check_size(std::size_t n) const {
        (void)n;
    }
    void construct(const Graph& graph,int root) {
        const int n=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldVertexCount(graph.size());
        if(n==0) return;
        root_=root;
        parent_.assign(n,-2); depth_.resize(n); component_.resize(n);
        index_.resize(n); subtree_.assign(n,1); head_.resize(n); order_.reserve(n);
        std::vector<int> pending,discovery,heavy(n,-1);
        pending.reserve(n); discovery.reserve(n);
        const auto grow=[&](int start) {
            const int id=static_cast<int>(roots_.size()); roots_.push_back(start);
            parent_[start]=-1;component_[start]=id;pending.push_back(start);
            while(!pending.empty()) {
                const int v=pending.back();pending.pop_back();discovery.push_back(v);
                for(int u:graph[v]) {
                    if(u==parent_[v]) {
                        continue;
                    }
                    parent_[u]=v;depth_[u]=depth_[v]+1;component_[u]=id;pending.push_back(u);
                }
            }
        };
        grow(root);
        for(int v=0;v<n;++v) if(parent_[v]==-2) grow(v);
        for(auto it=discovery.rbegin();it!=discovery.rend();++it) {
            const int v=*it,p=parent_[v];
            if(p<0) continue;
            subtree_[p]+=subtree_[v];
            if(heavy[p]<0 || subtree_[v]>subtree_[heavy[p]]) heavy[p]=v;
        }
        for(int start:roots_) {
            pending.push_back(start);
            while(!pending.empty()) {
                const int chain=pending.back();pending.pop_back();
                for(int v=chain;v!=-1;v=heavy[v]) {
                    head_[v]=chain;index_[v]=static_cast<int>(order_.size());order_.push_back(v);
                    for(auto it=graph[v].rbegin();it!=graph[v].rend();++it)
                        if(parent_[*it]==v && *it!=heavy[v]) pending.push_back(*it);
                }
            }
        }
    }
public:
    struct Interval { int left;int right;bool reversed; };
    HeavyLightDecomposition()=default;
    explicit HeavyLightDecomposition(const Graph& graph,int root=0) { construct(graph,root); }
    HeavyLightDecomposition(const HeavyLightDecomposition&)=default;
    HeavyLightDecomposition(HeavyLightDecomposition&& other) noexcept { swap(other); }
    HeavyLightDecomposition& operator=(HeavyLightDecomposition other) noexcept { swap(other);return *this; }
    void swap(HeavyLightDecomposition& other) noexcept {
        parent_.swap(other.parent_);depth_.swap(other.depth_);component_.swap(other.component_);
        roots_.swap(other.roots_);index_.swap(other.index_);order_.swap(other.order_);
        subtree_.swap(other.subtree_);head_.swap(other.head_);std::swap(root_,other.root_);
    }
    void clear() noexcept { HeavyLightDecomposition{}.swap(*this); }
    void build(const Graph& graph,int root=0) { HeavyLightDecomposition(graph,root).swap(*this); }
    int size() const noexcept { return static_cast<int>(order_.size()); }
    bool empty() const noexcept { return order_.empty(); }
    int root() const noexcept { return root_; }
    int component_count() const noexcept { return static_cast<int>(roots_.size()); }
    int parent(int v) const { check_vertex(v);return parent_[v]; }
    int depth(int v) const { check_vertex(v);return depth_[v]; }
    int component(int v) const { check_vertex(v);return component_[v]; }
    int root_of(int v) const { check_vertex(v);return roots_[component_[v]]; }
    int index(int v) const { check_vertex(v);return index_[v]; }
    int vertex_at(int i) const { check_vertex(i);return order_[i]; }
    const std::vector<int>& order() const noexcept { return order_; }
    bool same_component(int u,int v) const { check_vertex(u);check_vertex(v);return roots_.size()==1 || component_[u]==component_[v]; }
    bool is_ancestor(int u,int v) const {
        check_vertex(u);check_vertex(v);return component_[u]==component_[v] && index_[u]<=index_[v] && index_[v]-index_[u]<subtree_[u];
    }
    std::pair<int,int> subtree_range(int v,bool edges=false) const {
        check_vertex(v);return {index_[v]+static_cast<int>(edges),index_[v]+subtree_[v]};
    }
    int subtree_size(int v) const { check_vertex(v);return subtree_[v]; }
    int lca(int u,int v) const {
        if(!same_component(u,v)) return -1;
        while(head_[u]!=head_[v]) {
            if(depth_[head_[u]]>=depth_[head_[v]]) u=parent_[head_[u]];
            else v=parent_[head_[v]];
        }
        return depth_[u]<depth_[v]?u:v;
    }
    int distance(int u,int v) const {
        const int a=lca(u,v);return a<0?-1:(depth_[u]-depth_[a])+(depth_[v]-depth_[a]);
    }
    int kth_ancestor(int v,std::int64_t k) const {
        check_vertex(v);if(k<0 || k>depth_[v]) return -1;
        while(k>depth_[v]-depth_[head_[v]]) {
            k-=depth_[v]-depth_[head_[v]]+1;v=parent_[head_[v]];
        }
        return order_[index_[v]-static_cast<int>(k)];
    }
    int jump(int u,int v,std::int64_t k) const {
        check_vertex(u);check_vertex(v);
        if(k<0) return -1;
        if(head_[u]==head_[v]) {
            const int delta=index_[v]-index_[u];
            if(k>(delta<0?-delta:delta)) return -1;
            return order_[delta<0?index_[u]-static_cast<int>(k):index_[u]+static_cast<int>(k)];
        }
        if(roots_.size()!=1 && component_[u]!=component_[v]) return -1;
        if(k>static_cast<std::int64_t>(depth_[u])+depth_[v]) return -1;
        if(k==0) return u;
        const int a=lca(u,v);
        const int before=depth_[u]-depth_[a],after=depth_[v]-depth_[a];
        if(k>static_cast<std::int64_t>(before)+after) return -1;
        return k<=before?kth_ancestor(u,k):kth_ancestor(v,static_cast<std::int64_t>(before)+after-k);
    }
    template<class Fn> bool for_each_path(int u,int v,Fn&& fn,bool edges=false) const {
        if(!same_component(u,v)) return false;
        // 軽辺を一つ上がるごとに部分木が倍以上になるため、intの桁数で足りる。
        std::array<Interval,std::numeric_limits<unsigned>::digits> tail;
        std::size_t count=0;
        while(head_[u]!=head_[v]) {
            if(index_[u]>=index_[v]) {
                std::invoke(fn,index_[head_[u]],index_[u]+1,true);u=parent_[head_[u]];
            } else {
                tail[count++]={index_[head_[v]],index_[v]+1,false};v=parent_[head_[v]];
            }
        }
        if(index_[u]>=index_[v]) {
            const int left=index_[v]+static_cast<int>(edges),right=index_[u]+1;
            if(left<right) std::invoke(fn,left,right,true);
        } else {
            const int left=index_[u]+static_cast<int>(edges),right=index_[v]+1;
            std::invoke(fn,left,right,false);
        }
        while(count) { const auto p=tail[--count];std::invoke(fn,p.left,p.right,p.reversed); }
        return true;
    }
    std::vector<Interval> path(int u,int v,bool edges=false) const {
        std::vector<Interval> result;
        for_each_path(u,v,[&](int l,int r,bool rev){result.push_back({l,r,rev});},edges);return result;
    }
    template<class T> std::vector<T> reorder(const std::vector<T>& values) const {
        check_size(values.size());std::vector<T> result;result.reserve(values.size());
        for(int v:order_) result.push_back(values[v]);
        return result;
    }
    template<class T> std::vector<T> restore(const std::vector<T>& values) const {
        check_size(values.size());std::vector<T> result;result.reserve(values.size());
        for(int i:index_) result.push_back(values[i]);
        return result;
    }
    template<class Identity,class Q,class Op> requires /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldProduct<Identity,Q,Op>
    auto path_prod(int u,int v,Identity&& identity,Q&& query,Op&& op,bool edges=false) const {
        if constexpr(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_directional_query<Q>) {
            return path_prod(u,v,std::forward<Identity>(identity),std::forward<Q>(query),std::forward<Op>(op),std::identity{},edges);
        } else {
            using Value=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldQueryValue<Q>;
            if(!same_component(u,v)) {
                return Value(std::forward<Identity>(identity));
            }
            Value result(std::forward<Identity>(identity));
            // 可換積では通行順の保存を省き、両端から直接結合する。
            while(head_[u]!=head_[v]) {
                if(index_[u]<index_[v]) std::swap(u,v);
                result=std::invoke(op,std::move(result),/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,index_[head_[u]],index_[u]+1));
                u=parent_[head_[u]];
            }
            if(index_[u]>index_[v]) std::swap(u,v);
            const int left=index_[u]+static_cast<int>(edges),right=index_[v]+1;
            if(left<right) result=std::invoke(op,std::move(result),/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,left,right));
            return result;
        }
    }
    template<class Identity,class Q,class Op,class Reverse> requires /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldReversedProduct<Identity,Q,Op,Reverse>
    auto path_prod(int u,int v,Identity&& identity,Q&& query,Op&& op,Reverse&& reverse,bool edges=false) const {
        using Value=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldQueryValue<Q>;
        if(!same_component(u,v)) {
            return Value(std::forward<Identity>(identity));
        }
        Value empty_result(std::forward<Identity>(identity));
        std::optional<Value> from_u,from_v;
        const auto read=[&](int l,int r,bool rev) -> Value {
            Value part=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,l,r,rev);
            if constexpr(!/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_directional_query<Q>) if(rev) part=std::invoke(reverse,std::move(part));
            return part;
        };
        const auto append=[&](Value part) {
            if(from_u) *from_u=std::invoke(op,std::move(*from_u),std::move(part));
            else from_u.emplace(std::move(part));
        };
        const auto prepend=[&](Value part) {
            if(from_v) *from_v=std::invoke(op,std::move(part),std::move(*from_v));
            else from_v.emplace(std::move(part));
        };
        while(head_[u]!=head_[v]) {
            if(index_[u]>=index_[v]) {
                append(read(index_[head_[u]],index_[u]+1,true));u=parent_[head_[u]];
            } else {
                prepend(read(index_[head_[v]],index_[v]+1,false));v=parent_[head_[v]];
            }
        }
        if(index_[u]>=index_[v]) {
            const int left=index_[v]+static_cast<int>(edges),right=index_[u]+1;
            if(left<right) append(read(left,right,true));
        } else {
            const int left=index_[u]+static_cast<int>(edges),right=index_[v]+1;
            append(read(left,right,false));
        }
        if(from_u && from_v) return Value(std::invoke(op,std::move(*from_u),std::move(*from_v)));
        if(from_u) return std::move(*from_u);
        if(from_v) return std::move(*from_v);
        return empty_result;
    }
    template<class Range> requires (requires(Range& r){r.sum(0,0);} && !requires(Range& r){r.prod(0,0);})
    auto path_prod(int u,int v,Range& range,bool edges=false) const {
        using Value=std::remove_cvref_t<decltype(range.sum(0,0))>;
        return path_prod(u,v,Value{},range,[](const Value& a,const Value& b){return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldArithmetic<Value>::add(a,b);},edges);
    }
    template<template<class,auto,auto> class R,class S,auto Op,auto E>
    S path_prod(int u,int v,const R<S,Op,E>& range,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,edges);
    }
    template<template<class,auto,auto> class R,class S,auto Op,auto E,class Reverse>
        requires std::is_invocable_r_v<S,Reverse&,S>
    S path_prod(int u,int v,const R<S,Op,E>& range,Reverse&& reverse,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,std::forward<Reverse>(reverse),edges);
    }
    template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I>
    S path_prod(int u,int v,R<S,Op,E,F,M,C,I>& range,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,edges);
    }
    template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I,class Reverse>
        requires std::is_invocable_r_v<S,Reverse&,S>
    S path_prod(int u,int v,R<S,Op,E,F,M,C,I>& range,Reverse&& reverse,bool edges=false) const {
        return path_prod(u,v,std::invoke(E),range,Op,std::forward<Reverse>(reverse),edges);
    }
    template<class Q> requires /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldQuery<Q>
    auto subtree_prod(int v,Q&& query,bool edges=false) const {
        auto[l,r]=subtree_range(v,edges);return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(query,l,r);
    }
    template<class Range,class F> void path_apply(int u,int v,Range& range,const F f,bool edges=false) const {
        for_each_path(u,v,[&](int l,int r,bool){range.apply(l,r,f);},edges);
    }
    template<class Range,class F> void subtree_apply(int v,Range& range,const F f,bool edges=false) const {
        auto[l,r]=subtree_range(v,edges);if(l<r) range.apply(l,r,f);
    }
    template<class Range,bool Ordered=false,class T>
    HeavyLightValues<Range,Ordered> with_values(const std::vector<T>& values) const &;
    template<class Range,bool Ordered=false,class T>
    HeavyLightValues<Range,Ordered> with_values(const std::vector<T>& values) &&;
};
#endif

#ifndef SIRKIT_HLD_OWNED_RANGE_TRAITS_DEFINED
#define SIRKIT_HLD_OWNED_RANGE_TRAITS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class Range> struct HldOwnedRangeTraits {
    using Value=HldQueryValue<Range>;
    static Value identity() {return Value{};}
    static Value combine(const Value& a,const Value& b) {return HldArithmetic<Value>::add(a,b);}
};
template<template<class,auto,auto> class R,class S,auto Op,auto E>
struct HldOwnedRangeTraits<R<S,Op,E>> {
    using Value=S;
    static Value identity(){return std::invoke(E);}
    static Value combine(const Value& a,const Value& b){return std::invoke(Op,a,b);}
};
template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I>
struct HldOwnedRangeTraits<R<S,Op,E,F,M,C,I>> {
    using Value=S;
    static Value identity(){return std::invoke(E);}
    static Value combine(const Value& a,const Value& b){return std::invoke(Op,a,b);}
};
}
#endif

#ifndef SIRKIT_HLD_SEGMENT_ACCESS_DEFINED
#define SIRKIT_HLD_SEGMENT_ACCESS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class Range> struct HldSegmentAccess {
    static constexpr bool enabled=[] {
        if constexpr(requires {typename Range::HldSegmentTag;}) {
            using T=typename HldOwnedRangeTraits<Range>::Value;
            return sizeof(T)==16 && std::is_trivially_copyable_v<T>;
        } else return false;
    }();
};
}
#endif

#ifndef SIRKIT_HEAVY_LIGHT_VALUES_DEFINED
#define SIRKIT_HEAVY_LIGHT_VALUES_DEFINED
template<class Range,bool Ordered> class HeavyLightValues {
    using Traits=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldOwnedRangeTraits<Range>;
public:
    using value_type=typename Traits::Value;
private:
    struct State {
        HeavyLightDecomposition layout;
        Range forward;
        struct NoReverse {};
        [[no_unique_address]] std::conditional_t<Ordered,std::optional<Range>,NoReverse> backward;
        static Range make_range(const std::vector<value_type>& values) {
            if constexpr(std::constructible_from<Range,const std::vector<value_type>&>) return Range(values);
            else {
                Range range(static_cast<int>(values.size()));
                for(int i=0;i<static_cast<int>(values.size());++i) range.add(i,values[i]);
                return range;
            }
        }
        template<class Layout> State(Layout&& source,const std::vector<value_type>& ordered)
            :layout(std::forward<Layout>(source)),forward(make_range(ordered)) {
            if constexpr(Ordered) {
                auto reversed=ordered;std::reverse(reversed.begin(),reversed.end());backward.emplace(make_range(reversed));
            }
        }
    };
    std::unique_ptr<State> state_;
    static const HeavyLightDecomposition& empty_layout() {static const HeavyLightDecomposition h;return h;}
    const HeavyLightDecomposition& layout() const noexcept {return state_?state_->layout:empty_layout();}
    static value_type range_get(Range& range,int p) {
        if constexpr(requires {range.get(p);}) return range.get(p);
        else return range.sum(p,p+1);
    }
    static void range_set(Range& range,int p,const value_type& x) {
        if constexpr(requires {range.set(p,x);}) range.set(p,x);
        else range.add(p,/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldArithmetic<value_type>::subtract(x,range_get(range,p)));
    }
    void check_vertex(int v) const {(void)layout().index(v);}
    static value_type range_query(Range& range,int l,int r) {
        // 所有BITの一点区間は、二つの累積和を求めず直接読む。
        if constexpr(requires {range.sum(l,r);range.get(l);} && !requires {range.prod(l,r);})
            if(r-l==1) return range.get(l);
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::hld_query(range,l,r);
    }
    value_type query(int l,int r,bool reverse) {
        if constexpr(Ordered) if(reverse) return range_query(*state_->backward,size()-r,size()-l);
        return range_query(state_->forward,l,r);
    }
public:
    HeavyLightValues()=default;
    HeavyLightValues(const Graph& graph,const std::vector<value_type>& values,int root=0)
        :HeavyLightValues(HeavyLightDecomposition(graph,root),values) {}
    HeavyLightValues(const HeavyLightDecomposition& source,const std::vector<value_type>& values) {
        auto ordered=source.reorder(values);state_=std::make_unique<State>(source,ordered);
    }
    HeavyLightValues(HeavyLightDecomposition&& source,const std::vector<value_type>& values) {
        // valuesがsource.order()を参照する場合も、移動前に並べ替える。
        auto ordered=source.reorder(values);state_=std::make_unique<State>(std::move(source),ordered);
    }
    HeavyLightValues(const HeavyLightValues& other) {
        if(other.state_) state_=std::make_unique<State>(*other.state_);
    }
    HeavyLightValues(HeavyLightValues&&)=default;
    HeavyLightValues& operator=(HeavyLightValues other) noexcept {swap(other);return *this;}
    void swap(HeavyLightValues& other) noexcept {state_.swap(other.state_);}
    void clear() noexcept {state_.reset();}
    void build(const Graph& graph,const std::vector<value_type>& values,int root=0) {HeavyLightValues(graph,values,root).swap(*this);}
    int size() const noexcept {return layout().size();}
    bool empty() const noexcept {return layout().empty();}
    int root() const noexcept {return layout().root();}
    int component_count() const noexcept {return layout().component_count();}
    int parent(int v) const {return layout().parent(v);}
    int depth(int v) const {return layout().depth(v);}
    int component(int v) const {return layout().component(v);}
    int root_of(int v) const {return layout().root_of(v);}
    int index(int v) const {return layout().index(v);}
    int vertex_at(int i) const {return layout().vertex_at(i);}
    const std::vector<int>& order() const noexcept {return layout().order();}
    bool same_component(int u,int v) const {return layout().same_component(u,v);}
    bool is_ancestor(int u,int v) const {return layout().is_ancestor(u,v);}
    std::pair<int,int> subtree_range(int v,bool edges=false) const {return layout().subtree_range(v,edges);}
    int subtree_size(int v) const {return layout().subtree_size(v);}
    int lca(int u,int v) const {return layout().lca(u,v);}
    int distance(int u,int v) const {return layout().distance(u,v);}
    int kth_ancestor(int v,std::int64_t k) const {return layout().kth_ancestor(v,k);}
    int jump(int u,int v,std::int64_t k) const {return layout().jump(u,v,k);}
    template<class Fn> bool for_each_path(int u,int v,Fn&& fn,bool edges=false) const {return layout().for_each_path(u,v,std::forward<Fn>(fn),edges);}
    std::vector<HeavyLightDecomposition::Interval> path(int u,int v,bool edges=false) const {return layout().path(u,v,edges);}
    template<class T> std::vector<T> reorder(const std::vector<T>& values) const {return layout().reorder(values);}
    template<class T> std::vector<T> restore(const std::vector<T>& values) const {return layout().restore(values);}
    value_type get(int v) requires (requires(Range& r){r.get(0);} || requires(Range& r){r.sum(0,0);}) {
        const int p=index(v);return range_get(state_->forward,p);
    }
    void set(int v,const value_type x) requires (requires(Range& r){r.set(0,x);} || requires(Range& r){r.add(0,x-x);}) {
        const int p=index(v);range_set(state_->forward,p,x);
        if constexpr(Ordered) range_set(*state_->backward,size()-1-p,x);
    }
    void add(int v,const value_type x) requires requires(Range& r){r.add(0,x);} {
        const int p=index(v);state_->forward.add(p,x);
        if constexpr(Ordered) state_->backward->add(size()-1-p,x);
    }
    template<class F> void apply(int v,const F f) requires requires(Range& r){r.apply(0,f);} {
        const int p=index(v);state_->forward.apply(p,f);
        if constexpr(Ordered) state_->backward->apply(size()-1-p,f);
    }
    value_type path_prod(int u,int v,bool edges=false) {
        if constexpr(Ordered) {
            auto interval=[&](int l,int r,bool reverse){return query(l,r,reverse);};
            return layout().path_prod(u,v,Traits::identity(),interval,Traits::combine,edges);
        } else {
            auto interval=[&](int l,int r){return query(l,r,false);};
            return layout().path_prod(u,v,Traits::identity(),interval,Traits::combine,edges);
        }
    }
    value_type subtree_prod(int v,bool edges=false) {
        auto[l,r]=subtree_range(v,edges);return query(l,r,false);
    }
    template<class F> void path_apply(int u,int v,const F f,bool edges=false) requires requires(Range& r){r.apply(0,0,f);} {
        for_each_path(u,v,[&](int l,int r,bool) {
            state_->forward.apply(l,r,f);
            if constexpr(Ordered) state_->backward->apply(size()-r,size()-l,f);
        },edges);
    }
    template<class F> void subtree_apply(int v,const F f,bool edges=false) requires requires(Range& r){r.apply(0,0,f);} {
        auto[l,r]=subtree_range(v,edges);if(l==r) return;
        state_->forward.apply(l,r,f);
        if constexpr(Ordered) state_->backward->apply(size()-r,size()-l,f);
    }
    std::vector<value_type> values() {
        if(!state_) return {};
        if constexpr(requires {state_->forward.values();}) return restore(state_->forward.values());
        else {
            std::vector<value_type> ordered;ordered.reserve(static_cast<std::size_t>(size()));
            for(int i=0;i<size();++i) ordered.push_back(range_get(state_->forward,i));
            return restore(ordered);
        }
    }
};
template<class Range,bool Ordered,class T>
HeavyLightValues<Range,Ordered> HeavyLightDecomposition::with_values(const std::vector<T>& values) const & {return HeavyLightValues<Range,Ordered>(*this,values);}
template<class Range,bool Ordered,class T>
HeavyLightValues<Range,Ordered> HeavyLightDecomposition::with_values(const std::vector<T>& values) && {return HeavyLightValues<Range,Ordered>(std::move(*this),values);}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


// Library: DataStructure/Segment_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Fast/Source/DataStructure/Segment_Tree.cpp
// 参考（モノイドと区間積）: https://info.atcoder.jp/entry/algorithm_lectures/range_query_monoid_design
// API仕様: https://atcoder.github.io/ac-library/production/document_ja/segtree.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/segment_tree.md
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md
//
// API一覧
// 速度優先版: 内部の主要領域は破棄・解放しない。格納型のデストラクタの副作用を必要としないこと。
// 入力・容量・件数の前提と確保成功を利用側が保証する。例外時の状態復元は保証しない。
// [segtree<S, op, e>（ACL形式、sirkit内に定義）]
// segtree()/segtree(int n)/segtree(vector<S>): 空列/n個の単位元/列から構築。0<=n<=INT_MAX、O(n+1)時間・領域。
// S op(S,S)は結合的、S e()は左右の単位元。同じ呼び出し形式の関数オブジェクトも可。演算/コピーはO(1)。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()の半開区間。
// set(int p,S x): 点代入、O(log(n+1))。get(int p): 点のS値、O(1)。prod(int l,int r): 左からの区間積、O(log(n+1))。
// all_prod(): 全積のS値、O(1)。空区間・空列の積はe()。積は非可換でもよい。
// max_right<pred>(int l)/min_left<pred>(int r): bool pred(S)を使う境界探索。述語を第2引数に渡す形式も可。戻り値はint、O(log(n+1))。
// 探索はpred(e())==trueで、区間を延ばすと真から偽へ一度だけ変化すること。述語に副作用を持たせない。
// [追加機能]
// size()/empty(): intの要素数/空か、O(1)。transform(int p,f): 点をf(現在値)へ更新、O(log(n+1))。
// values(): 列のコピー、O(n+1)。assign(vector<S>)/reset(int n)/reset(): 列/n個/現在の長さの単位元へ再構築、O(旧n+n+1)。
// clear()/swap(other): 容量を保持して空へ/交換、O(旧n+1)/O(1)。コピーは独立、移動元は空。値の演算等が投げる場合は内容の強い保証なし。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FAST_BUFFER_DEFINED
#define SIRKIT_FAST_BUFFER_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <class T>
class FastBuffer {
    T* data_ = nullptr;
    std::size_t size_ = 0, capacity_ = 0;
    void grow() {
        const auto limit = max_size();
        reserve(capacity_ == 0 ? 1 : capacity_ > limit - capacity_ ? limit : capacity_ * 2);
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;
    using reference = T&;
    using const_reference = const T&;
    FastBuffer() noexcept = default;
    explicit FastBuffer(std::size_t count) {
        reserve(count);
        for (; size_ < count; ++size_) std::construct_at(data_ + size_);
    }
    FastBuffer(std::size_t count, const T& value) {
        reserve(count);
        for (; size_ < count; ++size_) std::construct_at(data_ + size_, value);
    }
    template <std::input_iterator Iterator>
    FastBuffer(Iterator first, Iterator last) {
        if (first == last) return;
        if constexpr (std::forward_iterator<Iterator>) reserve(static_cast<std::size_t>(std::distance(first, last)));
        for (; first != last; ++first) emplace_back(*first);
    }
    FastBuffer(const std::vector<T>& values) : FastBuffer(values.begin(), values.end()) {}
    FastBuffer(const FastBuffer& other) : FastBuffer(other.begin(), other.end()) {}
    FastBuffer& operator=(const FastBuffer& other) {
        if (this != &other) { FastBuffer next(other); swap(next); }
        return *this;
    }
    FastBuffer(FastBuffer&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)) {}
    FastBuffer& operator=(FastBuffer&& other) noexcept {
        if (this != &other) {
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0); capacity_ = std::exchange(other.capacity_, 0);
        }
        return *this;
    }
    // デストラクタは、格納値の破棄もdeallocateも行わない。
    ~FastBuffer() = default;
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }
    static constexpr std::size_t max_size() noexcept {
        return static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(T);
    }
    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }
    T* begin() noexcept { return data_; }
    const T* begin() const noexcept { return data_; }
    T* end() noexcept { return size_ == 0 ? data_ : data_ + size_; }
    const T* end() const noexcept { return size_ == 0 ? data_ : data_ + size_; }
    const T* cbegin() const noexcept { return begin(); }
    const T* cend() const noexcept { return end(); }
    T& operator[](std::size_t index) noexcept { return data_[index]; }
    const T& operator[](std::size_t index) const noexcept { return data_[index]; }
    T& front() noexcept { return data_[0]; }
    const T& front() const noexcept { return data_[0]; }
    T& back() noexcept { return data_[size_ - 1]; }
    const T& back() const noexcept { return data_[size_ - 1]; }
    void reserve(std::size_t count) {
        if (count <= capacity_) return;
        // allocator::allocateはT配列の領域を開始する。要素は使用時だけ構築する。
        T* next = std::allocator<T>{}.allocate(count);
        for (std::size_t i = 0; i < size_; ++i)
            std::construct_at(next + i, std::move_if_noexcept(data_[i]));
        data_ = next; capacity_ = count;
    }
    template <class... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            // 引数が自身の要素を参照していても、移動する前に新要素を作る。
            T value(std::forward<Args>(args)...);
            grow();
            T* added = std::construct_at(data_ + size_, std::move_if_noexcept(value));
            ++size_;
            return *added;
        }
        T* added = std::construct_at(data_ + size_, std::forward<Args>(args)...);
        ++size_;
        return *added;
    }
    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }
    void pop_back() noexcept { --size_; }
    void clear() noexcept { size_ = 0; }
    void resize(std::size_t count) {
        if (count < size_) { size_ = count; return; }
        reserve(count);
        for (; size_ < count; ++size_) std::construct_at(data_ + size_);
    }
    void resize(std::size_t count, const T& value) {
        if (count < size_) { size_ = count; return; }
        if (count > capacity_) { T copy(value); reserve(count); resize(count, copy); return; }
        for (; size_ < count; ++size_) std::construct_at(data_ + size_, value);
    }
    void assign(std::size_t count, const T& value) { FastBuffer next(count, value); swap(next); }
    template <std::input_iterator Iterator>
    void assign(Iterator first, Iterator last) { FastBuffer next(first, last); swap(next); }
    void swap(FastBuffer& other) noexcept {
        std::swap(data_, other.data_); std::swap(size_, other.size_); std::swap(capacity_, other.capacity_);
    }
    friend void swap(FastBuffer& a, FastBuffer& b) noexcept { a.swap(b); }
};
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <functional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <type_traits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_HLD_SEGMENT_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_SEGMENT_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldSegmentAccess; }
#endif

#ifndef SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
#define SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
namespace detail {
inline std::size_t CheckedSegmentLeafBase(std::size_t count, std::size_t limit, const char* error) {
    if (count == 0) return 1;
    if (count > std::bit_floor(limit)) throw std::length_error(error);
    return std::bit_ceil(count);
}

inline int CheckedSegmentArrayCount(std::size_t count, const char* error) {
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error(error);
    return static_cast<int>(count);
}
}
#endif

#ifndef SIRKIT_SEGMENT_TREE_DEFINED
#define SIRKIT_SEGMENT_TREE_DEFINED
template<class S, auto op, auto e>
class segtree {
    static_assert(std::is_invocable_r_v<S, decltype(op), S, S>, "segtree: op(S,S) must return S");
    static_assert(std::is_invocable_r_v<S, decltype(e)>, "segtree: e() must return S");
    using HldSegmentTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldSegmentAccess;
    S identity_;
    std::size_t n_ = 0, base_ = 1;
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FastBuffer<S> data_;
    static std::size_t leaf_base(std::size_t n) {
        return detail::CheckedSegmentLeafBase(n, std::vector<S>{}.max_size() / 2, "segtree: size");
    }
    static std::size_t checked_size(int n) {
        return static_cast<std::size_t>(n);
    }
    static int vector_size(const std::vector<S>& values) {
        return detail::CheckedSegmentArrayCount(values.size(), "segtree: int size overflow");
    }
    void check_index(int i) const {
        (void)i;
    }
    void check_range(int l, int r) const {
        (void)l; (void)r;
    }
    static unsigned aligned_node(unsigned value) {
        if (value == 0) std::unreachable();
        return value >> std::countr_zero(value);
    }
    void rebuild() {
        for (auto p = base_; p > 1;) {
            --p;
            data_[p] = std::invoke(op, data_[2 * p], data_[2 * p + 1]);
        }
    }
    void update_carried_ancestors(std::size_t p) {
        if (p <= 1) return;
        S value = std::as_const(data_)[p];
        while (p > 1) {
            const S sibling = std::as_const(data_)[p ^ 1];
            const S left = (p & 1) ? sibling : value;
            const S right = (p & 1) ? value : sibling;
            value = std::invoke(op, left, right);
            p >>= 1;
            data_[p] = std::as_const(value);
        }
    }
    void update_ancestors(std::size_t p) {
        if constexpr (std::is_trivially_copyable_v<S> && std::is_copy_constructible_v<S> &&
                      std::is_copy_assignable_v<S> && sizeof(S) <= 16 &&
                      std::is_invocable_r_v<S, decltype(op), const S&, const S&>) {
            update_carried_ancestors(p);
        } else {
            for (p >>= 1; p; p >>= 1) data_[p] = std::invoke(op, data_[2 * p], data_[2 * p + 1]);
        }
    }
public:
    segtree() : segtree(0) {}
    explicit segtree(int n)
        : identity_(std::invoke(e)), n_(checked_size(n)), base_(leaf_base(n_)),
          data_(n_ ? 2 * base_ : 0, identity_) {}
    explicit segtree(const std::vector<S>& values) : segtree(vector_size(values)) {
        if (!n_) return;
        std::copy(values.begin(), values.end(), data_.begin() + base_);
        rebuild();
    }
    segtree(const segtree&) = default;
    segtree& operator=(const segtree& other) {
        if (this != &other) { segtree replacement(other); swap(replacement); }
        return *this;
    }
    segtree(segtree&& other)
        : identity_(other.identity_), n_(other.n_), base_(other.base_), data_(std::move(other.data_)) {
        other.n_ = 0; other.base_ = 1; other.data_.clear();
    }
    segtree& operator=(segtree&& other) {
        if (this != &other) { segtree replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    int size() const noexcept { return static_cast<int>(n_); }
    bool empty() const noexcept { return n_ == 0; }
    S get(int i) const { check_index(i); return data_[base_ + static_cast<std::size_t>(i)]; }
    void set(int i, S value) {
        check_index(i);
        const auto p = base_ + static_cast<std::size_t>(i);
        data_[p] = std::move(value);
        update_ancestors(p);
    }
    template<class F> void transform(int i, F&& f) {
        check_index(i);
        const auto p = base_ + static_cast<std::size_t>(i);
        data_[p] = std::invoke(std::forward<F>(f), std::as_const(data_)[p]);
        update_ancestors(p);
    }
    S prod(int l, int r) const {
        check_range(l, r);
        if (l == r) return identity_;
        // n<=INT_MAXなので葉番号はunsignedに収まり、両端は常に正。
        unsigned first = static_cast<unsigned>(base_) + static_cast<unsigned>(l);
        unsigned last = static_cast<unsigned>(base_) + static_cast<unsigned>(r);
        auto step = first & (~first + 1);
        if (step > last - first) {
            // firstの整列幅より残りが短ければ、右端の下位bitを落としてfirstへ戻れる。
            S result = data_[aligned_node(last) - 1];
            last -= last & (~last + 1);
            while (first < last) {
                result = std::invoke(op, data_[aligned_node(last) - 1], result);
                last -= last & (~last + 1);
            }
            return result;
        }
        // 左から最大の整列ブロックを選ぶ。最初は節点をコピーし、単位元との積を省く。
        S left = data_[aligned_node(first)];
        first += step;
        if (first == last) return left;
        // 短い残区間では分割点の計算を省く。
        if (last - first <= 32) {
            while (first < last && (step = first & (~first + 1)) <= last - first) {
                left = std::invoke(op, left, data_[aligned_node(first)]);
                first += step;
            }
            if (first == last) return left;
            S right = data_[aligned_node(last) - 1];
            last -= last & (~last + 1);
            while (first < last) {
                right = std::invoke(op, data_[aligned_node(last) - 1], right);
                last -= last & (~last + 1);
            }
            return std::invoke(op, left, right);
        }
        // 左走査が止まる整列境界を先に求め、独立な左右の積を交互に進める。
        const unsigned stop = last & (0U - std::bit_floor((first - 1) ^ (last - 1)));
        if (stop == last) {
            while (first < stop) {
                left = std::invoke(op, left, data_[aligned_node(first)]);
                first += first & (~first + 1);
            }
            return left;
        }
        S right = data_[aligned_node(last) - 1];
        last -= last & (~last + 1);
        while (first < stop && last > stop) {
            left = std::invoke(op, left, data_[aligned_node(first)]);
            first += first & (~first + 1);
            right = std::invoke(op, data_[aligned_node(last) - 1], right);
            last -= last & (~last + 1);
        }
        while (first < stop) {
            left = std::invoke(op, left, data_[aligned_node(first)]);
            first += first & (~first + 1);
        }
        while (last > stop) {
            right = std::invoke(op, data_[aligned_node(last) - 1], right);
            last -= last & (~last + 1);
        }
        return std::invoke(op, left, right);
    }
    S all_prod() const { return n_ ? data_[1] : identity_; }
    template<bool (*Pred)(S)> int max_right(int l) const { return max_right(l, Pred); }
    template<class Pred> int max_right(int l, Pred pred) const {
        check_range(l, l);
        if (static_cast<std::size_t>(l) == n_) return size();
        S accumulated = identity_;
        auto node = base_ + static_cast<std::size_t>(l);
        while (true) {
            while (!(node & 1)) node >>= 1;
            S candidate = std::invoke(op, accumulated, data_[node]);
            if (!std::invoke(pred, candidate)) {
                while (node < base_) {
                    node *= 2;
                    S next = std::invoke(op, accumulated, data_[node]);
                    if (std::invoke(pred, next)) { accumulated = std::move(next); ++node; }
                }
                return static_cast<int>(std::min(n_, node - base_));
            }
            accumulated = std::move(candidate);
            ++node;
            if ((node & (node - 1)) == 0) return size();
        }
    }
    template<bool (*Pred)(S)> int min_left(int r) const { return min_left(r, Pred); }
    template<class Pred> int min_left(int r, Pred pred) const {
        check_range(r, r);
        if (r == 0) return 0;
        S accumulated = identity_;
        auto node = base_ + static_cast<std::size_t>(r);
        while (true) {
            --node;
            while (node > 1 && (node & 1)) node >>= 1;
            S candidate = std::invoke(op, data_[node], accumulated);
            if (!std::invoke(pred, candidate)) {
                while (node < base_) {
                    node = 2 * node + 1;
                    S next = std::invoke(op, data_[node], accumulated);
                    if (std::invoke(pred, next)) { accumulated = std::move(next); --node; }
                }
                return static_cast<int>(node + 1 - base_);
            }
            accumulated = std::move(candidate);
            if ((node & (node - 1)) == 0) return 0;
        }
    }
    std::vector<S> values() const {
        if (!n_) return {};
        return std::vector<S>(data_.begin() + base_, data_.begin() + base_ + n_);
    }
    void assign(const std::vector<S>& values) { segtree replacement(values); swap(replacement); }
    void reset(int n) { segtree replacement(n); swap(replacement); }
    void reset() { std::fill(data_.begin(), data_.end(), identity_); }
    void clear() noexcept { data_.clear(); n_ = 0; base_ = 1; }
    void swap(segtree& other) noexcept(std::is_nothrow_swappable_v<S>) {
        using std::swap;
        swap(identity_, other.identity_);
        swap(n_, other.n_); swap(base_, other.base_); data_.swap(other.data_);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif


#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::pair<int, int>> edges;
    edges.reserve(n - 1);
    std::vector<std::int64_t> weights(n);
    sirkit::Graph graph(n);
    for (int v = 1; v < n; ++v)
    {
        int parent{};
        verify_in >> parent >> weights[v];
        --parent;
        edges.emplace_back(parent, v);
        graph[parent].push_back(v);
        graph[v].push_back(parent);
    }
    sirkit::TreePathDifference<> paths(n, edges);
    int updates{};
    verify_in >> updates;
    for (int i = 0; i < updates; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        paths.add_edge_path(u - 1, v - 1, 1);
    }
    const auto increments = paths.edge_values();
    for (int v = 1; v < n; ++v)
    {
        weights[v] += increments[v - 1];
    }
    const sirkit::HeavyLightDecomposition hld(graph);
    constexpr auto minimum = [](std::int64_t a, std::int64_t b) { return std::min(a, b); };
    constexpr auto identity = [] { return std::numeric_limits<std::int64_t>::max(); };
    const sirkit::segtree<std::int64_t, minimum, identity> range(hld.reorder(weights));
    int questions{};
    verify_in >> questions;
    for (int i = 0; i < questions; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        verify_out << (u == v ? 0 : hld.path_prod(u - 1, v - 1, range, true)) << '\n';
    }
}
