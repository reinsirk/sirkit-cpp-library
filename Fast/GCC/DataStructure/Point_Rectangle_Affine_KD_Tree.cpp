// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Point_Rectangle_Affine_KD_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/point_rectangle_affine_kd_tree.md
//
// API一覧
// PointRectangleAffineKDTree<Mod>(points): 点の座標を登録する。各点は非活性で始まる。
// set(id,w): 点を有効化して値をwにする。再代入可。O(log(M+1))。
// range_affine(rect,a,b): 半開長方形内の有効な点だけw←a*w+b。O(√M+1)。
// range_sum(rect): 半開長方形内の有効な点の和。O(√M+1)。空長方形は0。
// size()/active_count()/is_active(id)/all_sum(): 登録数、有効数、点の状態、全体和。各O(1)。
// IDは登録時の0始まり添字。同一座標も点ごとに扱う。後から有効化した点は過去の更新を継がない。
// 座標・引数の重みと係数はint64_t。Modは正のuint32_tで既定998244353。
// M点の登録は平均O(1+M log(M+1))時間、O(M+1)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
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
#ifndef SIRKIT_POINT_RECTANGLE_AFFINE_KD_TREE_DEFINED
#define SIRKIT_POINT_RECTANGLE_AFFINE_KD_TREE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedPointRectangleAffineNodeCount(std::size_t count, std::size_t capacity)
{
    if (count == 0)
    {
        return 0;
    }
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        count > capacity / 2 + capacity % 2)
    {
        throw std::length_error("PointRectangleAffineKDTree: too many points");
    }
    return 2 * count - 1;
}
}

template <std::uint32_t Mod = 998244353>
class PointRectangleAffineKDTree
{
    static_assert(Mod > 0);

public:
    struct Point
    {
        std::int64_t x, y;
    };

    struct Rectangle
    {
        std::int64_t left, bottom, right, top;
    };

    explicit PointRectangleAffineKDTree(std::span<const Point> points = {})
    {
        nodes_.resize(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPointRectangleAffineNodeCount(points.size(), nodes_.max_size()));
        if (points.empty())
        {
            return;
        }
        leaf_.resize(points.size());
        std::vector<Candidate> candidates;
        candidates.reserve(points.size());
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            candidates.push_back({points[i], static_cast<Index>(i)});
        }
        Build(0, candidates, true);
    }

    PointRectangleAffineKDTree(const PointRectangleAffineKDTree &) = default;

    PointRectangleAffineKDTree &operator=(const PointRectangleAffineKDTree &other)
    {
        if (this != &other)
        {
            PointRectangleAffineKDTree next(other);
            nodes_.swap(next.nodes_);
            leaf_.swap(next.leaf_);
        }
        return *this;
    }

    PointRectangleAffineKDTree(PointRectangleAffineKDTree &&other) noexcept
        : nodes_(std::move(other.nodes_)), leaf_(std::move(other.leaf_))
    {
        other.nodes_.clear();
        other.leaf_.clear();
    }

    PointRectangleAffineKDTree &operator=(PointRectangleAffineKDTree &&other) noexcept
    {
        if (this != &other)
        {
            nodes_ = std::move(other.nodes_);
            leaf_ = std::move(other.leaf_);
            other.nodes_.clear();
            other.leaf_.clear();
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(leaf_.size());
    }

    [[nodiscard]] int active_count() const noexcept
    {
        return nodes_.empty() ? 0 : static_cast<int>(nodes_[0].count);
    }

    [[nodiscard]] std::uint32_t all_sum() const noexcept
    {
        return nodes_.empty() ? 0 : nodes_[0].sum;
    }

    [[nodiscard]] bool is_active(int id) const
    {
        CheckPoint(id);
        return nodes_[leaf_[id]].count != 0;
    }

    void set(int id, std::int64_t value)
    {
        CheckPoint(id);
        Set(0, leaf_[id], Normalize(value));
    }

    void range_affine(Rectangle rectangle, std::int64_t a, std::int64_t b)
    {
        if (!Nonempty(rectangle) || nodes_.empty())
        {
            return;
        }
        const auto multiplier = Normalize(a);
        const auto addition = Normalize(b);
        if (multiplier == kOne && addition == 0)
        {
            return;
        }
        Update(0, rectangle, multiplier, addition);
    }

    [[nodiscard]] std::uint32_t range_sum(Rectangle rectangle)
    {
        if (!Nonempty(rectangle) || nodes_.empty())
        {
            return 0;
        }
        return Sum(0, rectangle);
    }

private:
    using Index = std::uint32_t;
    static constexpr std::uint32_t kOne = 1 % Mod;

    struct Node
    {
        std::int64_t low_x = 0, high_x = 0, low_y = 0, high_y = 0;
        Index right = 0;
        std::uint32_t count = 0, sum = 0;
        std::uint32_t multiplier = kOne, addition = 0;
    };

    struct Candidate
    {
        Point point;
        Index id;
    };

    std::vector<Node> nodes_;
    std::vector<Index> leaf_;

    void CheckPoint(int id) const
    {
        (void)id;
    }

    static bool Nonempty(Rectangle rect)
    {
        return rect.left < rect.right && rect.bottom < rect.top;
    }

    static std::uint32_t Normalize(std::int64_t value) noexcept
    {
        const auto residue = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue);
    }

    static std::uint32_t Add(std::uint32_t a, std::uint32_t b) noexcept
    {
        const auto sum = std::uint64_t{a} + b;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Transform(std::uint32_t a, std::uint32_t b,
                                   std::uint32_t sum, std::uint32_t count) noexcept
    {
        const auto product = std::uint64_t{a} * sum;
        const auto offset = std::uint64_t{b} * count;
        if constexpr (Mod <= (std::uint32_t{1} << 31))
        {
            return static_cast<std::uint32_t>((product + offset) % Mod);
        }
        else
        {
            return Add(static_cast<std::uint32_t>(product % Mod), static_cast<std::uint32_t>(offset % Mod));
        }
    }

    static bool Disjoint(const Node &node, const Rectangle &rect) noexcept
    {
        return node.high_x < rect.left || node.low_x >= rect.right ||
               node.high_y < rect.bottom || node.low_y >= rect.top;
    }

    static bool Contained(const Node &node, const Rectangle &rect) noexcept
    {
        return rect.left <= node.low_x && node.high_x < rect.right &&
               rect.bottom <= node.low_y && node.high_y < rect.top;
    }

    void Build(Index index, std::span<Candidate> candidates, bool x_axis)
    {
        auto &node = nodes_[index];
        if (candidates.size() == 1)
        {
            const auto &[p, id] = candidates.front();
            node.low_x = node.high_x = p.x;
            node.low_y = node.high_y = p.y;
            leaf_[id] = index;
            return;
        }

        const auto half = candidates.size() / 2;
        std::nth_element(candidates.begin(), candidates.begin() + half, candidates.end(),
                         [x_axis](const Candidate &a, const Candidate &b)
                         {
                             if (x_axis)
                             {
                                 return std::tie(a.point.x, a.point.y, a.id) < std::tie(b.point.x, b.point.y, b.id);
                             }
                             return std::tie(a.point.y, a.point.x, a.id) < std::tie(b.point.y, b.point.x, b.id);
                         });

        // 部分木を前順で連続配置し、左の根と各点への経路を添字だけで復元する。
        node.right = static_cast<Index>(index + 2 * half);
        Build(index + 1, candidates.first(half), !x_axis);
        Build(node.right, candidates.subspan(half), !x_axis);
        const auto &left = nodes_[index + 1];
        const auto &right = nodes_[node.right];
        node.low_x = std::min(left.low_x, right.low_x);
        node.high_x = std::max(left.high_x, right.high_x);
        node.low_y = std::min(left.low_y, right.low_y);
        node.high_y = std::max(left.high_y, right.high_y);
    }

    void Apply(Node &node, std::uint32_t a, std::uint32_t b) noexcept
    {
        if (node.count == 0)
        {
            return;
        }
        node.sum = Transform(a, b, node.sum, node.count);
        if (node.right != 0)
        {
            node.multiplier = static_cast<std::uint32_t>(std::uint64_t{a} * node.multiplier % Mod);
            node.addition = Transform(a, b, node.addition, 1);
        }
    }

    void Push(Index index) noexcept
    {
        auto &node = nodes_[index];
        if (node.multiplier == kOne && node.addition == 0)
        {
            return;
        }
        Apply(nodes_[index + 1], node.multiplier, node.addition);
        Apply(nodes_[node.right], node.multiplier, node.addition);
        node.multiplier = kOne;
        node.addition = 0;
    }

    void Pull(Index index) noexcept
    {
        auto &node = nodes_[index];
        const auto &left = nodes_[index + 1];
        const auto &right = nodes_[node.right];
        node.count = left.count + right.count;
        node.sum = Add(left.sum, right.sum);
    }

    void Set(Index index, Index leaf, std::uint32_t value) noexcept
    {
        auto &node = nodes_[index];
        if (index == leaf)
        {
            node.count = 1;
            node.sum = value;
            return;
        }

        // 先に伝播してから有効点を増やす。その時点までの更新は新しい点に届かない。
        Push(index);
        Set(leaf < node.right ? index + 1 : node.right, leaf, value);
        Pull(index);
    }

    void Update(Index index, const Rectangle &rect, std::uint32_t a, std::uint32_t b) noexcept
    {
        auto &node = nodes_[index];
        if (node.count == 0 || Disjoint(node, rect))
        {
            return;
        }
        if (Contained(node, rect))
        {
            Apply(node, a, b);
            return;
        }
        Push(index);
        Update(index + 1, rect, a, b);
        Update(node.right, rect, a, b);
        Pull(index);
    }

    std::uint32_t Sum(Index index, const Rectangle &rect) noexcept
    {
        const auto &node = nodes_[index];
        if (node.count == 0 || Disjoint(node, rect))
        {
            return 0;
        }
        if (Contained(node, rect))
        {
            return node.sum;
        }
        Push(index);
        const auto left = Sum(index + 1, rect);
        return Add(left, Sum(node.right, rect));
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
