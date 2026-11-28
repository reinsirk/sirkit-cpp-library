// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Meldable_Heap.cpp
// 参考（左偏木）: C. A. Crane, Linear Lists and Priority Queues as Balanced Binary Trees, 1972.
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/meldable_heap.md
//
// API一覧
// MeldableHeap<T,Compare=std::less<T>>()/MeldableHeap(compare): 空の優先度付きキュー。既定は最大ヒープ。
// size()/empty()/top(): 要素数・空かどうか・最優先の要素へのconst参照。O(1)。
// push(value)/emplace(args...)/pop(): 要素の追加・構築して追加・最優先の要素の削除。O(log(n+1))。
// meld(other): 要素を取り込みotherを空にする。自分自身なら何もしない。O(log(n+m+1))。
// Compareはconstで呼べる狭義弱順序。結合する両方で同じ順序を使う。比較が例外を送出しても要素は保つ。
// clear()/デストラクタ: 全要素を再帰なしで破棄。O(n+1)時間・追加O(1)領域。全体の領域O(n)。
// コピー不可。Compareの対応するムーブ操作がnoexceptなら移動可。ムーブ元は空。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MELDABLE_HEAP_DEFINED
#define SIRKIT_MELDABLE_HEAP_DEFINED

template<class T, class Compare = std::less<T>>
class MeldableHeap
{
public:
    MeldableHeap() = default;

    explicit MeldableHeap(Compare compare) noexcept(std::is_nothrow_move_constructible_v<Compare>)
        : compare_(std::move(compare))
    {
    }

    MeldableHeap(const MeldableHeap &) = delete;
    MeldableHeap &operator=(const MeldableHeap &) = delete;

    MeldableHeap(MeldableHeap &&other) noexcept
        requires std::is_nothrow_move_constructible_v<Compare>
        : compare_(std::move(other.compare_)), root_(std::move(other.root_)), size_(std::exchange(other.size_, 0))
    {
    }

    MeldableHeap &operator=(MeldableHeap &&other) noexcept
        requires std::is_nothrow_move_assignable_v<Compare>
    {
        if (this != &other)
        {
            clear();
            compare_ = std::move(other.compare_);
            root_ = std::move(other.root_);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    ~MeldableHeap()
    {
        clear();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size_ == 0;
    }

    [[nodiscard]] const T &top() const
    {
        CheckNonEmpty();
        return root_->value;
    }

    void push(const T &value)
    {
        emplace(value);
    }

    void push(T &&value)
    {
        emplace(std::move(value));
    }

    template<class... Args>
    void emplace(Args &&...args)
    {
        auto node = std::make_unique<Node>(std::forward<Args>(args)...);
        MeldLinks(root_, node);
        ++size_;
    }

    void pop()
    {
        CheckNonEmpty();
        MeldLinks(root_->left, root_->right);
        Link old = std::move(root_);
        root_ = std::move(old->left);
        --size_;
    }

    void meld(MeldableHeap &other)
    {
        if (this == &other)
        {
            return;
        }
        MeldLinks(root_, other.root_);
        size_ += std::exchange(other.size_, 0);
    }

    void clear() noexcept
    {
        while (root_)
        {
            Link old = std::move(root_);
            if (old->left)
            {
                root_ = std::move(old->left);
                old->left = std::move(root_->right);
                root_->right = std::move(old);
            }
            else
            {
                root_ = std::move(old->right);
            }
        }
        size_ = 0;
    }

private:
    struct Node;
    using Link = std::unique_ptr<Node>;

    struct Node
    {
        T value;
        Link left, right;
        unsigned rank = 1;

        template<class... Args>
        explicit Node(Args &&...args) : value(std::forward<Args>(args)...)
        {
        }
    };

    [[no_unique_address]] Compare compare_{};
    Link root_;
    std::size_t size_ = 0;

    void CheckNonEmpty() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("MeldableHeap: empty heap");
        }
#endif
    }

    static unsigned Rank(const Link &link) noexcept
    {
        return link ? link->rank : 0;
    }

    void MeldLinks(Link &first, Link &second)
    {
        if (!second)
        {
            return;
        }
        if (!first)
        {
            first = std::move(second);
            return;
        }
        // 比較が終わってから再帰の戻り順に所有権を移す。
        if (std::invoke(std::as_const(compare_), std::as_const(first->value), std::as_const(second->value)))
        {
            MeldLinks(second->right, first);
            first = std::move(second);
        }
        else
        {
            MeldLinks(first->right, second);
        }
        if (Rank(first->left) < Rank(first->right))
        {
            first->left.swap(first->right);
        }
        first->rank = Rank(first->right) + 1;
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
