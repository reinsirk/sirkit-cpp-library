// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Deque_Aggregation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/deque_aggregation.md
//
// API一覧
// DequeAggregation<T,Op>(identity,op=Op{}): 空。Opはconst呼び出し可能な結合演算。O(1)。
// push_front(value)/push_back(value): 両端への追加。償却O(1)。
// pop_front()/pop_back(): 両端の削除。片側が空なら半分を移す。償却O(1)、最悪O(n)。
// front()/back(): 両端をconst参照で返す。all_prod(): 列順の積。空ならidentity。各O(1)。
// size()/empty(): 要素数と空判定。O(1)。clear(): 全削除。O(n)。
// swap(other): 演算と列の交換。O(1)。代入・交換にはTとOpの非送出swapが必要。
// front/back/popは非空が前提。入力検査有効時は空でout_of_range。
// 要素と演算はコピー可能。領域は過去最大要素数Mに対してO(M+1)。
// 両端の参照は更新・clear・交換・移動で無効になり得る。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DEQUE_AGGREGATION_DEFINED
#define SIRKIT_DEQUE_AGGREGATION_DEFINED

template <class T, class Op>
class DequeAggregation
{
    static_assert(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<Op>);
    static_assert(std::is_invocable_r_v<T, const Op &, const T &, const T &>);
    static constexpr bool kInPlaceRebalance =
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_invocable_r_v<T, const Op &, const T &, const T &>;

    struct Entry
    {
        T value, product;
    };

public:
    explicit DequeAggregation(const T &identity, Op op = Op{})
        : op_(std::move_if_noexcept(op)), identity_(identity)
    {
    }

    DequeAggregation(const DequeAggregation &) = default;

    DequeAggregation(DequeAggregation &&other) noexcept(
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_constructible_v<Op>)
        : op_(other.op_), identity_(other.identity_), left_(std::move(other.left_)), right_(std::move(other.right_))
    {
        other.clear();
    }

    DequeAggregation &operator=(DequeAggregation other) noexcept
        requires std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<Op>
    {
        swap(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        // EntryはTを2個含むため、左右の合計もsize_tに収まる。
        return left_.size() + right_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return left_.empty() && right_.empty();
    }

    void push_front(const T &value)
    {
        Append<true>(left_, value);
    }

    void push_back(const T &value)
    {
        Append<false>(right_, value);
    }

    void pop_front()
    {
        Pop<true>(left_, right_);
    }

    void pop_back()
    {
        Pop<false>(right_, left_);
    }

    [[nodiscard]] const T &front() const
    {
        CheckNonempty();
        return left_.empty() ? right_.front().value : left_.back().value;
    }

    [[nodiscard]] const T &back() const
    {
        CheckNonempty();
        return right_.empty() ? left_.front().value : right_.back().value;
    }

    [[nodiscard]] T all_prod() const
    {
        if (left_.empty())
        {
            return right_.empty() ? identity_ : right_.back().product;
        }
        if (right_.empty())
        {
            return left_.back().product;
        }
        return std::invoke(op_, left_.back().product, right_.back().product);
    }

    void clear() noexcept
    {
        left_.clear();
        right_.clear();
    }

    void swap(DequeAggregation &other) noexcept
        requires std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<Op>
    {
        if (this == &other)
        {
            return;
        }
        using std::swap;
        swap(op_, other.op_);
        swap(identity_, other.identity_);
        left_.swap(other.left_);
        right_.swap(other.right_);
    }

private:
    [[no_unique_address]] Op op_;
    T identity_;
    std::vector<Entry> left_, right_;

    void CheckNonempty() const
    {
    }

    template <bool Front>
    T Extend(const T &product, const T &value) const
    {
        if constexpr (Front)
        {
            return std::invoke(op_, value, product);
        }
        else
        {
            return std::invoke(op_, product, value);
        }
    }

    template <bool Front>
    void Append(std::vector<Entry> &stack, const T &value)
    {
        T product = stack.empty() ? value : Extend<Front>(stack.back().product, value);
        stack.push_back(Entry{value, std::move_if_noexcept(product)});
    }

    template <bool Front>
    void Pop(std::vector<Entry> &target, std::vector<Entry> &source)
    {
        CheckNonempty();
        if (target.empty())
        {
            if (source.size() == 1)
            {
                source.pop_back();
                return;
            }
            Rebalance<Front>(target, source);
        }
        target.pop_back();
    }

    template <bool Front>
    void Rebalance(std::vector<Entry> &target, std::vector<Entry> &source)
    {
        const auto count = source.size();
        const auto remaining = count / 2;
        const auto transferred = count - remaining;
        if constexpr (kInPlaceRebalance)
        {
            // 確保が終われば、値・集約の構築と代入は例外を送出しない。
            target.reserve(transferred);
            for (auto i = transferred; i != 0; --i)
            {
                Append<Front>(target, source[i - 1].value);
            }
            for (std::size_t i = 0; i < remaining; ++i)
            {
                auto &entry = source[i];
                entry.value = source[i + transferred].value;
                entry.product = i == 0 ? entry.value : Extend<!Front>(source[i - 1].product, entry.value);
            }
            source.erase(source.begin() + static_cast<typename std::vector<Entry>::difference_type>(remaining), source.end());
        }
        else
        {
            // 両方の集約を作り直せた時点で差し替える。途中で失敗しても元の列を保つ。
            std::vector<Entry> new_target, new_source;
            new_target.reserve(transferred);
            new_source.reserve(remaining);
            for (auto i = transferred; i != 0; --i)
            {
                Append<Front>(new_target, source[i - 1].value);
            }
            for (auto i = transferred; i < count; ++i)
            {
                Append<!Front>(new_source, source[i].value);
            }
            target.swap(new_target);
            source.swap(new_source);
        }
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
