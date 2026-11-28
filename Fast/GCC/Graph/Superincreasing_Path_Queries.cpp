// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Superincreasing_Path_Queries.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/superincreasing_path_queries.md
//
// API一覧
// SuperincreasingPathQueries<T>(n, edges): 辺iの長さが先行辺の長さの総和より大きい無向グラフを前処理。
//   SuperincreasingPathEdge<T>{from,to,value} は0始まりの端点と、長さとは別の集計値。
//   構築 O(n log(n+1)+m(log(m+1)+log(n+1))) 時間、O(n log(n+1)+m) 領域。
// path_sum(u,v): u-v最短路に含まれる value の和。path_sum_without_edge(u,v,id): 辺idだけ除外して同じ計算。
//   到達不能は std::nullopt。同じ頂点間は T{}。T{} は加法単位元。+,- が使え、加法は可換。
//   クエリ O(log(n+1)) 時間。自己辺と多重辺可。費用が 2^i のときは value=2^i mod P を使える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
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

#ifndef SIRKIT_MINIMUM_SPANNING_TREE_DEFINED
#define SIRKIT_MINIMUM_SPANNING_TREE_DEFINED

struct SpanningTreeEdge
{
    int from;
    int to;
    std::int64_t weight;
};

namespace detail
{
inline void SortSpanningEdgeWeights(std::vector<std::uint64_t> &keys, int id_bits, int weight_bits)
{
    constexpr unsigned kDigitBits = 8;
    constexpr std::size_t kBuckets = 1U << kDigitBits;
    constexpr auto kDigitMask = kBuckets - 1;
    std::vector<std::uint64_t> temporary(keys.size());
    for (int offset = 0; offset < weight_bits; offset += kDigitBits)
    {
        const int shift = offset + id_bits;
        std::array<std::size_t, kBuckets> count{};
        for (auto key : keys)
        {
            ++count[(key >> shift) & kDigitMask];
        }
        std::size_t seen = 0;
        for (auto &bucket : count)
        {
            const auto frequency = bucket;
            bucket = seen;
            seen += frequency;
        }
        for (auto key : keys)
        {
            temporary[count[(key >> shift) & kDigitMask]++] = key;
        }
        keys.swap(temporary);
    }
}
} // namespace detail

inline std::vector<std::size_t> minimum_spanning_forest(int n, std::span<const SpanningTreeEdge> edges)
{
    if (edges.empty())
    {
        return {};
    }

    constexpr std::size_t kRadixThreshold = 1024;
    const bool try_radix = edges.size() >= kRadixThreshold;
    auto minimum = edges.front().weight, maximum = minimum;
    if (try_radix)
    {
        for (const auto &[from, to, weight] : edges)
        {
            minimum = std::min(minimum, weight);
            maximum = std::max(maximum, weight);
        }
    }

    std::vector<std::size_t> result;
    if (n < 2)
    {
        return result;
    }
    std::vector<std::uint64_t> order(edges.size());
    auto id_mask = std::numeric_limits<std::uint64_t>::max();
    const auto unsigned_min = static_cast<std::uint64_t>(minimum);
    const auto weight_range = static_cast<std::uint64_t>(maximum) - unsigned_min;
    const int id_bits = std::bit_width(edges.size() - 1);
    const int weight_bits = std::bit_width(weight_range);
    if (try_radix && weight_bits <= 64 - id_bits)
    {
        if (weight_range == 0)
        {
            std::iota(order.begin(), order.end(), std::uint64_t{0});
        }
        else
        {
            // 下位ビットに辺番号を載せ、上位の重みだけを安定に並べる。
            id_mask = (std::uint64_t{1} << id_bits) - 1;
            for (std::size_t i = 0; i < order.size(); ++i)
            {
                order[i] = ((static_cast<std::uint64_t>(edges[i].weight) - unsigned_min) << id_bits) | i;
            }
            detail::SortSpanningEdgeWeights(order, id_bits, weight_bits);
        }
    }
    else
    {
        std::iota(order.begin(), order.end(), std::uint64_t{0});
        std::sort(order.begin(), order.end(), [&](auto a, auto b)
                  {
                      const auto left = edges[static_cast<std::size_t>(a)].weight;
                      const auto right = edges[static_cast<std::size_t>(b)].weight;
                      return left == right ? a < b : left < right;
                  });
    }

    std::vector<int> components(n, -1);
    const auto root = [&components](int vertex)
    {
        while (components[vertex] >= 0)
        {
            const int parent = components[vertex];
            if (components[parent] < 0)
            {
                return parent;
            }
            vertex = components[vertex] = components[parent];
        }
        return vertex;
    };
    const auto limit = static_cast<std::size_t>(n - 1);
    result.reserve(std::min(limit, edges.size()));
    for (auto key : order)
    {
        const auto id = static_cast<std::size_t>(key & id_mask);
        const auto &edge = edges[id];
        int a = root(edge.from), b = root(edge.to);
        if (a == b)
        {
            continue;
        }
        if (components[a] > components[b])
        {
            std::swap(a, b);
        }
        components[a] += components[b];
        components[b] = a;
        result.push_back(id);
        if (result.size() == limit)
        {
            break;
        }
    }
    return result;
}

#endif

#ifndef SIRKIT_MST_SENSITIVITY_DEFINED
#define SIRKIT_MST_SENSITIVITY_DEFINED

struct SpanningForestSensitivity
{
    std::vector<std::size_t> forest;
    std::vector<std::optional<std::size_t>> exchange;
};

namespace detail
{
class SpanningForestPathMaximum
{
public:
    SpanningForestPathMaximum(int n, std::span<const SpanningTreeEdge> edges,
                              const std::vector<std::size_t> &ids)
        : levels_(std::bit_width(static_cast<unsigned int>(n))), edges_(edges),
          depth_(n, -1), parent_edge_(n), jumps_(static_cast<std::size_t>(n) * levels_)
    {
        std::vector<std::vector<std::pair<int, std::size_t>>> adjacency(n);
        for (const auto id : ids)
        {
            const auto &[from, to, weight] = edges[id];
            adjacency[from].emplace_back(to, id);
            adjacency[to].emplace_back(from, id);
        }
        std::vector<int> order;
        order.reserve(n);
        for (int root = 0; root < n; ++root)
        {
            if (depth_[root] != -1)
            {
                continue;
            }
            depth_[root] = 0;
            std::fill_n(jumps_.begin() + offset(root), levels_, Jump{root, -1});
            auto cursor = order.size();
            order.push_back(root);
            for (; cursor < order.size(); ++cursor)
            {
                const int v = order[cursor];
                for (const auto &[u, edge] : adjacency[v])
                {
                    if (depth_[u] == -1)
                    {
                        depth_[u] = depth_[v] + 1;
                        parent_edge_[u] = edge;
                        auto *row = jumps_.data() + offset(u);
                        row[0] = {v, u};
                        for (unsigned int level = 1; level < levels_; ++level)
                        {
                            const auto low = row[level - 1];
                            const auto high = jump(low.ancestor, level - 1);
                            row[level] = {high.ancestor, maximum(low.heaviest_child, high.heaviest_child)};
                        }
                        order.push_back(u);
                    }
                }
            }
        }
    }

    [[nodiscard]] int depth(int v) const noexcept { return depth_[v]; }
    [[nodiscard]] int parent(int v) const noexcept { return jump(v, 0).ancestor; }
    [[nodiscard]] std::size_t parent_edge(int v) const noexcept { return parent_edge_[v]; }

    std::pair<int, int> query(int u, int v) const noexcept
    {
        int maximum_child = -1;
        if (depth_[u] < depth_[v])
        {
            std::swap(u, v);
        }
        auto difference = static_cast<unsigned int>(depth_[u] - depth_[v]);
        for (; difference != 0; difference &= difference - 1)
        {
            const auto up = jump(u, static_cast<unsigned int>(std::countr_zero(difference)));
            maximum_child = maximum(maximum_child, up.heaviest_child);
            u = up.ancestor;
        }
        if (u == v)
        {
            return {u, maximum_child};
        }
        for (auto level = levels_; level != 0; )
        {
            --level;
            const auto a = jump(u, level), b = jump(v, level);
            if (a.ancestor != b.ancestor)
            {
                maximum_child = maximum(maximum_child, maximum(a.heaviest_child, b.heaviest_child));
                u = a.ancestor;
                v = b.ancestor;
            }
        }
        return {parent(u), maximum(maximum_child, maximum(u, v))};
    }

private:
    struct Jump
    {
        int ancestor;
        int heaviest_child;
    };

    [[nodiscard]] std::size_t offset(int v) const noexcept
    {
        return static_cast<std::size_t>(v) * levels_;
    }

    [[nodiscard]] Jump jump(int v, unsigned int level) const noexcept
    {
        return jumps_[offset(v) + level];
    }

    [[nodiscard]] int maximum(int a, int b) const noexcept
    {
        if (a == -1)
        {
            return b;
        }
        if (b == -1)
        {
            return a;
        }
        const auto left = parent_edge_[a], right = parent_edge_[b];
        const auto wa = edges_[left].weight, wb = edges_[right].weight;
        return (wa == wb ? left < right : wa > wb) ? a : b;
    }

    unsigned int levels_;
    std::span<const SpanningTreeEdge> edges_;
    std::vector<int> depth_;
    std::vector<std::size_t> parent_edge_;
    std::vector<Jump> jumps_;
};

class SpanningForestUnassigned
{
public:
    explicit SpanningForestUnassigned(int n) : groups_(n, -1), next_(n)
    {
        std::iota(next_.begin(), next_.end(), 0);
    }

    int find(int v)
    {
        return next_[leader(v)];
    }

    int consume(int v, int parent)
    {
        int a = leader(v), b = leader(parent);
        const int next = next_[b];
        if (groups_[a] > groups_[b])
        {
            std::swap(a, b);
        }
        groups_[a] += groups_[b];
        groups_[b] = a;
        next_[a] = next;
        return next;
    }

private:
    int leader(int v)
    {
        auto root = v;
        while (groups_[root] >= 0)
        {
            root = groups_[root];
        }
        while (v != root)
        {
            const int parent = groups_[v];
            groups_[v] = root;
            v = parent;
        }
        return root;
    }

    std::vector<int> groups_, next_;
};
}

inline SpanningForestSensitivity minimum_spanning_forest_sensitivity(
    int n, std::span<const SpanningTreeEdge> edges)
{
    if (n < 0)
    {
        throw std::length_error("minimum_spanning_forest_sensitivity: negative vertex count");
    }
    SpanningForestSensitivity result{minimum_spanning_forest(n, edges),
                                     std::vector<std::optional<std::size_t>>(edges.size())};
    std::vector<std::uint8_t> in_tree(edges.size());
    for (const auto id : result.forest)
    {
        in_tree[id] = 1;
    }
    std::vector<std::size_t> candidates;
    candidates.reserve(edges.size() - result.forest.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (!in_tree[i] && edges[i].from != edges[i].to)
        {
            candidates.push_back(i);
        }
    }
    if (candidates.empty())
    {
        return result;
    }
    std::sort(candidates.begin(), candidates.end(), [&](std::size_t a, std::size_t b)
              {
                  const auto left = edges[a].weight, right = edges[b].weight;
                  return left == right ? a < b : left < right;
              });
    const detail::SpanningForestPathMaximum tree(n, edges, result.forest);
    detail::SpanningForestUnassigned unassigned(n);
    for (const auto id : candidates)
    {
        const auto &[u, v, weight] = edges[id];
        const auto [ancestor, heaviest] = tree.query(u, v);
        result.exchange[id] = tree.parent_edge(heaviest);
        // 最初に辺を通る非木辺が最小の代替辺。割り当てた木辺は次の照会から飛ばす。
        for (const int endpoint : {u, v})
        {
            int x = unassigned.find(endpoint);
            while (tree.depth(x) > tree.depth(ancestor))
            {
                result.exchange[tree.parent_edge(x)] = id;
                x = unassigned.consume(x, tree.parent(x));
            }
        }
    }
    return result;
}

#endif

#ifndef SIRKIT_SUPERINCREASING_PATH_QUERIES_DEFINED
#define SIRKIT_SUPERINCREASING_PATH_QUERIES_DEFINED

template <class T>
struct SuperincreasingPathEdge
{
    int from, to;
    T value;
};

template <class T>
class SuperincreasingPathQueries
{
public:
    SuperincreasingPathQueries(int n, std::span<const SuperincreasingPathEdge<T>> edges)
        : edges_(edges.begin(), edges.end()), child_(edges.size(), -1)
    {
        if (n < 0)
        {
            throw std::length_error("SuperincreasingPathQueries: negative vertex count");
        }
        std::vector<SpanningTreeEdge> ranked;
        ranked.reserve(edges.size());
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto &edge = edges[i];
            if (edge.from < 0 || edge.from >= n || edge.to < 0 || edge.to >= n)
            {
                throw std::out_of_range("SuperincreasingPathQueries: edge endpoint");
            }
            ranked.push_back({edge.from, edge.to, static_cast<std::int64_t>(i)});
        }
        auto sensitivity = minimum_spanning_forest_sensitivity(n, ranked);
        std::vector<std::pair<int, int>> forest;
        forest.reserve(sensitivity.forest.size());
        for (const auto id : sensitivity.forest)
        {
            const auto &edge = edges[id];
            forest.emplace_back(edge.from, edge.to);
        }
        lca_.build(n, forest);

        prefix_.resize(n);
        for (const auto id : sensitivity.forest)
        {
            const auto &edge = edges[id];
            const int child = lca_.parent(edge.from) == edge.to ? edge.from : edge.to;
            child_[id] = child;
            prefix_[child] = edge.value;
        }
        std::vector<int> preorder(n);
        for (int v = 0; v < n; ++v)
        {
            preorder[lca_.order_index(v)] = v;
        }
        for (const int vertex : preorder)
        {
            const int parent = lca_.parent(vertex);
            if (parent != -1)
            {
                prefix_[vertex] = prefix_[parent] + prefix_[vertex];
            }
        }
        exchanges_ = std::move(sensitivity.exchange);
    }

    [[nodiscard]] std::optional<T> path_sum(int from, int to) const
    {
        CheckVertex(from);
        CheckVertex(to);
        const int ancestor = lca_(from, to);
        if (ancestor == -1)
        {
            return std::nullopt;
        }
        return PathSum(from, to, ancestor);
    }

    [[nodiscard]] std::optional<T> path_sum_without_edge(int from, int to, std::size_t removed) const
    {
        if (removed >= edges_.size())
        {
            throw std::out_of_range("SuperincreasingPathQueries: edge id");
        }
        CheckVertex(from);
        CheckVertex(to);
        const int ancestor = lca_(from, to);
        if (ancestor == -1)
        {
            return std::nullopt;
        }
        const int child = child_[removed];
        if (child == -1)
        {
            return PathSum(from, to, ancestor);
        }
        const bool from_subtree = lca_.is_ancestor(child, from);
        if (from_subtree == lca_.is_ancestor(child, to))
        {
            return PathSum(from, to, ancestor);
        }
        if (!exchanges_[removed])
        {
            return std::nullopt;
        }

        const auto &bridge = edges_[*exchanges_[removed]];
        int start = bridge.from, finish = bridge.to;
        if (from_subtree != lca_.is_ancestor(child, start))
        {
            std::swap(start, finish);
        }
        return PathSum(from, start, lca_(from, start)) + bridge.value +
               PathSum(finish, to, lca_(finish, to));
    }

private:
    std::vector<SuperincreasingPathEdge<T>> edges_;
    std::vector<int> child_;
    std::vector<std::optional<std::size_t>> exchanges_;
    LCA lca_;
    std::vector<T> prefix_;

    void CheckVertex(int vertex) const
    {
        if (static_cast<std::size_t>(vertex) >= lca_.size())
        {
            throw std::out_of_range("SuperincreasingPathQueries: vertex");
        }
    }

    T PathSum(int from, int to, int ancestor) const
    {
        return (prefix_[from] - prefix_[ancestor]) + (prefix_[to] - prefix_[ancestor]);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
