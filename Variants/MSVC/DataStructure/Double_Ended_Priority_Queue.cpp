// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: DataStructure/Double_Ended_Priority_Queue.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/double_ended_priority_queue.md
//
// API一覧
// DoubleEndedPriorityQueue<T,Compare>(): 空。DoubleEndedPriorityQueue(values,compare={}): O(n)構築。
// size()/empty(): 個数/空か。O(1)。push(value): 要素を追加。償却O(log(n+1))。
// min()/max(): 比較順の両端へのconst参照。O(1)。pop_min()/pop_max(): 両端を取り出す。O(log(n+1))。
// 両端の取得・削除には要素が必要。重複可。追加・削除で取得済みの参照を無効にする。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DOUBLE_ENDED_PRIORITY_QUEUE_DEFINED
#define SIRKIT_DOUBLE_ENDED_PRIORITY_QUEUE_DEFINED

template <class T, class Compare = std::less<T>>
class DoubleEndedPriorityQueue
{
public:
    DoubleEndedPriorityQueue() = default;

    explicit DoubleEndedPriorityQueue(std::vector<T> values, Compare compare = {})
        : values_(std::move(values)), compare_(std::move(compare))
    {
        for (auto i = values_.size() / 2; i != 0;)
        {
            --i;
            sift_down(i, is_min_level(i));
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return values_.empty();
    }

    [[nodiscard]] const T &min() const
    {
        require_nonempty();
        return values_.front();
    }

    [[nodiscard]] const T &max() const
    {
        require_nonempty();
        return values_[max_index()];
    }

    void push(T value)
    {
        auto i = values_.size();
        values_.push_back(std::move(value));
        if (i == 0)
        {
            return;
        }

        auto min_level = is_min_level(i);
        const auto parent = (i - 1) / 2;
        if (earlier(parent, i, min_level))
        {
            // 親レベルとの順序が逆なら、親を起点に更新する。
            std::swap(values_[i], values_[parent]);
            i = parent;
            min_level = !min_level;
        }
        while (i >= 3)
        {
            const auto grandparent = (i - 3) / 4;
            if (!earlier(i, grandparent, min_level))
            {
                break;
            }
            std::swap(values_[i], values_[grandparent]);
            i = grandparent;
        }
    }

    T pop_min()
    {
        require_nonempty();
        return take(0, true);
    }

    T pop_max()
    {
        require_nonempty();
        const auto index = max_index();
        return take(index, index == 0);
    }

private:
    std::vector<T> values_;
    [[no_unique_address]] Compare compare_{};

    static bool is_min_level(std::size_t index) noexcept
    {
        return (std::bit_width(index + 1) & 1U) != 0;
    }

    void require_nonempty() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("DoubleEndedPriorityQueue: empty");
        }
#endif
    }

    std::size_t max_index() const
    {
        if (size() < 3)
        {
            return size() - 1;
        }
        return compare_(values_[1], values_[2]) ? 2 : 1;
    }

    bool earlier(std::size_t a, std::size_t b, bool min_level) const
    {
        return min_level ? compare_(values_[a], values_[b]) : compare_(values_[b], values_[a]);
    }

    void sift_down(std::size_t index, bool min_level)
    {
        const auto count = size();
        while (index < count / 2)
        {
            const auto left = index * 2 + 1;
            auto best = left;
            if (left + 1 < count && earlier(left + 1, best, min_level))
            {
                ++best;
            }
            if (left < count / 2)
            {
                const auto first = left * 2 + 1;
                const auto last = first + std::min<std::size_t>(4, count - first);
                for (auto child = first; child < last; ++child)
                {
                    if (earlier(child, best, min_level))
                    {
                        best = child;
                    }
                }
            }
            if (!earlier(best, index, min_level))
            {
                break;
            }
            std::swap(values_[best], values_[index]);
            if (best <= left + 1)
            {
                break;
            }
            const auto parent = (best - 1) / 2;
            if (earlier(parent, best, min_level))
            {
                std::swap(values_[parent], values_[best]);
            }
            index = best;
        }
    }

    T take(std::size_t index, bool min_level)
    {
        auto result = std::move(values_[index]);
        if (index + 1 == size())
        {
            values_.pop_back();
        }
        else
        {
            values_[index] = std::move(values_.back());
            values_.pop_back();
            sift_down(index, min_level);
        }
        return result;
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
