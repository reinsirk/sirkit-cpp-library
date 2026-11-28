// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Rollback_LIS.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rollback_lis.md
//
// API一覧
// [RollbackLIS<T,Strict=true>] 末尾追加・削除する列の最長増加部分列。Strict=falseは非減少。
// push_back(value): 末尾追加。現在のLIS長をLとして償却O(1+log(L+1))。
// pop_back(): 末尾削除。空でないこと。O(1)。
// rollback(prefix_size): 指定した長さまで末尾を削除。prefix_size<=size()。1要素あたりO(1)。
// indices(): 一つの最長増加部分列を、現在の列の0始まりの添字で返す。O(L+1)時間・領域。
// length()/size()/empty(): LIS長/現在の列長/空か。各O(1)。
// reserve(capacity)/clear(): 領域予約/空へ戻す。確保済み領域は末尾削除やclearで解放しない。
// Tのムーブ構築と破棄はnoexcept、operator<は一貫した狭義弱順序。比較とT操作をO(1)として計算。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROLLBACK_LIS_DEFINED
#define SIRKIT_ROLLBACK_LIS_DEFINED

template<class T, bool Strict = true>
    requires std::destructible<T> && std::is_nothrow_move_constructible_v<T>
class RollbackLIS
{
    static constexpr auto kNoIndex = std::numeric_limits<std::size_t>::max();

    struct Entry
    {
        T value;
        std::size_t slot;
        std::size_t previous_tail;
        std::size_t predecessor;
    };

  public:
    RollbackLIS() = default;
    RollbackLIS(const RollbackLIS &) requires std::copy_constructible<T> = default;
    RollbackLIS(RollbackLIS &&other) noexcept
    {
        exchange(other);
    }

    RollbackLIS &operator=(RollbackLIS other) noexcept
    {
        exchange(other);
        return *this;
    }

    void push_back(T value)
    {
        const auto slot = find_slot(value);
        const auto index = size();
        const bool extends = slot == length();
        const auto previous = extends ? kNoIndex : tails_[slot];
        const auto predecessor = slot == 0 ? kNoIndex : tails_[slot - 1];
        entries_.push_back({std::move(value), slot, previous, predecessor});
        if (extends)
        {
            try
            {
                tails_.push_back(index);
            }
            catch (...)
            {
                entries_.pop_back();
                throw;
            }
        }
        else
        {
            tails_[slot] = index;
        }
    }

    void pop_back()
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("RollbackLIS: pop_back on an empty sequence");
        }
#endif
        undo_last();
    }

    void rollback(std::size_t prefix_size)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (prefix_size > size())
        {
            throw std::out_of_range("RollbackLIS: invalid prefix length");
        }
#endif
        while (size() > prefix_size)
        {
            undo_last();
        }
    }

    [[nodiscard]] std::vector<std::size_t> indices() const
    {
        std::vector<std::size_t> result(length());
        if (!result.empty())
        {
            auto index = tails_.back();
            for (std::size_t i = result.size(); i > 0; --i)
            {
                result[i - 1] = index;
                index = entries_[index].predecessor;
            }
        }
        return result;
    }

    [[nodiscard]] std::size_t length() const noexcept
    {
        return tails_.size();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return entries_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return entries_.empty();
    }

    void reserve(std::size_t capacity)
    {
        entries_.reserve(capacity);
        tails_.reserve(capacity);
    }

    void clear() noexcept
    {
        entries_.clear();
        tails_.clear();
    }

  private:
    [[nodiscard]] std::size_t find_slot(const T &value) const
    {
        if constexpr (Strict)
        {
            const auto found = std::lower_bound(tails_.begin(), tails_.end(), value,
                [this](std::size_t index, const T &target) { return entries_[index].value < target; });
            return static_cast<std::size_t>(found - tails_.begin());
        }
        else
        {
            const auto found = std::upper_bound(tails_.begin(), tails_.end(), value,
                [this](const T &target, std::size_t index) { return target < entries_[index].value; });
            return static_cast<std::size_t>(found - tails_.begin());
        }
    }

    void undo_last() noexcept
    {
        const auto &entry = entries_.back();
        if (entry.previous_tail == kNoIndex)
        {
            tails_.pop_back();
        }
        else
        {
            tails_[entry.slot] = entry.previous_tail;
        }
        entries_.pop_back();
    }

    void exchange(RollbackLIS &other) noexcept
    {
        entries_.swap(other.entries_);
        tails_.swap(other.tails_);
    }

    std::vector<Entry> entries_;
    std::vector<std::size_t> tails_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
