// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Range_Edge_Graph.cpp
// 参考（セグメント木での区間辺の圧縮）: https://atcoder.jp/contests/abc414/editorial/13415
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_edge_graph.md
//
// API一覧
// RangeEdgeGraph(n): 頂点0..n-1の有向グラフ。定数の辺費用を使う層0を用意する。O(n)時間・領域。
// add_layer(departure,arrival): 頂点ごとの出発・到着費用を持つ層を追加し、層番号を返す。O(n)。
// add_range_edge(l,r,L,R,w,layer=0): [l,r)のuから[L,R)のvへ w+departure[u]+arrival[v] の辺を追加。
// 空区間があれば何もしない。補助頂点は高々2個、辺の追加O(log(n+1))、辺列拡張を含め償却。
// add_edge(u,v,w): 元の頂点間の有向辺を1本追加。償却O(1)。重みの型はstd::int64_t。
// size()/vertex_count(): 元の頂点数/補助頂点込みの頂点数。edges(): (始点,終点,重み)の辺列へのconst参照。各O(1)。
// graph(): WGraph形式に変換して返す。拡張後の頂点数Vと辺数Eに対しO(V+E)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRAPH_TYPES_DEFINED
#define SIRKIT_GRAPH_TYPES_DEFINED
struct Edge
{
    int to;
    std::int64_t w;
    Edge(int destination, std::int64_t weight) : to(destination), w(weight) {}
};
using WGraph = std::vector<std::vector<Edge>>;
#endif

#ifndef SIRKIT_RANGE_EDGE_GRAPH_DEFINED
#define SIRKIT_RANGE_EDGE_GRAPH_DEFINED

namespace detail
{
inline void check_range_edge_vertex_count(int current, std::size_t extra)
{
    if (extra > static_cast<std::size_t>(std::numeric_limits<int>::max() - current))
    {
        throw std::length_error("RangeEdgeGraph: too many vertices");
    }
}
}

class RangeEdgeGraph
{
public:
    using WeightedEdge = std::tuple<int, int, std::int64_t>;

    explicit RangeEdgeGraph(int n) : n_(n), vertex_count_(n)
    {
        if (n < 0)
        {
            throw std::length_error("RangeEdgeGraph: negative vertex count");
        }
        append_layer({}, {});
    }

    [[nodiscard]] int size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] int vertex_count() const noexcept
    {
        return vertex_count_;
    }

    [[nodiscard]] const std::vector<WeightedEdge> &edges() const noexcept
    {
        return edges_;
    }

    std::size_t add_layer(std::span<const std::int64_t> departure, std::span<const std::int64_t> arrival)
    {
        return append_layer(departure, arrival);
    }

    void add_edge(int u, int v, std::int64_t w)
    {
        edges_.emplace_back(u, v, w);
    }

    void add_range_edge(int l, int r, int left, int right, std::int64_t w, std::size_t layer = 0)
    {
        if (l == r || left == right)
        {
            return;
        }
        const auto &costs = layers_[layer];
        Cover from_buffer, to_buffer;
        const auto from = cover(l, r, from_buffer);
        const auto to = cover(left, right, to_buffer);
        const auto first = minimum(costs.departure, from), second = minimum(costs.arrival, to);
        const auto middle = add_three(w, first, second);
        detail::check_range_edge_vertex_count(vertex_count_, 2);
        const int board = vertex_count_, arrive = board + 1;
        std::array<WeightedEdge, 2 * kCoverCapacity + 1> added;
        std::size_t count = 0;
        for (const int node : from)
        {
            added[count++] = {vertex(costs, node, true), board, costs.departure[node] - first};
        }
        added[count++] = {board, arrive, middle};
        for (const int node : to)
        {
            added[count++] = {arrive, vertex(costs, node, false), costs.arrival[node] - second};
        }
        edges_.insert(edges_.end(), added.begin(), added.begin() + count);
        vertex_count_ += 2;
    }

    [[nodiscard]] WGraph graph() const
    {
        WGraph result(vertex_count_);
        std::vector<std::size_t> degrees(vertex_count_);
        for (const auto &[from, to, cost] : edges_)
        {
            ++degrees[from];
        }
        for (int v = 0; v < vertex_count_; ++v)
        {
            result[v].reserve(degrees[v]);
        }
        for (const auto &[from, to, cost] : edges_)
        {
            result[from].emplace_back(to, cost);
        }
        return result;
    }

private:
    // 半開区間の分解では各高さから高々2頂点を使う。
    static constexpr std::size_t kCoverCapacity = std::numeric_limits<unsigned int>::digits * 2;
    using Cover = std::array<int, kCoverCapacity>;
    struct Layer
    {
        int base;
        std::vector<std::int64_t> departure;
        std::vector<std::int64_t> arrival;
    };


    static std::int64_t safe_add(std::int64_t a, std::int64_t b)
    {
        if ((b > 0 && a > std::numeric_limits<std::int64_t>::max() - b) ||
            (b < 0 && a < std::numeric_limits<std::int64_t>::min() - b))
        {
            throw std::overflow_error("RangeEdgeGraph: range edge cost out of range");
        }
        return a + b;
    }

    static std::int64_t add_three(std::int64_t a, std::int64_t b, std::int64_t c)
    {
        // 異符号の2項を先に足せば、中間結果だけがはみ出すことを避けられる。
        if ((a < 0) == (b < 0))
        {
            return safe_add(safe_add(a, c), b);
        }
        return safe_add(safe_add(a, b), c);
    }

    std::vector<std::int64_t> anchors(std::span<const std::int64_t> costs) const
    {
        std::vector<std::int64_t> result(static_cast<std::size_t>(n_) * 2);
        if (!costs.empty())
        {
            const auto [minimum_cost, maximum_cost] = std::minmax_element(costs.begin(), costs.end());
            const auto spread = static_cast<std::uint64_t>(*maximum_cost) - static_cast<std::uint64_t>(*minimum_cost);
            if (spread > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
            {
                throw std::overflow_error("RangeEdgeGraph: cost layer span out of range");
            }
            std::copy(costs.begin(), costs.end(), result.begin() + n_);
            for (int i = n_ - 1; i > 0; --i)
            {
                result[i] = std::min(result[2 * i], result[2 * i + 1]);
            }
        }
        return result;
    }

    int vertex(const Layer &layer, int node, bool upward) const noexcept
    {
        return node >= n_ ? node - n_ : layer.base + node - 1 + (upward ? 0 : n_ - 1);
    }

    std::size_t append_layer(std::span<const std::int64_t> departure, std::span<const std::int64_t> arrival)
    {
        const auto extra = 2 * static_cast<std::size_t>(std::max(n_ - 1, 0));
        detail::check_range_edge_vertex_count(vertex_count_, extra);
        Layer layer{vertex_count_, anchors(departure), anchors(arrival)};
        std::vector<WeightedEdge> added;
        added.reserve(2 * extra);
        for (int node = 2; node < 2 * n_; ++node)
        {
            const int parent = node / 2;
            added.emplace_back(vertex(layer, node, true), vertex(layer, parent, true),
                               layer.departure[node] - layer.departure[parent]);
            added.emplace_back(vertex(layer, parent, false), vertex(layer, node, false),
                               layer.arrival[node] - layer.arrival[parent]);
        }
        const auto needed_layers = layers_.size() + 1;
        if (needed_layers > layers_.capacity())
        {
            // Layerは2バイトより大きく、要素数の2倍はsize_tに収まる。
            const auto capacity = std::min(layers_.max_size(), 2 * (needed_layers - 1) + 1);
            layers_.reserve(std::max(needed_layers, capacity));
        }
        // 辺列の確保が失敗しても、層を公開する前なのでグラフは変わらない。
        edges_.insert(edges_.end(), added.begin(), added.end());
        layers_.push_back(std::move(layer));
        vertex_count_ += static_cast<int>(extra);
        return layers_.size() - 1;
    }

    std::span<const int> cover(int l, int r, Cover &output) const
    {
        std::size_t count = 0;
        for (l += n_, r += n_; l < r; l /= 2, r /= 2)
        {
            if (l & 1)
            {
                output[count++] = l++;
            }
            if (r & 1)
            {
                output[count++] = --r;
            }
        }
        return {output.data(), count};
    }

    static std::int64_t minimum(const std::vector<std::int64_t> &anchors, std::span<const int> interval_nodes)
    {
        auto result = std::numeric_limits<std::int64_t>::max();
        for (const int node : interval_nodes)
        {
            result = std::min(result, anchors[node]);
        }
        return result;
    }

    int n_, vertex_count_;
    std::vector<Layer> layers_;
    std::vector<WeightedEdge> edges_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
