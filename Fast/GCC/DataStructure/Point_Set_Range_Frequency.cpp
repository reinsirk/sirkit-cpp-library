// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Point_Set_Range_Frequency.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/point_set_range_frequency.md
//
// API一覧
// PointSetRangeFrequency<T>(values): 整数列で初期化する。O(n log(n+1))時間、O(n)領域。
// size(): 要素数を返す。O(1)。
// get(i): 添字iの値を返す。O(1)。
// set(i,value): 添字iの値を置き換える。O(log(n+1))。
// count(l,r,value): 半開区間[l,r)でのvalueの出現回数を返す。O(log(n+1))。
// 添字は0始まり。初期列にない値も指定できる。空区間の答えは0。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_POINT_SET_RANGE_FREQUENCY_DEFINED
#define SIRKIT_POINT_SET_RANGE_FREQUENCY_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckPointSetRangeFrequencySize(std::size_t size, std::size_t limit)
{
    if (size > limit)
    {
        throw std::length_error("PointSetRangeFrequency: too many values");
    }
}
} // namespace detail

template <std::integral T = std::int64_t>
class PointSetRangeFrequency
{
public:
    explicit PointSetRangeFrequency(std::span<const T> values)
    {
        // 添字0は空ノード。添字i+1は列の要素iに対応する。
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckPointSetRangeFrequencySize(values.size(), std::numeric_limits<Index>::max() - 1);
        nodes_.reserve(values.size() + 1);
        nodes_.emplace_back();
        for (const auto value : values)
        {
            nodes_.emplace_back(value);
        }

        std::vector<Index> order(values.size());
        std::iota(order.begin(), order.end(), Index{1});
        std::sort(order.begin(), order.end(), [this](Index a, Index b)
                  {
                      return Less(a, b);
                  });
        root_ = Build(order);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return nodes_.size() - static_cast<std::size_t>(!nodes_.empty());
    }

    [[nodiscard]] T get(std::size_t i) const
    {
        CheckIndex(i);
        return nodes_[i + 1].value;
    }

    void set(std::size_t i, T value)
    {
        CheckIndex(i);
        const auto item = static_cast<Index>(i + 1);
        auto &node = nodes_[item];
        if (node.value == value)
        {
            return;
        }

        root_ = Erase(root_, item);
        node = Node{value, 0, 0, 1, 1};
        root_ = Insert(root_, item);
    }

    [[nodiscard]] std::size_t count(std::size_t left, std::size_t right, T value) const
    {
        if (left == right)
        {
            return 0;
        }
        return Rank(value, right) - Rank(value, left);
    }

private:
    using Index = std::uint32_t;

    struct Node
    {
        T value{};
        Index left = 0, right = 0, count = 0, height = 0;
    };

    std::vector<Node> nodes_;
    Index root_ = 0;

    void CheckIndex(std::size_t i) const
    {
        (void)i;
    }

    bool Less(Index a, Index b) const
    {
        const auto a_value = nodes_[a].value;
        const auto b_value = nodes_[b].value;
        return a_value < b_value || (a_value == b_value && a < b);
    }

    void Pull(Index root)
    {
        auto &node = nodes_[root];
        const auto &left = nodes_[node.left];
        const auto &right = nodes_[node.right];
        node.count = left.count + right.count + 1;
        node.height = std::max(left.height, right.height) + 1;
    }

    Index RotateLeft(Index root)
    {
        auto &node = nodes_[root];
        const auto pivot = node.right;
        node.right = nodes_[pivot].left;
        nodes_[pivot].left = root;
        Pull(root);
        Pull(pivot);
        return pivot;
    }

    Index RotateRight(Index root)
    {
        auto &node = nodes_[root];
        const auto pivot = node.left;
        node.left = nodes_[pivot].right;
        nodes_[pivot].right = root;
        Pull(root);
        Pull(pivot);
        return pivot;
    }

    Index Balance(Index root)
    {
        Pull(root);
        auto &node = nodes_[root];
        const auto &left = nodes_[node.left];
        const auto &right = nodes_[node.right];
        if (left.height > right.height + 1)
        {
            if (nodes_[left.right].height > nodes_[left.left].height)
            {
                node.left = RotateLeft(node.left);
            }
            return RotateRight(root);
        }
        if (right.height > left.height + 1)
        {
            if (nodes_[right.left].height > nodes_[right.right].height)
            {
                node.right = RotateRight(node.right);
            }
            return RotateLeft(root);
        }
        return root;
    }

    Index Build(std::span<const Index> order)
    {
        if (order.empty())
        {
            return 0;
        }
        const auto middle = order.size() / 2;
        const auto root = order[middle];
        nodes_[root].left = Build(order.first(middle));
        nodes_[root].right = Build(order.subspan(middle + 1));
        Pull(root);
        return root;
    }

    Index Insert(Index root, Index item)
    {
        if (root == 0)
        {
            return item;
        }
        auto &node = nodes_[root];
        if (Less(item, root))
        {
            node.left = Insert(node.left, item);
        }
        else
        {
            node.right = Insert(node.right, item);
        }
        return Balance(root);
    }

    Index ExtractMinimum(Index root, Index &minimum)
    {
        auto &node = nodes_[root];
        if (node.left == 0)
        {
            minimum = root;
            return node.right;
        }
        node.left = ExtractMinimum(node.left, minimum);
        return Balance(root);
    }

    Index Erase(Index root, Index item)
    {
        auto &node = nodes_[root];
        if (root == item)
        {
            if (node.left == 0)
            {
                return node.right;
            }
            if (node.right == 0)
            {
                return node.left;
            }

            Index successor = 0;
            const auto new_right = ExtractMinimum(node.right, successor);
            nodes_[successor].left = node.left;
            nodes_[successor].right = new_right;
            return Balance(successor);
        }
        if (Less(item, root))
        {
            node.left = Erase(node.left, item);
        }
        else
        {
            node.right = Erase(node.right, item);
        }
        return Balance(root);
    }

    // (value, bound) より小さい (値,添字) の組を数える。
    std::size_t Rank(T value, std::size_t bound) const
    {
        std::size_t result = 0;
        auto root = root_;
        while (root != 0)
        {
            const auto &node = nodes_[root];
            if (node.value < value || (node.value == value && root - 1 < bound))
            {
                result += nodes_[node.left].count + 1;
                root = node.right;
            }
            else
            {
                root = node.left;
            }
        }
        return result;
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
