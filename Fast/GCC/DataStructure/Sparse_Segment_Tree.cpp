// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Sparse_Segment_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_segment_tree.md
//
// API一覧
// SparseSegmentTree<S,op,e>(n=0): 長さnの全要素を単位元で初期化。O(1)時間・領域。
// set(p,value): pを代入。等値比較できる型では単位元の代入で点を削除。償却O(1+log(n+1))。
// erase(p): pを単位元に戻し、点を削除したときtrue。O(1+log(n+1))。
// get(p): pの値。未登録位置は単位元。O(1+log(n+1))。
// prod(l,r): 0始まり半開区間[l,r)を左から右へ集約。空区間は単位元。O(1+log(n+1))。
// size()/all_prod()/stored_points(): 長さ・全体の集約・保存点数。各O(1)。
// 長さ・端点はuint64_t。領域は同時に保存した最大点数Pに対してO(P+1)。
// opとSのコピー・ムーブの構築・代入はnoexcept。演算の途中の値もSで表現できること。単位元の自動削除に使う比較は設定前に行う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_SEGMENT_TREE_DEFINED
#define SIRKIT_SPARSE_SEGMENT_TREE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t NextSparseSegmentTreeCapacity(std::size_t used, std::size_t capacity, std::size_t extra,
                                                 std::size_t limit)
{
    if (extra > limit - used)
    {
        throw std::length_error("SparseSegmentTree: too many nodes");
    }
    if (used + extra <= capacity)
    {
        return capacity;
    }
    return std::max(used + extra, capacity + std::min(capacity, limit - capacity));
}
}

template <class S, auto op, auto e>
class SparseSegmentTree
{
    static_assert(std::is_nothrow_invocable_r_v<S, decltype(op), const S &, const S &>);
    static_assert(std::is_nothrow_copy_constructible_v<S> && std::is_nothrow_move_constructible_v<S> &&
                  std::is_nothrow_copy_assignable_v<S> && std::is_nothrow_move_assignable_v<S>);

public:
    explicit SparseSegmentTree(std::uint64_t n = 0) : length_(n), identity_(std::invoke(e))
    {
    }

    SparseSegmentTree(const SparseSegmentTree &) = default;

    SparseSegmentTree(SparseSegmentTree &&other) noexcept
        : nodes_(std::move(other.nodes_)), length_(std::exchange(other.length_, 0)), identity_(other.identity_),
          root_(std::exchange(other.root_, 0)), free_(std::exchange(other.free_, 0)),
          points_(std::exchange(other.points_, 0)), free_count_(std::exchange(other.free_count_, 0))
    {
        other.nodes_.clear();
    }

    SparseSegmentTree &operator=(SparseSegmentTree other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(length_, other.length_);
        std::swap(identity_, other.identity_);
        std::swap(root_, other.root_);
        std::swap(free_, other.free_);
        std::swap(points_, other.points_);
        std::swap(free_count_, other.free_count_);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return length_;
    }

    [[nodiscard]] std::size_t stored_points() const noexcept
    {
        return points_;
    }

    [[nodiscard]] S all_prod() const noexcept
    {
        return root_ != 0 ? At(root_).aggregate : identity_;
    }

    void set(std::uint64_t position, S value)
    {
        CheckPosition(position);
        if constexpr (std::equality_comparable<S>)
        {
            if (std::as_const(value) == std::as_const(identity_))
            {
                root_ = Erase(root_, position);
                return;
            }
        }
        root_ = Set(root_, position, value);
    }

    bool erase(std::uint64_t position)
    {
        CheckPosition(position);
        const auto old_points = points_;
        root_ = Erase(root_, position);
        return points_ != old_points;
    }

    [[nodiscard]] S get(std::uint64_t position) const
    {
        CheckPosition(position);
        auto node = root_;
        while (node != 0)
        {
            const auto &current = At(node);
            if (current.bits == 0)
            {
                return current.pivot == position ? current.aggregate : identity_;
            }
            node = position < current.pivot ? current.left : current.right;
        }
        return identity_;
    }

    [[nodiscard]] S prod(std::uint64_t left, std::uint64_t right) const
    {
        CheckRange(left, right);
        if (left == right || root_ == 0)
        {
            return identity_;
        }
        if (left == 0 && right == length_)
        {
            return At(root_).aggregate;
        }
        return Fold(root_, left, right);
    }

private:
    using Index = std::uint32_t;

    struct Node
    {
        // 葉のpivotは位置。分岐では右半分の先頭位置を表す。
        std::uint64_t pivot = 0;
        Index left = 0, right = 0;
        S aggregate;
        unsigned bits = 0;
    };

    std::vector<Node> nodes_;
    std::uint64_t length_;
    S identity_;
    Index root_ = 0, free_ = 0;
    std::size_t points_ = 0, free_count_ = 0;

    void CheckPosition([[maybe_unused]] std::uint64_t position) const
    {
    }

    void CheckRange([[maybe_unused]] std::uint64_t left, [[maybe_unused]] std::uint64_t right) const
    {
    }

    void Reserve(std::size_t extra)
    {
        const auto slots = extra - std::min(extra, free_count_);
        const auto limit = std::min<std::size_t>(std::numeric_limits<Index>::max(), nodes_.max_size());
        nodes_.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NextSparseSegmentTreeCapacity(
            nodes_.size(), nodes_.capacity(), slots, limit));
    }

    [[nodiscard]] const Node &At(Index index) const noexcept
    {
        return nodes_[index - 1];
    }

    Node &At(Index index) noexcept
    {
        return nodes_[index - 1];
    }

    Index Allocate(Node node) noexcept
    {
        if (free_ == 0)
        {
            nodes_.push_back(std::move(node));
            return static_cast<Index>(nodes_.size());
        }
        const auto index = free_;
        free_ = At(index).left;
        --free_count_;
        At(index) = std::move(node);
        return index;
    }

    void Release(Index index) noexcept
    {
        auto &node = At(index);
        node.aggregate = identity_;
        node.left = free_;
        free_ = index;
        ++free_count_;
    }

    static S Product(const S &left, const S &right) noexcept
    {
        return std::invoke(op, left, right);
    }

    void Pull(Index index) noexcept
    {
        auto &node = At(index);
        node.aggregate = Product(At(node.left).aggregate, At(node.right).aggregate);
    }

    Index Set(Index index, std::uint64_t position, const S &value)
    {
        if (index == 0)
        {
            Reserve(1);
            ++points_;
            return Allocate({position, 0, 0, value});
        }
        const auto pivot = At(index).pivot;
        const auto bits = static_cast<unsigned>(std::bit_width(pivot ^ position));
        if (bits > At(index).bits)
        {
            const auto half = std::uint64_t{1} << (bits - 1);
            const auto split = (position | half) & ~(half - 1);
            Reserve(2);
            ++points_;
            const auto leaf = Allocate({position, 0, 0, value});
            const auto left = position < split ? leaf : index;
            const auto right = position < split ? index : leaf;
            return Allocate({split, left, right, Product(At(left).aggregate, At(right).aggregate), bits});
        }
        if (At(index).bits == 0)
        {
            At(index).aggregate = value;
            return index;
        }

        // 挿入でvectorが移動し得るため、子の添字を更新してから親を取り直す。
        if (position < pivot)
        {
            const auto child = Set(At(index).left, position, value);
            At(index).left = child;
        }
        else
        {
            const auto child = Set(At(index).right, position, value);
            At(index).right = child;
        }
        Pull(index);
        return index;
    }

    Index Erase(Index index, std::uint64_t position) noexcept
    {
        if (index == 0)
        {
            return 0;
        }
        auto &node = At(index);
        if (node.bits == 0)
        {
            if (node.pivot != position)
            {
                return index;
            }
            Release(index);
            --points_;
            return 0;
        }
        auto &child = position < node.pivot ? node.left : node.right;
        child = Erase(child, position);
        if (child == 0)
        {
            const auto sibling = node.left == 0 ? node.right : node.left;
            Release(index);
            return sibling;
        }
        Pull(index);
        return index;
    }

    [[nodiscard]] S Fold(Index index, std::uint64_t left, std::uint64_t right) const noexcept
    {
        const auto &node = At(index);
        if (node.bits == 0)
        {
            return left <= node.pivot && node.pivot < right ? node.aggregate : identity_;
        }
        // 右端を含む形にして、64ビット全体にまたがる区間でも加算であふれない。
        const auto half = std::uint64_t{1} << (node.bits - 1);
        const auto first = node.pivot - half;
        const auto last = node.pivot | (half - 1);
        if (right <= first || last < left)
        {
            return identity_;
        }
        if (left <= first && last < right)
        {
            return node.aggregate;
        }
        if (right <= node.pivot)
        {
            return Fold(node.left, left, right);
        }
        if (node.pivot <= left)
        {
            return Fold(node.right, left, right);
        }
        return Product(Fold(node.left, left, right), Fold(node.right, left, right));
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
