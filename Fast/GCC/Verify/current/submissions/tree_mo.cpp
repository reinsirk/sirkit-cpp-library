#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc477/tasks/abc477_g

// Library: Graph/Tree_Mo.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Mo.cpp
// 参考（木上のMo法）: https://atcoder.jp/contests/abc477/editorial/25886?lang=ja
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_mo.md
//
// API一覧
// TreeMo(n,edges,root=0): 頂点0..n-1の無向木。各辺を入力順に1回ずつ渡す。
// size(): 頂点数、O(1)。構築時間・領域はB=ceil(n/32)としてO(n+B log(B+1))。
// run(queries,add,erase,answer,block_size=0): (始点,終点)列の各パス上の頂点をMo順に集計。
// run_edges(...): 辺を集計。addとeraseの引数は入力順の辺番号。頂点版では頂点番号。
// answer(照会番号)は集計がパスに一致したとき呼ぶ。外部集計は開始時に空、正常終了時も空。
// 各パス上の順序は保持しない。n頂点q照会で、幅bなら入れ替えO(qb+n(n/b+1)+q)回。
// ほかに照会の整列O(q log(q+1))時間と作業領域O(n+q)。幅0で自動。
// callback中に本体を変更しない。callbackの例外では外部集計を巻き戻さない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
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

#ifndef SIRKIT_MO_ORDERING_HELPERS_DEFINED
#define SIRKIT_MO_ORDERING_HELPERS_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
struct MoOrderEntry { std::size_t id; std::int32_t left, right; };
struct TimedMoOrderEntry { std::size_t id, time; std::int32_t left, right; };
inline std::uint64_t mo_integer_sqrt(std::uint64_t value) noexcept {
    std::uint64_t low = 0, high = std::min<std::uint64_t>(value, 0xFFFFFFFFu);
    while (low < high) {
        const auto distance = high - low, middle = low + distance / 2 + distance % 2;
        if (middle <= value / middle) low = middle; else high = middle - 1;
    }
    return low;
}
inline void mo_check_width(std::int32_t requested) {
    (void)requested;
}
// 左端の往復QB/3と右端の掃引N^2/(2B)の見積りを基に、実測で係数2を選んだ。
inline std::int32_t mo_static_width(std::int32_t n, std::size_t queries, std::int32_t requested) {
    mo_check_width(requested);
    if (requested > 0) return requested;
    if (n <= 0) return 1;
    // int32_tの正の長さの二乗の2倍はuint64_tに収まる。
    const auto square = static_cast<std::uint64_t>(n) * static_cast<std::uint64_t>(n);
    const auto width = mo_integer_sqrt(2 * square / std::max<std::uint64_t>(1, queries));
    return static_cast<std::int32_t>(std::clamp<std::uint64_t>(width, 1, static_cast<std::uint64_t>(n)));
}
// 伸縮(2/3)QB+N^2/(2B)と時刻UK^2/2の見積りで、時刻1回を伸縮2回分として和を最小にする。
// 導関数が0以上になる最小のBを二分探索する。比較の丸め誤差は幅の選択だけに影響する。
inline std::int32_t mo_timed_width(std::int32_t n, std::size_t queries, std::size_t updates, std::int32_t requested) {
    mo_check_width(requested);
    if (requested > 0) return requested;
    if (n <= 0) return 1;
    const double count = static_cast<double>(std::max<std::size_t>(1, queries)), square = static_cast<double>(n) * static_cast<double>(n);
    const double time_cost = 12.0 * static_cast<double>(updates) * square;
    std::int32_t low = 1, high = n;
    while (low < high) {
        const auto middle = low + (high - low) / 2;
        const auto width = static_cast<double>(middle);
        if (4.0 * count * width * width * width >= 3.0 * square * width + time_cost) high = middle; else low = middle + 1;
    }
    return low;
}
inline void mo_check_range(std::int32_t n, std::int32_t left, std::int32_t right) {
    (void)n; (void)left; (void)right;
}
// 下位id_bitsに追加順のIDを持つ相異なる鍵を昇順に並べる。
// 大きい入力は上位の比較鍵だけを安定な基数ソートで並べ、同じ比較鍵の要求はID順のまま残す。
inline void mo_sort_keys(std::vector<std::uint64_t>& keys, unsigned id_bits) {
    std::uint64_t used = 0;
    for (const auto key : keys) used |= key;
    const auto high = static_cast<unsigned>(std::bit_width(used));
    if (high <= id_bits) return;
    if (keys.size() < 1024) { std::sort(keys.begin(), keys.end()); return; }
    // 桁の種類数を要求数程度に抑え、各桁をO(Q)で処理する。
    const auto widest = std::clamp(static_cast<unsigned>(std::bit_width(keys.size())), 8u, 16u);
    const unsigned span = high - id_bits, passes = (span + widest - 1) / widest, digit = (span + passes - 1) / passes;
    const auto mask = (std::uint64_t{1} << digit) - 1;
    std::vector<std::uint64_t> buffer(keys.size());
    std::vector<std::size_t> start(std::size_t{1} << digit);
    for (unsigned pass = 0; pass < passes; ++pass) {
        const unsigned shift = id_bits + pass * digit;
        std::fill(start.begin(), start.end(), std::size_t{0});
        for (const auto key : keys) ++start[(key >> shift) & mask];
        if (start[(keys.front() >> shift) & mask] == keys.size()) continue;
        std::size_t sum = 0;
        for (auto& count : start) { const auto value = count; count = sum; sum += value; }
        for (const auto key : keys) buffer[start[(key >> shift) & mask]++] = key;
        keys.swap(buffer);
    }
}
}
#endif

#ifndef SIRKIT_MO_DEFINED
#define SIRKIT_MO_DEFINED
class Mo {
public:
    struct Query { std::int32_t left, right; bool operator==(const Query&) const = default; };
    struct Window { std::int32_t left, right; bool operator==(const Window&) const = default; };
private:
    std::int32_t length_ = 0;
    std::vector<Query> queries_;
    // left block→right→IDの順にemit(id)を呼ぶ。snakeなら奇数blockのrightを降順にする。
    template<class Emit>
    void sorted_ids(std::int32_t width, bool snake, Emit&& emit) const {
        const auto count = queries_.size();
        if (count == 0) return;
        const auto divisor = static_cast<std::uint32_t>(width);
        const auto block_bits = static_cast<unsigned>(std::bit_width(static_cast<std::uint32_t>(length_) / divisor));
        const auto right_bits = static_cast<unsigned>(std::bit_width(static_cast<std::uint32_t>(length_)));
        const auto id_bits = static_cast<unsigned>(std::bit_width(count - 1));
        if (block_bits + right_bits + id_bits <= 64) {
            const auto right_max = (std::uint64_t{1} << right_bits) - 1;
            std::vector<std::uint64_t> keys(count);
            for (std::size_t i = 0; i < count; ++i) {
                const auto block = static_cast<std::uint64_t>(static_cast<std::uint32_t>(queries_[i].left) / divisor);
                const auto right = static_cast<std::uint64_t>(queries_[i].right);
                keys[i] = (((block << right_bits) | (snake && (block & 1) ? right_max - right : right)) << id_bits) | i;
            }
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_sort_keys(keys, id_bits);
            const auto id_mask = (std::uint64_t{1} << id_bits) - 1;
            for (const auto key : keys) emit(static_cast<std::size_t>(key & id_mask));
            return;
        }
        // 長さが大きく幅が小さいと鍵が64bitを超えるため、比較ソートで同じ順に並べる。
        std::vector<std::size_t> ids(count);
        for (std::size_t i = 0; i < count; ++i) ids[i] = i;
        std::sort(ids.begin(), ids.end(), [&](std::size_t a, std::size_t b) {
            const auto ab = queries_[a].left / width, bb = queries_[b].left / width;
            if (ab != bb) return ab < bb;
            const auto ar = queries_[a].right, br = queries_[b].right;
            if (ar != br) return snake && (ab & 1) ? ar > br : ar < br;
            return a < b;
        });
        for (const auto id : ids) emit(id);
    }
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MoOrderEntry> ordered_entries(std::int32_t width, bool snake) const {
        std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MoOrderEntry> result; result.reserve(queries_.size());
        sorted_ids(width, snake, [&](std::size_t id) { result.push_back({id, queries_[id].left, queries_[id].right}); });
        return result;
    }
public:
    explicit Mo(std::int32_t n = 0) : length_(n) {
    }
    Mo(const Mo&) = default;
    Mo& operator=(const Mo& other) { if (this != &other) { Mo copy(other); swap(copy); } return *this; }
    Mo(Mo&& other) noexcept { swap(other); }
    Mo& operator=(Mo&& other) noexcept { if (this != &other) { clear(); swap(other); } return *this; }
    void swap(Mo& other) noexcept { std::swap(length_, other.length_); queries_.swap(other.queries_); }
    friend void swap(Mo& a, Mo& b) noexcept { a.swap(b); }
    std::int32_t length() const noexcept { return length_; }
    std::size_t query_count() const noexcept { return queries_.size(); }
    bool empty() const noexcept { return queries_.empty(); }
    std::span<const Query> queries() const noexcept { return queries_; }
    Query query(std::size_t id) const {
        return queries_[id];
    }
    std::size_t add_query(std::int32_t left, std::int32_t right) {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_check_range(length_, left, right); const auto id = queries_.size(); queries_.push_back({left, right}); return id;
    }
    std::int32_t block_size(std::int32_t requested = 0) const { return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::mo_static_width(length_, queries_.size(), requested); }
    std::vector<std::size_t> order(std::int32_t requested = 0) const {
        const auto width = block_size(requested); std::vector<std::size_t> result; result.reserve(queries_.size());
        sorted_ids(width, true, [&](std::size_t id) { result.push_back(id); });
        return result;
    }
    template<class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Answer>
    Window run(AddLeft&& add_left, AddRight&& add_right, EraseLeft&& erase_left, EraseRight&& erase_right, Answer&& answer, std::int32_t requested = 0) const {
        const auto entries = ordered_entries(block_size(requested), true); if (entries.empty()) return {0, 0};
        auto left = entries.front().left, right = left;
        // callbackの書き込みと別名にならないよう、要求は値で読む。
        for (const auto entry : entries) {
            while (left > entry.left) std::invoke(add_left, --left);
            while (right < entry.right) { std::invoke(add_right, right); ++right; }
            while (left < entry.left) { std::invoke(erase_left, left); ++left; }
            while (right > entry.right) std::invoke(erase_right, --right);
            std::invoke(answer, entry.id);
        }
        return {left, right};
    }
    template<class Add, class Erase, class Answer>
    Window run(Add&& add, Erase&& erase, Answer&& answer, std::int32_t requested = 0) const { return run(add, add, erase, erase, answer, requested); }
    template<class AddLeft, class AddRight, class Snapshot, class Rollback, class Reset, class Answer>
    requires std::is_invocable_v<Answer&, std::size_t>
    void run_rollback(AddLeft&& add_left, AddRight&& add_right, Snapshot&& snapshot, Rollback&& rollback, Reset&& reset, Answer&& answer, std::int32_t requested = 0) const {
        const auto width = block_size(requested); const auto entries = ordered_entries(width, false);
        for (std::size_t i = 0; i < entries.size();) {
            // 同じblockの要求はright昇順に並ぶ。block内に収まる要求が先に来るので、そのとき集計は空。
            const auto block = entries[i].left / width;
            const auto end = static_cast<std::int32_t>(std::min<std::int64_t>(length_, (std::int64_t{block} + 1) * width));
            auto right = end;
            for (; i < entries.size() && entries[i].left / width == block; ++i) {
                const auto entry = entries[i];
                if (entry.right <= end) {
                    std::invoke(snapshot);
                    for (auto index = entry.left; index < entry.right; ++index) std::invoke(add_right, index);
                } else {
                    while (right < entry.right) { std::invoke(add_right, right); ++right; }
                    std::invoke(snapshot);
                    for (auto index = end; index > entry.left;) std::invoke(add_left, --index);
                }
                std::invoke(answer, entry.id);
                std::invoke(rollback);
            }
            if (right > end) std::invoke(reset);
        }
    }
    template<class Add, class Snapshot, class Rollback, class Reset, class Answer>
    void run_rollback(Add&& add, Snapshot&& snapshot, Rollback&& rollback, Reset&& reset, Answer&& answer, std::int32_t requested = 0) const {
        run_rollback(add, add, snapshot, rollback, reset, answer, requested);
    }
    void reserve_queries(std::size_t capacity) { queries_.reserve(capacity); }
    void clear_requests() noexcept { queries_.clear(); }
    void reset(std::int32_t n = 0) { Mo next(n); swap(next); }
    void clear() noexcept { queries_.clear(); length_ = 0; }
};
#endif

#ifndef SIRKIT_TREE_MO_DEFINED
#define SIRKIT_TREE_MO_DEFINED

class TreeMo
{
public:
    TreeMo(int n, std::span<const std::pair<int, int>> edges, int root = 0)
        : lca_(checked_size(n), edges, root), entry_(n), edge_to_parent_(n)
    {
        if (n == 0)
        {
            return;
        }
        std::vector<std::vector<int>> children(n);
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            auto [parent, child] = edges[i];
            if (lca_.parent(child) != parent)
            {
                std::swap(parent, child);
            }
            edge_to_parent_[child] = i;
            children[parent].push_back(child);
        }
        struct Frame
        {
            int vertex;
            std::size_t next;
        };
        std::vector<Frame> stack;
        stack.reserve(n);
        tour_.reserve(2 * static_cast<std::size_t>(n));
        visible_tour_size_ = 1;
        tour_.push_back(root);
        stack.push_back({root, 0});
        while (!stack.empty())
        {
            auto &frame = stack.back();
            const int vertex = frame.vertex;
            if (frame.next == children[vertex].size())
            {
                tour_.push_back(vertex);
                stack.pop_back();
            }
            else
            {
                const int child = children[vertex][frame.next++];
                entry_[child] = static_cast<std::int32_t>(tour_.size());
                visible_tour_size_ = entry_[child] + 1;
                tour_.push_back(child);
                stack.push_back({child, 0});
            }
        }
    }

    TreeMo(const TreeMo &) = default;
    TreeMo(TreeMo &&) noexcept = default;

    TreeMo &operator=(TreeMo other) noexcept
    {
        lca_.swap(other.lca_);
        entry_.swap(other.entry_);
        edge_to_parent_.swap(other.edge_to_parent_);
        tour_.swap(other.tour_);
        std::swap(visible_tour_size_, other.visible_tour_size_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lca_.size();
    }

    template<class Add, class Erase, class Answer>
    void run(std::span<const std::pair<int, int>> queries, Add &&add, Erase &&erase,
             Answer &&answer, std::int32_t block_size = 0) const
    {
        run_impl<false>(queries, add, erase, answer, block_size);
    }

    template<class Add, class Erase, class Answer>
    void run_edges(std::span<const std::pair<int, int>> queries, Add &&add, Erase &&erase,
                   Answer &&answer, std::int32_t block_size = 0) const
    {
        run_impl<true>(queries, add, erase, answer, block_size);
    }

private:
    static int checked_size(int n)
    {
        if (n < 0 || n > std::numeric_limits<std::int32_t>::max() / 2)
        {
            throw std::length_error("TreeMo: tour length out of range");
        }
        return n;
    }

    template<bool Edges, class Add, class Erase, class Answer>
    void run_impl(std::span<const std::pair<int, int>> queries, Add &add, Erase &erase,
                  Answer &answer, std::int32_t block_size) const
    {
        Mo windows(visible_tour_size_);
        windows.reserve_queries(queries.size());
        std::vector<int> correction;
        if constexpr (!Edges)
        {
            correction.reserve(queries.size());
        }
        for (const auto &[u, v] : queries)
        {
            // 頂点の出現回数を奇数で判定すると、2つの根からの経路の対称差が得られる。
            if constexpr (!Edges)
            {
                correction.push_back(lca_.get(u, v));
            }
            else
            {
            }
            const auto [left, right] = std::minmax(entry_[u], entry_[v]);
            windows.add_query(left + 1, right + 1);
        }
        std::vector<std::uint8_t> active(size());
        const auto id = [&](int v)
        {
            if constexpr (Edges)
            {
                return edge_to_parent_[v];
            }
            else
            {
                return v;
            }
        };
        const auto toggle = [&](std::int32_t index)
        {
            const int vertex = tour_[index];
            auto &present = active[vertex];
            present ^= 1;
            if (present)
            {
                std::invoke(add, id(vertex));
            }
            else
            {
                std::invoke(erase, id(vertex));
            }
        };
        const auto visit = [&](std::size_t query)
        {
            if constexpr (!Edges)
            {
                std::invoke(add, correction[query]);
            }
            std::invoke(answer, query);
            if constexpr (!Edges)
            {
                std::invoke(erase, correction[query]);
            }
        };
        windows.run(toggle, toggle, visit, block_size);
        for (std::size_t v = 0; v < active.size(); ++v)
        {
            if (active[v])
            {
                std::invoke(erase, id(static_cast<int>(v)));
            }
        }
    }

    LCA lca_;
    std::vector<std::int32_t> entry_;
    std::vector<std::size_t> edge_to_parent_;
    std::vector<int> tour_;
    std::int32_t visible_tour_size_ = 0;
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


#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<int> label(n);
    for (auto &value : label)
    {
        verify_in >> value;
    }
    std::vector<std::pair<int, int>> edges(n - 1), queries(q), bounds(q);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    for (int i = 0; i < q; ++i)
    {
        auto &[u, v] = queries[i];
        auto &[lo, hi] = bounds[i];
        verify_in >> u >> v >> lo >> hi;
        --u;
        --v;
    }

    const sirkit::TreeMo tree(n, edges);
    std::vector<int> frequency(n + 1), at_least(n + 2), answers(q);
    const auto add = [&](int v)
    {
        ++at_least[++frequency[label[v]]];
    };
    const auto erase = [&](int v)
    {
        --at_least[frequency[label[v]]--];
    };
    tree.run(queries, add, erase, [&](std::size_t query)
             {
                 const auto [lo, hi] = bounds[query];
                 answers[query] = at_least[lo] - at_least[hi + 1];
             });
    for (const int answer : answers)
    {
        verify_out << answer << '\n';
    }
}
