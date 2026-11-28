// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Run_Length_Queue.cpp
// 参考（運用例）: https://atcoder.jp/contests/abc413/tasks/abc413_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/run_length_queue.md
//
// API一覧
// RunLengthQueue<T>(): 空で構築。同じ値の連続を1組にまとめる。Tのconst比較==が必要。
// size()/empty()/run_count(): 要素数(uint64_t)/空か/連続グループ数。O(1)。
// front()/front_count(): 先頭の値/その連続個数。空のfront()はout_of_range、front_count()は0。O(1)。
// push_back(value,count=1): valueをcount個追加。O(1)。検査有効時は要素数上限を超えるとlength_error。
// pop_front(count=1): 先頭count個を削除。consume_front(count,visitor): 削除する各組にvisitor(const T&,個数)を呼ぶ。
// 削除は触る連続グループ数+1に比例。空からの追加・削除全体では操作回数に比例。count<=size()。
// 保存する値は現在の連続グループ数。内部領域は同時保持グループ数の最大+1に比例。コールバックは同じキューを変更しないこと。例外時は成功済みの呼出し分だけ削除。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RUN_LENGTH_QUEUE_DEFINED
#define SIRKIT_RUN_LENGTH_QUEUE_DEFINED

template <typename T>
class RunLengthQueue
{
    using Count = std::uint64_t;
    struct Run
    {
        T value;
        Count end;
    };

    std::deque<Run> runs_;
    Count head_ = 0;

public:
    RunLengthQueue() = default;
    RunLengthQueue(const RunLengthQueue &) requires std::copy_constructible<T> = default;

    RunLengthQueue(RunLengthQueue &&other)
        : runs_(std::move(other.runs_)), head_(std::exchange(other.head_, 0))
    {
        other.runs_.clear();
    }

    RunLengthQueue &operator=(const RunLengthQueue &other) requires std::copy_constructible<T>
    {
        if (this != &other)
        {
            auto copied = other.runs_;
            runs_.swap(copied);
            head_ = other.head_;
        }
        return *this;
    }

    RunLengthQueue &operator=(RunLengthQueue &&other)
    {
        if (this != &other)
        {
            runs_ = std::move(other.runs_);
            head_ = std::exchange(other.head_, 0);
            other.runs_.clear();
        }
        return *this;
    }

    [[nodiscard]] Count size() const noexcept
    {
        return runs_.empty() ? 0 : runs_.back().end - head_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return runs_.empty();
    }

    [[nodiscard]] std::size_t run_count() const noexcept
    {
        return runs_.size();
    }

    [[nodiscard]] const T &front() const
    {
        return runs_.front().value;
    }

    [[nodiscard]] Count front_count() const noexcept
    {
        return runs_.empty() ? 0 : runs_.front().end - head_;
    }

    void push_back(T value, Count count = 1)
    {
        if (count == 0)
        {
            return;
        }
        const Count previous = size();
        // 通算位置は周回してよい。差を取ると現在の個数に戻る。
        const Count end = head_ + previous + count;
        if (!runs_.empty() && std::as_const(runs_.back().value) == std::as_const(value))
        {
            runs_.back().end = end;
        }
        else
        {
            runs_.push_back(Run{std::move(value), end});
        }
    }

    template <typename Visitor>
    void consume_front(Count count, Visitor &&visitor)
    {
        while (count != 0)
        {
            const auto &run = runs_.front();
            const Count removed = std::min(count, run.end - head_);
            std::invoke(visitor, run.value, removed);
            head_ += removed;
            count -= removed;
            if (head_ == run.end)
            {
                runs_.pop_front();
            }
        }
    }

    void pop_front(Count count = 1)
    {
        consume_front(count, [](const T &, Count) noexcept
        {
        });
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
