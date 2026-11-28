// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Ring_Deque.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/ring_deque.md
//
// API一覧
// RingDeque<T>(): 空の両端キュー。O(1)。Tはコピー構築可能、またはムーブ構築が非送出。
// push_front/back(value), emplace_front/back(args...): 前後に追加。償却O(1)。emplaceは追加要素への参照。
// pop_front/back(): 前後から削除。O(1)。front()/back()/operator[](i)/at(i): 要素への参照。O(1)。
// front/back/popは空でないこと、添字は0始まり。atは常に、他は検査有効時に範囲外を例外にする。
// size()/empty()/capacity(): 要素数、空判定、容量。O(1)。reserve(n): 容量を確保。容量増加時にO(n)。
// clear(): 容量を保持して全要素を破棄。O(size)。コピーは独立、移動元は空。容量増加で既存要素の参照が無効になる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RING_DEQUE_DEFINED
#define SIRKIT_RING_DEQUE_DEFINED

template <class T>
class RingDeque
{
    static_assert(std::is_nothrow_move_constructible_v<T> || std::is_copy_constructible_v<T>);

public:
    RingDeque() = default;
    RingDeque(const RingDeque &) = default;

    RingDeque(RingDeque &&other) noexcept
        : slots_(std::move(other.slots_)),
          head_(std::exchange(other.head_, 0)),
          count_(std::exchange(other.count_, 0))
    {
    }

    RingDeque &operator=(RingDeque other) noexcept
    {
        slots_.swap(other.slots_);
        std::swap(head_, other.head_);
        std::swap(count_, other.count_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return count_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return count_ == 0;
    }

    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return slots_.size();
    }

    void reserve(std::size_t minimum)
    {
        if (minimum <= capacity())
        {
            return;
        }
        if (minimum > std::bit_floor(slots_.max_size()))
        {
            throw std::length_error("RingDeque: capacity");
        }
        std::vector<std::optional<T>> next(std::bit_ceil(minimum));
        for (std::size_t i = 0; i < count_; ++i)
        {
            next[i].emplace(std::move_if_noexcept(*slots_[Physical(i)]));
        }
        slots_.swap(next);
        head_ = 0;
    }

    template <class... Args>
    T &emplace_front(Args &&...args)
    {
        return Emplace(true, std::forward<Args>(args)...);
    }

    template <class... Args>
    T &emplace_back(Args &&...args)
    {
        return Emplace(false, std::forward<Args>(args)...);
    }

    void push_front(const T &value)
    {
        emplace_front(value);
    }

    void push_front(T &&value)
    {
        emplace_front(std::move(value));
    }

    void push_back(const T &value)
    {
        emplace_back(value);
    }

    void push_back(T &&value)
    {
        emplace_back(std::move(value));
    }

    void pop_front()
    {
        CheckIndex(0);
        slots_[head_].reset();
        head_ = Physical(1);
        --count_;
    }

    void pop_back()
    {
        CheckIndex(0);
        slots_[Physical(--count_)].reset();
    }

    void clear() noexcept
    {
        for (std::size_t i = 0; i < count_; ++i)
        {
            slots_[Physical(i)].reset();
        }
        head_ = count_ = 0;
    }

    [[nodiscard]] T &operator[](std::size_t i)
    {
        CheckIndex(i);
        return *slots_[Physical(i)];
    }

    [[nodiscard]] const T &operator[](std::size_t i) const
    {
        CheckIndex(i);
        return *slots_[Physical(i)];
    }

    [[nodiscard]] T &at(std::size_t i)
    {
        RequireIndex(i);
        return *slots_[Physical(i)];
    }

    [[nodiscard]] const T &at(std::size_t i) const
    {
        RequireIndex(i);
        return *slots_[Physical(i)];
    }

    [[nodiscard]] T &front()
    {
        return (*this)[0];
    }

    [[nodiscard]] const T &front() const
    {
        return (*this)[0];
    }

    [[nodiscard]] T &back()
    {
        return (*this)[count_ - 1];
    }

    [[nodiscard]] const T &back() const
    {
        return (*this)[count_ - 1];
    }

private:
    std::vector<std::optional<T>> slots_;
    std::size_t head_ = 0;
    std::size_t count_ = 0;

    [[nodiscard]] std::size_t Physical(std::size_t index) const noexcept
    {
        return (head_ + index) & (capacity() - 1);
    }

    void RequireIndex(std::size_t i) const
    {
        if (i >= count_)
        {
            throw std::out_of_range("RingDeque: index");
        }
    }

    void CheckIndex([[maybe_unused]] std::size_t i) const
    {
    }

    template <class... Args>
    T &Place(bool on_front, Args &&...args)
    {
        const auto position = on_front ? Physical(static_cast<std::size_t>(-1)) : Physical(count_);
        T &result = slots_[position].emplace(std::forward<Args>(args)...);
        head_ = on_front ? position : head_;
        ++count_;
        return result;
    }

    template <class... Args>
    T &Emplace(bool on_front, Args &&...args)
    {
        if (count_ < capacity())
        {
            return Place(on_front, std::forward<Args>(args)...);
        }
        // 引数が既存要素を参照していても、容量を増やす前に値を作る。
        T value(std::forward<Args>(args)...);
        reserve(std::max(std::size_t{8}, count_ + 1));
        return Place(on_front, std::move_if_noexcept(value));
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
