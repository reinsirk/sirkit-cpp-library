// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Persistent_Affine_Sequence.cpp
// 参考: https://img.atcoder.jp/arc030/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_affine_sequence.md
//
// API一覧
// PersistentAffineSequence<T>(values): 数列を構築。O(n)時間・領域。空列はPersistentAffineSequence<T>()。
// size()/empty()/total(): 要素数、空判定、全体和。O(1)。sum(l,r): [l,r)の和。O(log(n+1))、ノード追加なし。
// affine(l,r,a,b): [l,r)の各xをa*x+bにする。reverse(l,r): [l,r)を反転する。
// slice(l,r): [l,r)だけを取り出す。split(k): [0,k)と[k,n)の組に分ける。
// concat(other): 後ろに別の列を連結。replace(l,r,other): [l,r)を別の列で置き換える。長さの異なる列も可。
// copy_from(l,r,other): 別の列の同じ添字の範囲で[l,r)を置き換える。
// 更新・切り出しは元の列を保存し、新しい列を返す。時間・新規ノード数O(log(n+m+1))。mは連結・置換する列の長さ。
// コピーはノードを共有しO(1)。最後の参照を失ったノードは解放。移動元は空。
// 0始まりの半開区間。要素数はINT_MAX以下。T{0}, T{1}, 加算、乗算が使え、Tで要素数と演算の中間値を表現できること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_AFFINE_SEQUENCE_DEFINED
#define SIRKIT_PERSISTENT_AFFINE_SEQUENCE_DEFINED

template <class T>
class PersistentAffineSequence
{
    struct Affine
    {
        T multiplier = T(1);
        T increment = T(0);

        T Sum(const T &value, std::size_t count) const
        {
            return multiplier * value + increment * T(count);
        }
    };

    struct Node;
    using Pointer = std::shared_ptr<const Node>;

    struct Node
    {
        Pointer left;
        Pointer right;
        T sum;
        std::size_t length = 1;
        int height = 1;
        Affine affine;
        bool has_affine = false;
        bool reverse = false;
    };

public:
    PersistentAffineSequence() = default;

    explicit PersistentAffineSequence(std::span<const T> values)
        : root_(BuildChecked(values))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return root_ ? root_->length : 0;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return !root_;
    }

    [[nodiscard]] T total() const
    {
        return root_ ? root_->sum : T(0);
    }

    [[nodiscard]] T sum(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        return left == right ? T(0) : RangeSum(root_, left, right, false);
    }

    [[nodiscard]] PersistentAffineSequence affine(std::size_t left, std::size_t right, const T &a, const T &b) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return *this;
        }
        return PersistentAffineSequence(TransformRange(root_, left, right, {a, b}));
    }

    [[nodiscard]] PersistentAffineSequence reverse(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return *this;
        }
        auto [prefix, suffix] = Split(root_, left);
        auto [middle, tail] = Split(suffix, right - left);
        return PersistentAffineSequence(Join(Join(prefix, Reverse(middle)), tail));
    }

    [[nodiscard]] PersistentAffineSequence slice(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return {};
        }
        return PersistentAffineSequence(Slice(root_, left, right));
    }

    [[nodiscard]] std::pair<PersistentAffineSequence, PersistentAffineSequence> split(std::size_t position) const
    {
        CheckRange(0, position);
        auto [prefix, suffix] = Split(root_, position);
        return {PersistentAffineSequence(std::move(prefix)), PersistentAffineSequence(std::move(suffix))};
    }

    [[nodiscard]] PersistentAffineSequence concat(const PersistentAffineSequence &other) const
    {
        CheckSize(size() + other.size());
        return PersistentAffineSequence(Join(root_, other.root_));
    }

    [[nodiscard]] PersistentAffineSequence replace(std::size_t left, std::size_t right, const PersistentAffineSequence &other) const
    {
        CheckRange(left, right);
        CheckSize(size() - (right - left) + other.size());
        return PersistentAffineSequence(Replace(root_, left, right, other.root_));
    }

    [[nodiscard]] PersistentAffineSequence copy_from(std::size_t left, std::size_t right, const PersistentAffineSequence &other) const
    {
        CheckRange(left, right);
        other.CheckRange(left, right);
        if (left == right)
        {
            return *this;
        }
        if (size() != other.size())
        {
            return PersistentAffineSequence(Replace(root_, left, right, Slice(other.root_, left, right)));
        }
        return PersistentAffineSequence(CopyAligned(root_, other.root_, left, right));
    }

private:
    Pointer root_;

    explicit PersistentAffineSequence(Pointer root) : root_(std::move(root))
    {
    }

    static void CheckSize(std::size_t length)
    {
        if (length > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("PersistentAffineSequence: too many elements");
        }
    }

    void CheckRange([[maybe_unused]] std::size_t left, [[maybe_unused]] std::size_t right) const
    {
    }

    static Pointer Leaf(const T &value)
    {
        return std::make_shared<const Node>(Node{nullptr, nullptr, value, 1, 1, {}, false, false});
    }

    static Pointer Branch(const Pointer &left, const Pointer &right)
    {
        return std::make_shared<const Node>(Node{left, right, left->sum + right->sum, left->length + right->length, 1 + std::max(left->height, right->height), {}, false, false});
    }

    static Pointer Build(std::span<const T> values)
    {
        if (values.size() == 1)
        {
            return Leaf(values[0]);
        }
        const auto half = values.size() / 2;
        return Branch(Build(values.first(half)), Build(values.subspan(half)));
    }

    static Pointer BuildChecked(std::span<const T> values)
    {
        CheckSize(values.size());
        return values.empty() ? nullptr : Build(values);
    }

    static Pointer Transform(const Pointer &node, const Affine &f)
    {
        auto result = std::make_shared<Node>(*node);
        result->sum = f.Sum(result->sum, result->length);
        if (result->height > 1)
        {
            result->affine = {f.multiplier * result->affine.multiplier,
                              f.multiplier * result->affine.increment + f.increment};
            result->has_affine = true;
        }
        return result;
    }

    static Pointer Reverse(const Pointer &node)
    {
        if (node->height == 1)
        {
            return node;
        }
        auto result = std::make_shared<Node>(*node);
        std::swap(result->left, result->right);
        result->reverse = !result->reverse;
        return result;
    }

    // Returns the logical children, propagating only the retained parent's tags.
    static std::pair<Pointer, Pointer> Children(const Pointer &node)
    {
        auto left = node->left;
        auto right = node->right;
        if (node->reverse)
        {
            left = Reverse(left);
            right = Reverse(right);
        }
        if (node->has_affine)
        {
            left = Transform(left, node->affine);
            right = Transform(right, node->affine);
        }
        return {std::move(left), std::move(right)};
    }

    static Pointer Balance(const Pointer &left, const Pointer &right)
    {
        if (left->height > right->height + 1)
        {
            auto [outer, inner] = Children(left);
            if (outer->height >= inner->height)
            {
                return Branch(outer, Branch(inner, right));
            }
            auto [a, b] = Children(inner);
            return Branch(Branch(outer, a), Branch(b, right));
        }
        if (right->height > left->height + 1)
        {
            auto [inner, outer] = Children(right);
            if (outer->height >= inner->height)
            {
                return Branch(Branch(left, inner), outer);
            }
            auto [a, b] = Children(inner);
            return Branch(Branch(left, a), Branch(b, outer));
        }
        return Branch(left, right);
    }

    static Pointer Join(const Pointer &left, const Pointer &right)
    {
        if (!left)
        {
            return right;
        }
        if (!right)
        {
            return left;
        }
        if (left->height > right->height + 1)
        {
            auto [a, b] = Children(left);
            return Balance(a, Join(b, right));
        }
        if (right->height > left->height + 1)
        {
            auto [a, b] = Children(right);
            return Balance(Join(left, a), b);
        }
        return Branch(left, right);
    }

    static std::pair<Pointer, Pointer> Split(const Pointer &node, std::size_t size)
    {
        if (size == 0)
        {
            return {nullptr, node};
        }
        if (size == node->length)
        {
            return {node, nullptr};
        }
        auto [left, right] = Children(node);
        const auto left_size = left->length;
        if (size < left_size)
        {
            auto [a, b] = Split(left, size);
            return {std::move(a), Join(b, right)};
        }
        if (size > left_size)
        {
            auto [a, b] = Split(right, size - left_size);
            return {Join(left, a), std::move(b)};
        }
        return {std::move(left), std::move(right)};
    }

    static Pointer Slice(const Pointer &node, std::size_t left, std::size_t right)
    {
        const auto suffix = Split(node, left).second;
        return Split(suffix, right - left).first;
    }

    static Pointer Replace(const Pointer &node, std::size_t left, std::size_t right, const Pointer &other)
    {
        auto [prefix, suffix] = Split(node, left);
        const auto tail = Split(suffix, right - left).second;
        return Join(Join(prefix, other), tail);
    }

    static Pointer TransformRange(const Pointer &node, std::size_t left, std::size_t right, const Affine &f)
    {
        if (left == 0 && right == node->length)
        {
            return Transform(node, f);
        }
        auto [a, b] = Children(node);
        const auto mid = a->length;
        if (right <= mid)
        {
            return Branch(TransformRange(a, left, right, f), b);
        }
        if (left >= mid)
        {
            return Branch(a, TransformRange(b, left - mid, right - mid, f));
        }
        return Branch(TransformRange(a, left, mid, f), TransformRange(b, 0, right - mid, f));
    }

    static Pointer CopyAligned(const Pointer &target, const Pointer &source, std::size_t left, std::size_t right)
    {
        if (target == source)
        {
            return target;
        }
        if (left == 0 && right == target->length)
        {
            return source;
        }
        const auto mid = target->left->length;
        if (mid != source->left->length)
        {
            return Replace(target, left, right, Slice(source, left, right));
        }
        auto [dest_a, dest_b] = Children(target);
        const auto [src_a, src_b] = Children(source);
        if (right <= mid)
        {
            return Branch(CopyAligned(dest_a, src_a, left, right), dest_b);
        }
        if (left >= mid)
        {
            return Branch(dest_a, CopyAligned(dest_b, src_b, left - mid, right - mid));
        }
        return Branch(CopyAligned(dest_a, src_a, left, mid), CopyAligned(dest_b, src_b, 0, right - mid));
    }

    // Sum directly in the shared tree; querying a slice allocates no nodes.
    static T RangeSum(const Pointer &node, std::size_t left, std::size_t right, bool flip)
    {
        if (left == 0 && right == node->length)
        {
            return node->sum;
        }
        const auto &a = flip ? node->right : node->left;
        const auto &b = flip ? node->left : node->right;
        const bool child_flip = flip != node->reverse;
        const auto mid = a->length;
        const T value = right <= mid
                            ? RangeSum(a, left, right, child_flip)
                        : left >= mid
                            ? RangeSum(b, left - mid, right - mid, child_flip)
                            : RangeSum(a, left, mid, child_flip) + RangeSum(b, 0, right - mid, child_flip);
        return node->has_affine ? node->affine.Sum(value, right - left) : value;
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
