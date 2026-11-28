// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Top_K_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/top_k_sum.md
//
// API一覧
// TopKSum<T,Largest,Sum>(k=0): 空の多重集合を作る。Largestがtrueなら大きい方から選ぶ。O(1)。
// insert(value)/erase(value): 1個ずつ追加・削除する。eraseは要素がなければfalse。O(log(n+1))。
// set_k(k): 選ぶ個数を更新する。実際に選ぶ個数の差をdとしてO(1+d log(n+1))。
// sum(): 先頭min(k,n)個の和。結果がSumに収まること。O(1)。
// kth(): 順序上のk番目を返す。k=0またはn<kならnullopt。O(1)。
// k()/size()/empty(): 選ぶ個数・全要素数・空判定。O(1)。
// clear(): kを保ちながら全要素を削除する。O(n)。swap(other): O(1)。
// T・Sumはbool以外の整数型。nは現在の要素数。領域O(n+1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <set>
#include <type_traits>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TOP_K_SUM_DEFINED
#define SIRKIT_TOP_K_SUM_DEFINED

template <std::integral T = int, bool Largest = false, std::integral Sum = T>
    requires(!std::same_as<T, bool> && !std::same_as<Sum, bool>)
class TopKSum
{
public:
    explicit TopKSum(std::size_t k = 0) : k_(k)
    {
    }

    TopKSum(const TopKSum &) = default;

    TopKSum(TopKSum &&other)
    {
        swap(other);
    }

    TopKSum &operator=(TopKSum other) noexcept
    {
        swap(other);
        return *this;
    }

    void insert(T value)
    {
        if (selected_.size() < k_)
        {
            selected_.insert(value);
            sum_bits_ += static_cast<SumBits>(value);
        }
        else if (!selected_.empty() && Order{}(value, *selected_.rbegin()))
        {
            selected_.insert(value);
            sum_bits_ += static_cast<SumBits>(value);
            ExcludeLast();
        }
        else
        {
            remaining_.insert(value);
        }
    }

    bool erase(T value)
    {
        const auto remaining = remaining_.find(value);
        if (remaining != remaining_.end())
        {
            remaining_.erase(remaining);
            return true;
        }
        const auto selected = selected_.find(value);
        if (selected == selected_.end())
        {
            return false;
        }
        sum_bits_ -= static_cast<SumBits>(*selected);
        selected_.erase(selected);
        if (!remaining_.empty())
        {
            IncludeFirst();
        }
        return true;
    }

    void set_k(std::size_t k)
    {
        k_ = k;
        while (selected_.size() > k_)
        {
            ExcludeLast();
        }
        while (selected_.size() < k_ && !remaining_.empty())
        {
            IncludeFirst();
        }
    }

    [[nodiscard]] Sum sum() const noexcept
    {
        return static_cast<Sum>(sum_bits_);
    }

    [[nodiscard]] std::optional<T> kth() const noexcept
    {
        if (k_ == 0 || selected_.size() < k_)
        {
            return std::nullopt;
        }
        return *selected_.rbegin();
    }

    [[nodiscard]] std::size_t k() const noexcept
    {
        return k_;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return selected_.size() + remaining_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    void clear() noexcept
    {
        selected_.clear();
        remaining_.clear();
        sum_bits_ = 0;
    }

    void swap(TopKSum &other) noexcept
    {
        selected_.swap(other.selected_);
        remaining_.swap(other.remaining_);
        std::swap(k_, other.k_);
        std::swap(sum_bits_, other.sum_bits_);
    }

private:
    using Order = std::conditional_t<Largest, std::greater<T>, std::less<T>>;
    using SumBits = std::make_unsigned_t<Sum>;

    void ExcludeLast()
    {
        const auto position = std::prev(selected_.end());
        sum_bits_ -= static_cast<SumBits>(*position);
        remaining_.insert(remaining_.begin(), selected_.extract(position));
    }

    void IncludeFirst()
    {
        const auto position = remaining_.begin();
        sum_bits_ += static_cast<SumBits>(*position);
        selected_.insert(selected_.end(), remaining_.extract(position));
    }

    std::size_t k_ = 0;
    // 符号なしで保持し、境界入替えの途中でも符号付きのオーバーフローを起こさない。
    SumBits sum_bits_ = 0;
    std::multiset<T, Order> selected_, remaining_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
