// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Ordered_Multiset.cpp
// 参考（多重集合の削除と順位照会）: https://atcoder.jp/contests/abc253/tasks/abc253_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/ordered_multiset.md
//
// API一覧
// OrderedMultiset<T,Compare>(compare={}): 空のAVL木。Compareはconstで呼べる狭義弱順序。
// size()/empty(): 重複込みの要素数(uint64_t)/空か。O(1)。
// insert(value,copies=1): 同値な要素をcopies個追加。総数の範囲超過はlength_error。
// erase(value,copies=1): 最大copies個削除し、削除数を返す。不足分は無視。count(value): 同値な要素数。
// count_less(value)/count_less_equal(value): 比較順でvalue未満/以下の要素数。
// kth(rank): 比較順の0始まり順位の値をconst T*で返す。範囲外はnullptr。
// ポインタはその値の最後の1個を消すまで有効。所有権は木にある。count以降と追加・削除はO(1+log(u+1))、uは異なるキー数。
// コピー不可。比較器が例外なく移動できる場合は移動可で、移動元は空。領域O(u+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ORDERED_MULTISET_DEFINED
#define SIRKIT_ORDERED_MULTISET_DEFINED

template <typename T, typename Compare = std::less<>>
class OrderedMultiset
{
    using Count = std::uint64_t;

    struct Node
    {
        T key;
        Count copies;
        Count total;
        int height = 1;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        Node(T &&value, Count times)
            : key(std::move(value)), copies(times), total(times)
        {
        }
    };

    using Link = std::unique_ptr<Node>;
    [[no_unique_address]] Compare compare_;
    Link root_;

    [[nodiscard]] bool less(const T &a, const T &b) const
    {
        return std::invoke(compare_, a, b);
    }

    static int height(const Link &link) noexcept
    {
        return link ? link->height : 0;
    }

    static Count total(const Link &link) noexcept
    {
        return link ? link->total : 0;
    }

    static void pull(Node &node) noexcept
    {
        node.height = 1 + std::max(height(node.left), height(node.right));
        node.total = total(node.left) + node.copies + total(node.right);
    }

    static void rotate_left(Link &link) noexcept
    {
        auto pivot = std::move(link->right);
        link->right = std::move(pivot->left);
        pull(*link);
        pivot->left = std::move(link);
        pull(*pivot);
        link = std::move(pivot);
    }

    static void rotate_right(Link &link) noexcept
    {
        auto pivot = std::move(link->left);
        link->left = std::move(pivot->right);
        pull(*link);
        pivot->right = std::move(link);
        pull(*pivot);
        link = std::move(pivot);
    }

    static void rebalance(Link &link) noexcept
    {
        pull(*link);
        const int difference = height(link->left) - height(link->right);
        if (difference > 1)
        {
            if (height(link->left->left) < height(link->left->right))
            {
                rotate_left(link->left);
            }
            rotate_right(link);
        }
        else if (difference < -1)
        {
            if (height(link->right->right) < height(link->right->left))
            {
                rotate_right(link->right);
            }
            rotate_left(link);
        }
    }

    void add(Link &link, T &value, Count copies)
    {
        if (!link)
        {
            link = std::make_unique<Node>(std::move(value), copies);
            return;
        }
        if (less(value, link->key))
        {
            add(link->left, value, copies);
        }
        else if (less(link->key, value))
        {
            add(link->right, value, copies);
        }
        else
        {
            link->copies += copies;
        }
        rebalance(link);
    }

    static Link detach_first(Link &link) noexcept
    {
        if (!link->left)
        {
            auto first = std::move(link);
            link = std::move(first->right);
            return first;
        }
        auto first = detach_first(link->left);
        rebalance(link);
        return first;
    }

    static void remove_node(Link &link) noexcept
    {
        if (!link->left)
        {
            auto right = std::move(link->right);
            link = std::move(right);
            return;
        }
        if (!link->right)
        {
            auto left = std::move(link->left);
            link = std::move(left);
            return;
        }
        // キーを移動せず、後継ノード自体を付け替える。
        auto replacement = detach_first(link->right);
        replacement->left = std::move(link->left);
        replacement->right = std::move(link->right);
        link = std::move(replacement);
        rebalance(link);
    }

    Count remove(Link &link, const T &value, Count copies)
    {
        if (!link)
        {
            return 0;
        }
        Count removed;
        if (less(value, link->key))
        {
            removed = remove(link->left, value, copies);
        }
        else if (less(link->key, value))
        {
            removed = remove(link->right, value, copies);
        }
        else
        {
            removed = std::min(copies, link->copies);
            if (removed == link->copies)
            {
                remove_node(link);
                return removed;
            }
            link->copies -= removed;
        }
        rebalance(link);
        return removed;
    }

    [[nodiscard]] Count rank(const T &value, bool include_equal) const
    {
        Count smaller = 0;
        const Node *node = root_.get();
        while (node)
        {
            const bool counted = include_equal ? !less(value, node->key) : less(node->key, value);
            if (counted)
            {
                smaller += total(node->left) + node->copies;
                node = node->right.get();
            }
            else
            {
                node = node->left.get();
            }
        }
        return smaller;
    }

public:
    explicit OrderedMultiset(Compare compare = {}) : compare_(std::move(compare))
    {
    }

    OrderedMultiset(const OrderedMultiset &) = delete;
    OrderedMultiset &operator=(const OrderedMultiset &) = delete;

    OrderedMultiset(OrderedMultiset &&) noexcept
        requires std::is_nothrow_move_constructible_v<Compare> = default;

    OrderedMultiset &operator=(OrderedMultiset &&other) noexcept
        requires std::is_nothrow_move_assignable_v<Compare>
    {
        if (this != &other)
        {
            compare_ = std::move(other.compare_);
            root_ = std::move(other.root_);
        }
        return *this;
    }

    [[nodiscard]] Count size() const noexcept
    {
        return total(root_);
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return !root_;
    }

    void insert(T value, Count copies = 1)
    {
        if (copies == 0)
        {
            return;
        }
        if (copies > std::numeric_limits<Count>::max() - size())
        {
            throw std::length_error("OrderedMultiset: too many elements");
        }
        add(root_, value, copies);
    }

    Count erase(const T &value, Count copies = 1)
    {
        return copies == 0 ? 0 : remove(root_, value, copies);
    }

    [[nodiscard]] Count count(const T &value) const
    {
        const Node *node = root_.get();
        while (node)
        {
            if (less(value, node->key))
            {
                node = node->left.get();
            }
            else if (less(node->key, value))
            {
                node = node->right.get();
            }
            else
            {
                return node->copies;
            }
        }
        return 0;
    }

    [[nodiscard]] Count count_less(const T &value) const
    {
        return rank(value, false);
    }

    [[nodiscard]] Count count_less_equal(const T &value) const
    {
        return rank(value, true);
    }

    [[nodiscard]] const T *kth(Count rank) const noexcept
    {
        const Node *node = root_.get();
        while (node)
        {
            const auto before = total(node->left);
            if (rank < before)
            {
                node = node->left.get();
            }
            else
            {
                rank -= before;
                if (rank < node->copies)
                {
                    return std::addressof(node->key);
                }
                rank -= node->copies;
                node = node->right.get();
            }
        }
        return nullptr;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
