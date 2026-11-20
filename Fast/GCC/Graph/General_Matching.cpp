// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/General_Matching.cpp
// 数学的参考: https://doi.org/10.4153/CJM-1965-045-4
// 参考: https://ei1333.github.io/library/graph/flow/gabow-edmonds.hpp.html
// 奇数閉路の縮約という数学的構成を参考に、単純な反復Edmonds探索を独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/general_matching.md
// API一覧
// GeneralMatching(n=0): 頂点0..n-1の無向グラフ。n>=0、構築O(n)。empty()は頂点数0の判定。
// add_edge(u,v): 無向辺を追加、自己loopは無視、多重辺可。償却O(1)。対応は保ち最大性を無効化。
// reserve_edges(cap): 辺容量を予約、再確保時O(m)。edge_count()は自己loopを除いた追加件数。
// edges(): 追加順のconst span<pair<int32_t,int32_t>>。重複・端点の入力順も保持する。
// solve(): 現在の対応を最大マッチングへ拡張して総本数を返す。再帰なし。最大性既知ならO(1)。
//   n頂点m辺としてO(n^3+nm)時間・追加O(n+m)領域。単純グラフではO(n^3)。
// size()/empty()/matching_size()/is_maximum()/is_perfect(): O(1)。空グラフはperfect。
// mate(v): 対応先のint32_t、未対応は-1。partners(): 全対応のconst span。各O(1)。
// pairs(): u<vの対応辺をuの昇順で返す。O(n)時間・O(matching_size())領域。
// solve前/辺追加後の対応も有効だが、is_maximum()==falseなら最大とは限らない。
// clear_matching(): 辺を保ち対応を消す、O(n)。clear_edges(): 頂点数を保ち辺/対応を消す、O(n+m)。
// reset(n=0)/clear(): 指定頂点数/0頂点に初期化。copyはO(n+m)で独立、move元は0頂点、swapはO(1)。
// copyはO(n+m)。reset/clearは旧データ破棄と新頂点初期化に比例。
// add_edge/reset/copy代入は内容に強い例外保証。solveは全作業領域確保後のみ対応を変更する。
// partners viewはreset/clear/代入/所有権移動まで有効。edges viewは追加/reserve/clear_edgesでも無効化し得る。
// 頂点番号/負数の検査はSKIP_RUNTIME_CHECKSで除外。容量・n+1/2mの添字保護は常に維持する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GENERAL_MATCHING_WORKSPACE_DEFINED
#define SIRKIT_GENERAL_MATCHING_WORKSPACE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline std::size_t GeneralMatchingArcCount(std::size_t edges) {
    if (edges > std::vector<std::int32_t>{}.max_size() / 2)
        throw std::length_error("GeneralMatching workspace capacity");
    return edges * 2;
}

struct GeneralMatchingWorkspace {
    std::vector<std::size_t> offsets, cursor;
    std::vector<std::int32_t> neighbors, parent, base, queue;
    std::vector<std::uint8_t> outer, ancestor, contracted;
    std::uint8_t generation = 0;
    GeneralMatchingWorkspace(std::size_t n, std::size_t arcs)
        : offsets(n + 1), cursor(n + 1), neighbors(arcs), parent(n) {}
};
}
#endif

#ifndef SIRKIT_GENERAL_MATCHING_DEFINED
#define SIRKIT_GENERAL_MATCHING_DEFINED
class GeneralMatching {
    std::vector<std::pair<std::int32_t, std::int32_t>> edges_;
    std::vector<std::int32_t> partners_;
    std::int32_t cardinality_ = 0;
    bool maximum_ = true;
    void check_vertex(std::int32_t vertex) const {
        (void)vertex;
    }
    bool augment(std::int32_t root, /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::GeneralMatchingWorkspace& work) noexcept {
        // cursorの先頭n要素は到達一覧、末尾は前回の到達数。
        const auto previous_reached = work.cursor[partners_.size()];
        if (previous_reached == 0 || previous_reached > partners_.size() / 4) {
            std::fill(work.parent.begin(), work.parent.end(), -1);
            std::iota(work.base.begin(), work.base.end(), std::int32_t{0});
            std::fill(work.outer.begin(), work.outer.end(), std::uint8_t{0});
        } else {
            for (std::size_t index = 0; index < previous_reached; ++index) {
                const auto vertex = work.cursor[index];
                work.parent[vertex] = -1;
                work.base[vertex] = static_cast<std::int32_t>(vertex);
                work.outer[vertex] = 0;
            }
        }
        // 前の一覧を読み終えてから、今回の到達一覧に置き換える。
        std::size_t head = 0, tail = 0;
        std::size_t reached = 1;
        work.cursor[0] = static_cast<std::size_t>(root);
        work.outer[static_cast<std::size_t>(root)] = 1; work.queue[tail++] = root;
        auto join = [&](std::int32_t a, std::int32_t b) {
            if (++work.generation == 0) {
                std::fill(work.ancestor.begin(), work.ancestor.end(), std::uint8_t{0});
                std::fill(work.contracted.begin(), work.contracted.end(), std::uint8_t{0});
                work.generation = 1;
            }
            for (;;) {
                a = work.base[static_cast<std::size_t>(a)];
                work.ancestor[static_cast<std::size_t>(a)] = work.generation;
                const auto mate = partners_[static_cast<std::size_t>(a)];
                if (mate == -1) break;
                a = work.parent[static_cast<std::size_t>(mate)];
            }
            for (;;) {
                b = work.base[static_cast<std::size_t>(b)];
                if (work.ancestor[static_cast<std::size_t>(b)] == work.generation) return b;
                b = work.parent[static_cast<std::size_t>(partners_[static_cast<std::size_t>(b)])];
            }
        };
        auto mark = [&](std::int32_t vertex, std::int32_t common, std::int32_t child) {
            while (work.base[static_cast<std::size_t>(vertex)] != common) {
                const auto mate = partners_[static_cast<std::size_t>(vertex)];
                work.contracted[static_cast<std::size_t>(work.base[static_cast<std::size_t>(vertex)])] = work.generation;
                work.contracted[static_cast<std::size_t>(work.base[static_cast<std::size_t>(mate)])] = work.generation;
                work.parent[static_cast<std::size_t>(vertex)] = child;
                child = mate; vertex = work.parent[static_cast<std::size_t>(mate)];
            }
        };
        while (head < tail) {
            const auto u = work.queue[head++]; const auto ui = static_cast<std::size_t>(u);
            for (auto edge = work.offsets[ui]; edge < work.offsets[ui + 1]; ++edge) {
                const auto v = work.neighbors[edge]; const auto vi = static_cast<std::size_t>(v);
                if (work.base[ui] == work.base[vi] || partners_[ui] == v) continue;
                if (work.outer[vi]) {
                    const auto common = join(u, v);
                    mark(u, common, v); mark(v, common, u);
                    for (std::size_t index = 0; index < reached; ++index) {
                        const auto i = work.cursor[index];
                        if (work.contracted[static_cast<std::size_t>(work.base[i])] != work.generation) continue;
                        work.base[i] = common;
                        if (!work.outer[i]) { work.outer[i] = 1; work.queue[tail++] = static_cast<std::int32_t>(i); }
                    }
                } else if (work.parent[vi] == -1) {
                    work.parent[vi] = u;
                    work.cursor[reached++] = vi;
                    if (partners_[vi] == -1) {
                        // 縮約時に付け替えた親をたどり、増加路の対応を反転する。
                        auto vertex = v;
                        while (vertex != -1) {
                            const auto parent = work.parent[static_cast<std::size_t>(vertex)];
                            // 反転する各頂点に親がある。未対応の探索根と結ぶと次の頂点が-1になる。
                            const auto next = partners_[static_cast<std::size_t>(parent)];
                            partners_[static_cast<std::size_t>(vertex)] = parent;
                            partners_[static_cast<std::size_t>(parent)] = vertex;
                            vertex = next;
                        }
                        work.cursor[partners_.size()] = reached;
                        ++cardinality_; return true;
                    }
                    const auto mate = partners_[vi];
                    work.cursor[reached++] = static_cast<std::size_t>(mate);
                    work.outer[static_cast<std::size_t>(mate)] = 1; work.queue[tail++] = mate;
                }
            }
        }
        work.cursor[partners_.size()] = reached;
        return false;
    }
public:
    explicit GeneralMatching(std::int32_t n = 0) {
        partners_.assign(static_cast<std::size_t>(n), -1);
    }
    GeneralMatching(const GeneralMatching&) = default;
    GeneralMatching& operator=(const GeneralMatching& other) {
        if (this != &other) { GeneralMatching copy(other); swap(copy); }
        return *this;
    }
    GeneralMatching(GeneralMatching&& other) noexcept { swap(other); }
    GeneralMatching& operator=(GeneralMatching&& other) noexcept {
        if (this != &other) { GeneralMatching moved(std::move(other)); swap(moved); }
        return *this;
    }
    void swap(GeneralMatching& other) noexcept {
        edges_.swap(other.edges_); partners_.swap(other.partners_);
        std::swap(cardinality_, other.cardinality_); std::swap(maximum_, other.maximum_);
    }
    friend void swap(GeneralMatching& a, GeneralMatching& b) noexcept { a.swap(b); }
    std::int32_t size() const noexcept { return static_cast<std::int32_t>(partners_.size()); }
    bool empty() const noexcept { return partners_.empty(); }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    std::int32_t matching_size() const noexcept { return cardinality_; }
    bool is_maximum() const noexcept { return maximum_; }
    bool is_perfect() const noexcept { return static_cast<std::size_t>(cardinality_) * 2 == partners_.size(); }
    std::int32_t mate(std::int32_t vertex) const { check_vertex(vertex); return partners_[static_cast<std::size_t>(vertex)]; }
    std::span<const std::int32_t> partners() const noexcept { return partners_; }
    std::span<const std::pair<std::int32_t, std::int32_t>> edges() const noexcept { return edges_; }
    void reserve_edges(std::size_t capacity) { edges_.reserve(capacity); }
    void add_edge(std::int32_t u, std::int32_t v) {
        check_vertex(u); check_vertex(v); if (u == v) return;
        edges_.emplace_back(u, v); maximum_ = false;
    }
    std::int32_t solve() {
        if (maximum_) return cardinality_;
        if (static_cast<std::size_t>(cardinality_) == partners_.size() / 2) { maximum_ = true; return cardinality_; }
        const auto n = partners_.size(), m = edges_.size();
        // nは既に確保したint32_t配列の要素数なので、n+1はsize_tに収まる。
        const auto arcs = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::GeneralMatchingArcCount(m);
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::GeneralMatchingWorkspace work(n, arcs);
        for (const auto& [u,v] : edges_) { ++work.offsets[static_cast<std::size_t>(u) + 1]; ++work.offsets[static_cast<std::size_t>(v) + 1]; }
        std::size_t active_vertices = 0;
        for (std::size_t i = 0; i < n; ++i) {
            work.offsets[i + 1] += work.offsets[i]; work.cursor[i] = work.offsets[i];
            active_vertices += work.offsets[i + 1] != work.offsets[i];
        }
        for (const auto& [u,v] : edges_) {
            work.neighbors[work.cursor[static_cast<std::size_t>(u)]++] = v;
            work.neighbors[work.cursor[static_cast<std::size_t>(v)]++] = u;
        }
        // 親配列を一時的な対応表に使い、必要な確保が終わるまで結果を変更しない。
        std::copy(partners_.begin(), partners_.end(), work.parent.begin());
        auto next_cardinality = cardinality_;
        for (std::size_t u = 0; u < n; ++u) if (work.parent[u] == -1) {
            for (auto edge = work.offsets[u]; edge < work.offsets[u + 1]; ++edge) {
                const auto v = work.neighbors[edge];
                if (work.parent[static_cast<std::size_t>(v)] != -1) continue;
                work.parent[u] = v; work.parent[static_cast<std::size_t>(v)] = static_cast<std::int32_t>(u);
                ++next_cardinality; break;
            }
        }
        const auto matching_bound = active_vertices / 2;
        // 貪欲な対応だけで上界に届けば、増加路探索用の5配列は確保しない。
        if (static_cast<std::size_t>(next_cardinality) != matching_bound) {
            work.base.resize(n); work.queue.resize(n); work.outer.resize(n);
            work.ancestor.resize(n); work.contracted.resize(n);
        }
        std::copy(work.parent.begin(), work.parent.end(), partners_.begin());
        cardinality_ = next_cardinality;
        if (static_cast<std::size_t>(cardinality_) != matching_bound) {
            for (std::int32_t root = 0; root < size(); ++root) {
                const auto ri = static_cast<std::size_t>(root);
                if (partners_[ri] == -1 && work.offsets[ri] != work.offsets[ri + 1]) {
                    augment(root, work);
                    if (static_cast<std::size_t>(cardinality_) == matching_bound) break;
                }
            }
        }
        maximum_ = true; return cardinality_;
    }
    std::vector<std::pair<std::int32_t, std::int32_t>> pairs() const {
        std::vector<std::pair<std::int32_t, std::int32_t>> result;
        result.reserve(static_cast<std::size_t>(cardinality_));
        for (std::int32_t u = 0; u < size(); ++u) if (u < partners_[static_cast<std::size_t>(u)]) result.emplace_back(u, partners_[static_cast<std::size_t>(u)]);
        return result;
    }
    void clear_matching() noexcept {
        std::fill(partners_.begin(), partners_.end(), -1); cardinality_ = 0; maximum_ = edges_.empty();
    }
    void clear_edges() noexcept { edges_.clear(); clear_matching(); }
    void reset(std::int32_t n = 0) { GeneralMatching next(n); swap(next); }
    void clear() { reset(); }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
