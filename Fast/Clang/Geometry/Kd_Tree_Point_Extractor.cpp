// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Kd_Tree_Point_Extractor.cpp
// 数学的参考: https://atcoder.jp/contests/awc0163/editorial/25858
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/kd_tree_point_extractor.md
//
// API一覧
// KdTreePointExtractor(points): 2次元整数点を前処理する。平均O(n log(n+1))時間、O(n)領域。
// Point: std::array<std::int32_t,2>。入力順の0始まりのIDを使う。同じ座標にも別々のIDが付く。
// size(): まだ取り出していない点の数。O(1)。
// take_in_circle(center,radius,output): 閉円内の点のIDをoutputの末尾に追加し、木から取り除く。radiusはuint32_t。
// take_in_rectangle(lower,upper,output): 閉長方形内の点を同様に取り出す。各軸でlower<=upper。
// 出力順は未規定。照会は訪れた節点数と出力数に比例し最悪O(n)。再確保時はoutputの既存要素数も加算。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_KD_TREE_POINT_EXTRACTOR_DEFINED
#define SIRKIT_KD_TREE_POINT_EXTRACTOR_DEFINED

class KdTreePointExtractor
{
public:
    using Point = std::array<std::int32_t, 2>;

    explicit KdTreePointExtractor(const std::vector<Point> &points)
    {
        nodes_.reserve(points.size());
        for (std::size_t id = 0; id < points.size(); ++id)
        {
            nodes_.push_back(Node{.point = points[id], .id = id});
        }
        build(0, nodes_.size());
    }

    std::size_t size() const
    {
        return alive(root());
    }

    void take_in_circle(Point center, std::uint32_t radius,
                        std::vector<std::size_t> &output)
    {
        extract(Circle{center, square(radius)}, output);
    }

    void take_in_rectangle(Point lower, Point upper,
                           std::vector<std::size_t> &output)
    {
        extract(Box{lower, upper}, output);
    }

private:
    static constexpr auto kNoNode = std::numeric_limits<std::size_t>::max();

    struct Box
    {
        Point lower;
        Point upper;

        bool contains(Point p) const
        {
            return lower[0] <= p[0] && p[0] <= upper[0] && lower[1] <= p[1] &&
                   p[1] <= upper[1];
        }

        bool excludes(const Box &box) const
        {
            return upper[0] < box.lower[0] || box.upper[0] < lower[0] ||
                   upper[1] < box.lower[1] || box.upper[1] < lower[1];
        }

        bool covers(const Box &box) const
        {
            return contains(box.lower) && contains(box.upper);
        }
    };

    static std::uint64_t square(std::int64_t delta)
    {
        const auto unsigned_delta = static_cast<std::uint64_t>(delta);
        return unsigned_delta * unsigned_delta;
    }

    struct Circle
    {
        Point center;
        std::uint64_t radius_squared;

        bool inside_squared(std::uint64_t dx_squared,
                            std::uint64_t dy_squared) const
        {
            // 一軸の距離の二乗は64ビットに収まる。和を作らずに加算の桁あふれを防ぐ。
            return dx_squared <= radius_squared &&
                   dy_squared <= radius_squared - dx_squared;
        }

        bool contains(Point p) const
        {
            return inside_squared(
                square(static_cast<std::int64_t>(p[0]) - center[0]),
                square(static_cast<std::int64_t>(p[1]) - center[1]));
        }

        bool excludes(const Box &box) const
        {
            const Point closest{
                std::clamp(center[0], box.lower[0], box.upper[0]),
                std::clamp(center[1], box.lower[1], box.upper[1]),
            };
            return !contains(closest);
        }

        bool covers(const Box &box) const
        {
            const auto max_squared = [&](std::size_t axis)
            {
                return std::max(
                    square(static_cast<std::int64_t>(box.lower[axis]) - center[axis]),
                    square(static_cast<std::int64_t>(box.upper[axis]) - center[axis]));
            };
            return inside_squared(max_squared(0), max_squared(1));
        }
    };

    struct Node
    {
        Point point;
        std::size_t id;
        Box bounds{};
        std::size_t left = kNoNode, right = kNoNode, remaining = 1;
        bool present = true;
    };

    std::vector<Node> nodes_;
    std::size_t root() const
    {
        return nodes_.empty() ? kNoNode : nodes_.size() / 2;
    }

    std::size_t build(std::size_t first, std::size_t last)
    {
        if (first == last)
        {
            return kNoNode;
        }
        Box bounds{nodes_[first].point, nodes_[first].point};
        for (std::size_t i = first + 1; i < last; ++i)
        {
            for (std::size_t axis = 0; axis < 2; ++axis)
            {
                bounds.lower[axis] =
                    std::min(bounds.lower[axis], nodes_[i].point[axis]);
                bounds.upper[axis] =
                    std::max(bounds.upper[axis], nodes_[i].point[axis]);
            }
        }
        const auto width =
            static_cast<std::int64_t>(bounds.upper[0]) - bounds.lower[0];
        const auto height =
            static_cast<std::int64_t>(bounds.upper[1]) - bounds.lower[1];
        const std::size_t axis = height > width;
        const std::size_t mid = first + (last - first) / 2;
        std::nth_element(nodes_.begin() + first, nodes_.begin() + mid,
                         nodes_.begin() + last,
                         [axis](const Node &a, const Node &b)
                         {
                             return a.point[axis] < b.point[axis];
                         });
        Node &node = nodes_[mid];
        node.bounds = bounds;
        node.left = build(first, mid);
        node.right = build(mid + 1, last);
        node.remaining = last - first;
        return mid;
    }

    std::size_t alive(std::size_t id) const
    {
        return id == kNoNode ? 0 : nodes_[id].remaining;
    }

    template <class Shape>
    std::size_t count(std::size_t id, const Shape &shape) const
    {
        if (alive(id) == 0 || shape.excludes(nodes_[id].bounds))
        {
            return 0;
        }
        const auto &node = nodes_[id];
        if (shape.covers(node.bounds))
        {
            return node.remaining;
        }
        return std::size_t(node.present && shape.contains(node.point)) +
               count(node.left, shape) + count(node.right, shape);
    }

    template <class Shape>
    void extract(const Shape &shape, std::vector<std::size_t> &output)
    {
        const auto id = root();
        const auto capacity = output.capacity();
        if (alive(id) > capacity - output.size())
        {
            const auto required = output.size() + count(id, shape);
            if (required > capacity)
            {
                const auto growth = std::min(capacity / 2, output.max_size() - capacity);
                output.reserve(std::max(required, capacity + growth));
            }
        }
        take(id, shape, output);
    }

    void take_all(std::size_t id, std::vector<std::size_t> &output)
    {
        if (alive(id) == 0)
        {
            return;
        }
        Node &node = nodes_[id];
        if (node.present)
        {
            output.push_back(node.id);
        }
        take_all(node.left, output);
        take_all(node.right, output);
        node.remaining = 0;
    }

    template <class Shape>
    void take(std::size_t id, const Shape &shape,
              std::vector<std::size_t> &output)
    {
        if (alive(id) == 0)
        {
            return;
        }
        Node &node = nodes_[id];
        if (shape.excludes(node.bounds))
        {
            return;
        }
        if (shape.covers(node.bounds))
        {
            take_all(id, output);
            return;
        }
        if (node.present && shape.contains(node.point))
        {
            output.push_back(node.id);
            node.present = false;
        }
        take(node.left, shape, output);
        take(node.right, shape, output);
        node.remaining =
            alive(node.left) + alive(node.right) + std::size_t(node.present);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
