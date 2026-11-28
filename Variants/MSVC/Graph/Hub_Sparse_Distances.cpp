// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Hub_Sparse_Distances.cpp
// 参考（ハブ経由の移動と少数の短縮辺を使うVerify）: https://yukicoder.me/problems/no/3810
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hub_sparse_distances.md
//
// API一覧
// HubSparseDistances(gates, shortcuts): 任意の異なるu,vをgates[u]+gates[v]で結ぶ無向グラフに追加辺を与えて前処理する。
// Shortcut{from,to,cost}: 0-indexedの無向辺。多重辺、自己辺、費用0を許す。gates[i]はUINT64_MAX/2以下。
// distance(u,v): 最短距離をuint64_tで返す。同じ頂点の距離は0。O(1)。size(): 頂点数。O(1)。
// 追加辺m本、頂点n個。0辺を縮約しハブ経由以上の辺を除いた残りの端点数をkとする。
// 前処理はO(1+(n+m)α(n+1)+k^3)時間、O(1+n+k²)領域。nはINT_MAX以下。
//
// [同梱のUnionFind]
// [UnionFind]
// UnionFind(n=0): n要素を独立成分として構築。O(n+1)時間・領域、n<=INT_MAX。
// root(i)/leader(i): 代表、unite(a,b): 結合（戻り値なし）、merge(a,b): 新規結合ならtrue。償却O(α(n+1))。
// issame(a,b)/same(a,b): 同一成分か、size(i): 成分サイズ。償却O(α(n+1))。探索時に経路を圧縮する。
// size()/empty()/component_count(): 全要素数/空か/成分数、O(1)。groups(): 頂点昇順の成分一覧、O(1+n α(n+1))償却。
// reset(n)/reset(): n要素/現在の要素数へ初期化。容量を再利用、O(旧n+n+1)上界。確保失敗時は元を保つ。
// clear(): 空へ戻して容量保持、O(旧n+1)上界、noexcept。swap(other): 内容交換、O(1)、noexcept。
// コピーはO(n+1)で独立、移動元は空。自己代入・自己ムーブは内容を保つ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cstdint>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNION_FIND_DEFINED
#define SIRKIT_UNION_FIND_DEFINED
class UnionFind {
    std::vector<int> param;
    int components_ = 0;
    static std::size_t checked_size(std::size_t n) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("UnionFind: too many vertices");
        return n;
    }
    void check(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(i) >= param.size())
            throw std::out_of_range("UnionFind: vertex");
#else
        (void)i;
#endif
    }
    int leader_unchecked(int i) {
        while (param[i] >= 0) {
            const int p = param[i];
            if (param[p] < 0) return p;
            i = param[i] = param[p];
        }
        return i;
    }
public:
    explicit UnionFind(std::size_t n = 0)
        : param(checked_size(n), -1), components_(static_cast<int>(n)) {}
    UnionFind(const UnionFind&) = default;
    UnionFind& operator=(const UnionFind&) = default;
    UnionFind(UnionFind&& other) noexcept
        : param(std::move(other.param)), components_(std::exchange(other.components_, 0)) { other.param.clear(); }
    UnionFind& operator=(UnionFind&& other) noexcept {
        if (this != &other) {
            param = std::move(other.param);
            components_ = std::exchange(other.components_, 0); other.param.clear();
        }
        return *this;
    }
    void reset(std::size_t n) {
        const auto count = checked_size(n);
        param.reserve(count);
        param.clear(); param.resize(count, -1);
        components_ = static_cast<int>(count);
    }
    void reset() { reset(param.size()); }
    void clear() noexcept { param.clear(); components_ = 0; }
    bool empty() const noexcept { return param.empty(); }
    void swap(UnionFind& other) noexcept {
        param.swap(other.param); std::swap(components_, other.components_);
    }
    friend void swap(UnionFind& left, UnionFind& right) noexcept { left.swap(right); }
    int root(int i) { check(i); return leader_unchecked(i); }
    int leader(int i) { return root(i); }
    bool merge(int a, int b) {
        check(a); check(b);
        a = leader_unchecked(a); b = leader_unchecked(b);
        if (a == b) return false;
        if (param[a] > param[b]) std::swap(a, b);
        param[a] += param[b]; param[b] = a; --components_;
        return true;
    }
    void unite(int a, int b) { (void)merge(a, b); }
    bool same(int a, int b) { check(a); check(b); return leader_unchecked(a) == leader_unchecked(b); }
    bool issame(int a, int b) { return same(a, b); }
    int size(int i) { return -param[root(i)]; }
    int size() const noexcept { return static_cast<int>(param.size()); }
    int component_count() const noexcept { return components_; }
    std::vector<std::vector<int>> groups() {
        std::vector<std::vector<int>> result;
        result.reserve(static_cast<std::size_t>(components_));
        if (components_ == size()) {
            for (int v = 0; v < size(); ++v) result.emplace_back(std::size_t{1}, v);
            return result;
        }
        if (components_ == 1) {
            const int root = leader_unchecked(0);
            result.emplace_back(param.size());
            const int negative_size = param[root];
            std::fill(param.begin(), param.end(), root);
            param[root] = negative_size;
            std::iota(result[0].begin(), result[0].end(), 0);
            return result;
        }
        std::vector<int> slot(param.size());
        for (int i = 0; i < size(); ++i) if (param[i] < 0) {
            slot[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(static_cast<std::size_t>(-param[i]));
        }
        for (int i = 0; i < size(); ++i) result[slot[leader_unchecked(i)]].push_back(i);
        return result;
    }
};
#endif

#ifndef SIRKIT_HUB_SPARSE_DISTANCES
#define SIRKIT_HUB_SPARSE_DISTANCES

class HubSparseDistances
{
public:
    struct Shortcut
    {
        int from, to;
        std::uint64_t cost;
    };

    HubSparseDistances(std::span<const std::uint64_t> gates, std::span<const Shortcut> shortcuts)
    {
        UnionFind components(gates.size());
        component_.resize(gates.size());
        for (const auto &shortcut : shortcuts)
        {
            check_vertex(shortcut.from);
            check_vertex(shortcut.to);
            if (shortcut.cost == 0)
            {
                components.unite(shortcut.from, shortcut.to);
            }
        }
        gate_distance_.assign(gates.size(), kMaxGate);
        for (std::size_t v = 0; v < gates.size(); ++v)
        {
            if (gates[v] > kMaxGate)
            {
                throw std::invalid_argument("HubSparseDistances: gate cost too large");
            }
            const auto root = components.root(static_cast<int>(v));
            component_[v] = root;
            gate_distance_[root] = std::min(gate_distance_[root], gates[v]);
        }
        core_index_.resize(gates.size());
        // 行列の添字0をハブに、1以降を残った短縮辺の端点に割り当てる。
        std::vector<std::uint64_t> core_gates{0};
        for (const auto &shortcut : shortcuts)
        {
            const auto from = component_[shortcut.from];
            const auto to = component_[shortcut.to];
            if (from == to || shortcut.cost >= gate_distance_[from] + gate_distance_[to])
            {
                continue;
            }
            for (const auto vertex : {from, to})
            {
                if (core_index_[vertex] == 0)
                {
                    core_index_[vertex] = core_gates.size();
                    core_gates.push_back(gate_distance_[vertex]);
                }
            }
        }
        side_ = core_gates.size();
        distances_.resize(side_ * side_);
        for (std::size_t i = 0; i < side_; ++i)
        {
            for (std::size_t j = i + 1; j < side_; ++j)
            {
                distances_[i * side_ + j] = distances_[j * side_ + i] = core_gates[i] + core_gates[j];
            }
        }
        for (const auto &shortcut : shortcuts)
        {
            const auto from = core_index_[component_[shortcut.from]];
            const auto to = core_index_[component_[shortcut.to]];
            if (from != 0 && to != 0)
            {
                auto &d = distances_[from * side_ + to];
                d = std::min(d, shortcut.cost);
                distances_[to * side_ + from] = d;
            }
        }
        for (std::size_t pivot = 1; pivot < side_; ++pivot)
        {
            for (std::size_t from = 0; from < side_; ++from)
            {
                const auto head = distances_[from * side_ + pivot];
                for (std::size_t to = 0; to < side_; ++to)
                {
                    const auto tail = distances_[pivot * side_ + to];
                    auto &direct = distances_[from * side_ + to];
                    // 比較が先なので、head+tailがuint64_tの上限を超える場合も安全。
                    if (head < direct && tail < direct - head)
                    {
                        direct = head + tail;
                    }
                }
            }
        }
        for (std::size_t v = 0; v < gates.size(); ++v)
        {
            if (core_index_[v] != 0)
            {
                gate_distance_[v] = distances_[core_index_[v]];
            }
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return component_.size();
    }

    [[nodiscard]] std::uint64_t distance(int from, int to) const
    {
        check_vertex(from);
        check_vertex(to);
        const auto a = component_[from];
        const auto b = component_[to];
        if (a == b)
        {
            return 0;
        }
        const auto i = core_index_[a];
        const auto j = core_index_[b];
        if (i != 0 && j != 0)
        {
            return distances_[i * side_ + j];
        }
        return gate_distance_[a] + gate_distance_[b];
    }

private:
    static constexpr auto kMaxGate = std::numeric_limits<std::uint64_t>::max() / 2;
    std::size_t side_ = 0;
    std::vector<int> component_;
    std::vector<std::size_t> core_index_;
    std::vector<std::uint64_t> gate_distance_, distances_;

    void check_vertex(int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(vertex) >= size())
        {
            throw std::out_of_range("HubSparseDistances: vertex out of range");
        }
#else
        static_cast<void>(vertex);
#endif
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
