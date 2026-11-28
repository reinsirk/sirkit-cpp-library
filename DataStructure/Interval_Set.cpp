// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_set.md
//
// API一覧
// IntervalSet<T>(): 空の整数集合を作る。Tはbool以外の整数型。O(1)。
// add(left,right)/erase(left,right): 半開区間の整数を追加・削除する。
// left<=right。空区間は何もしない。時間はO(log(m+1)+k)で、mは区間数、kは変更する区間数。
// contains(value): valueが含まれるか。O(log(m+1))。
// covers(left,right): 半開区間をすべて含むか。left<=right。空区間はtrue。O(log(m+1))。
// mex(first=0): first以上で含まれない最小の整数。O(log(m+1))。
// interval_at(value): valueを含む最大の半開区間。含まれなければnullopt。O(log(m+1))。
// size()/interval_count()/empty(): 整数の総数・区間数・空判定。O(1)。
// clear(): 全削除。O(m)。swap(other): 集合を交換する。O(1)。
// Tの最大値は右端として使える。要素としては格納しない。領域O(m+1)。
// 入力検査有効時はleft>rightでinvalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INTERVAL_SET_DEFINED
#define SIRKIT_INTERVAL_SET_DEFINED

template <std::integral T = int>
    requires(!std::same_as<T, bool>)
class IntervalSet
{
public:
    using size_type = std::make_unsigned_t<T>;

    IntervalSet() = default;
    IntervalSet(const IntervalSet &) = default;

    IntervalSet(IntervalSet &&other)
    {
        swap(other);
    }

    IntervalSet &operator=(IntervalSet other) noexcept
    {
        swap(other);
        return *this;
    }

    void add(T left, T right)
    {
        CheckInterval(left, right);
        if (left == right)
        {
            return;
        }

        auto first = intervals_.lower_bound(left);
        if (first != intervals_.begin())
        {
            const auto previous = std::prev(first);
            if (left <= previous->second)
            {
                first = previous;
            }
        }
        if (first == intervals_.end() || left < first->first)
        {
            first = intervals_.emplace_hint(first, left, right);
        }
        else
        {
            size_ -= Length(first->first, first->second);
            first->second = std::max(first->second, right);
        }
        auto next = std::next(first);
        while (next != intervals_.end() && next->first <= first->second)
        {
            size_ -= Length(next->first, next->second);
            first->second = std::max(first->second, next->second);
            next = intervals_.erase(next);
        }
        size_ += Length(first->first, first->second);
    }

    void erase(T left, T right)
    {
        CheckInterval(left, right);
        if (left == right)
        {
            return;
        }

        auto current = intervals_.lower_bound(left);
        if (current != intervals_.begin())
        {
            const auto previous = std::prev(current);
            const T old_end = previous->second;
            if (left < old_end)
            {
                if (right < old_end)
                {
                    intervals_.emplace_hint(current, right, old_end);
                    previous->second = left;
                    size_ -= Length(left, right);
                    return;
                }
                previous->second = left;
                size_ -= Length(left, old_end);
            }
        }

        while (current != intervals_.end() && current->first < right)
        {
            if (right < current->second)
            {
                size_ -= Length(current->first, right);
                const auto next = std::next(current);
                auto node = intervals_.extract(current);
                node.key() = right;
                intervals_.insert(next, std::move(node));
                return;
            }
            size_ -= Length(current->first, current->second);
            current = intervals_.erase(current);
        }
    }

    [[nodiscard]] bool contains(T value) const
    {
        return Containing(value) != intervals_.end();
    }

    [[nodiscard]] bool covers(T left, T right) const
    {
        CheckInterval(left, right);
        if (left == right)
        {
            return true;
        }
        const auto current = Containing(left);
        return current != intervals_.end() && right <= current->second;
    }

    [[nodiscard]] T mex(T first = 0) const
    {
        const auto current = Containing(first);
        return current == intervals_.end() ? first : current->second;
    }

    [[nodiscard]] std::optional<std::pair<T, T>> interval_at(T value) const
    {
        const auto current = Containing(value);
        if (current == intervals_.end())
        {
            return std::nullopt;
        }
        return *current;
    }

    [[nodiscard]] size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::size_t interval_count() const noexcept
    {
        return intervals_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return intervals_.empty();
    }

    void clear() noexcept
    {
        intervals_.clear();
        size_ = 0;
    }

    void swap(IntervalSet &other) noexcept
    {
        intervals_.swap(other.intervals_);
        std::swap(size_, other.size_);
    }

private:
    static void CheckInterval([[maybe_unused]] T left, [[maybe_unused]] T right)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (right < left)
        {
            throw std::invalid_argument("IntervalSet: reversed interval");
        }
#endif
    }

    [[nodiscard]] static size_type Length(T left, T right) noexcept
    {
        return static_cast<size_type>(static_cast<size_type>(right) - static_cast<size_type>(left));
    }

    [[nodiscard]] typename std::map<T, T>::const_iterator Containing(T value) const
    {
        auto current = intervals_.upper_bound(value);
        if (current == intervals_.begin())
        {
            return intervals_.end();
        }
        --current;
        return value < current->second ? current : intervals_.end();
    }

    std::map<T, T> intervals_;
    size_type size_ = 0;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
