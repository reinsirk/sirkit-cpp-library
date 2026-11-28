// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/indexed_priority_queue.md
//
// API一覧
// IndexedPriorityQueue<Key,Largest>(id_limit): IDが0以上id_limit未満の空のキュー。O(id_limit+1)。
// Keyは整数型。Largestがfalseなら最小、trueなら最大を先に取り出す。同じ優先度は小さいIDを先に扱う。
// set(id,priority): 未登録なら追加、登録済みなら優先度を更新。O(log(n+1))。
// erase(id): 削除できたかを返す。O(log(n+1))。
// top()/pop(): {ID,優先度}の取得・取り出し。O(1)/O(log(n+1))。
// priority(id)/contains(id): 登録済みの優先度・登録済みか。各O(1)。
// id_limit()/size()/empty(): ID上限・登録数・空判定。O(1)。clear(): 全削除。O(n)。
// nは登録数。領域O(id_limit+1)。入力検査有効時は範囲外のID、空のtop/pop、未登録のpriorityでout_of_range。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INDEXED_PRIORITY_QUEUE_DEFINED
#define SIRKIT_INDEXED_PRIORITY_QUEUE_DEFINED

template <std::integral Key = int, bool Largest = false>
class IndexedPriorityQueue
{
public:
    explicit IndexedPriorityQueue(std::size_t id_limit)
        : positions_(id_limit, kMissing), keys_(id_limit)
    {
        heap_.reserve(id_limit);
    }

    IndexedPriorityQueue(const IndexedPriorityQueue &other)
        : positions_(other.positions_), keys_(other.keys_), heap_(other.heap_)
    {
        heap_.reserve(id_limit());
    }

    IndexedPriorityQueue(IndexedPriorityQueue &&) noexcept = default;

    IndexedPriorityQueue &operator=(IndexedPriorityQueue other) noexcept
    {
        positions_.swap(other.positions_);
        keys_.swap(other.keys_);
        heap_.swap(other.heap_);
        return *this;
    }

    void set(std::size_t id, Key value)
    {
        CheckId(id);
        auto position = positions_[id];
        if (position == kMissing)
        {
            position = heap_.size();
            heap_.push_back(id);
            positions_[id] = position;
            keys_[id] = value;
            Swim(position);
            return;
        }
        const Key previous = keys_[id];
        keys_[id] = value;
        if (Precedes(value, previous))
        {
            Swim(position);
        }
        else
        {
            Sink(position);
        }
    }

    bool erase(std::size_t id)
    {
        CheckId(id);
        if (positions_[id] == kMissing)
        {
            return false;
        }
        Remove(id);
        return true;
    }

    [[nodiscard]] std::pair<std::size_t, Key> top() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("IndexedPriorityQueue: empty queue");
        }
#endif
        const auto id = heap_.front();
        return {id, static_cast<Key>(keys_[id])};
    }

    [[nodiscard]] std::pair<std::size_t, Key> pop()
    {
        const auto result = top();
        Remove(result.first);
        return result;
    }

    [[nodiscard]] Key priority(std::size_t id) const
    {
        CheckId(id);
#ifndef SKIP_RUNTIME_CHECKS
        if (positions_[id] == kMissing)
        {
            throw std::out_of_range("IndexedPriorityQueue: inactive ID");
        }
#endif
        return keys_[id];
    }

    [[nodiscard]] bool contains(std::size_t id) const
    {
        CheckId(id);
        return positions_[id] != kMissing;
    }

    [[nodiscard]] std::size_t id_limit() const noexcept
    {
        return positions_.size();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return heap_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return heap_.empty();
    }

    void clear() noexcept
    {
        for (const auto id : heap_)
        {
            positions_[id] = kMissing;
        }
        heap_.clear();
    }

private:
    static constexpr std::size_t kMissing = std::numeric_limits<std::size_t>::max();

    [[nodiscard]] static bool Precedes(Key lhs, Key rhs) noexcept
    {
        if constexpr (Largest)
        {
            return lhs > rhs;
        }
        else
        {
            return lhs < rhs;
        }
    }

    [[nodiscard]] bool Earlier(std::size_t lhs, std::size_t rhs) const noexcept
    {
        const Key left = keys_[lhs], right = keys_[rhs];
        return left == right ? lhs < rhs : Precedes(left, right);
    }

    void CheckId([[maybe_unused]] std::size_t id) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= id_limit())
        {
            throw std::out_of_range("IndexedPriorityQueue: ID out of bounds");
        }
#endif
    }

    void SwapEntries(std::size_t a, std::size_t b) noexcept
    {
        std::swap(heap_[a], heap_[b]);
        positions_[heap_[a]] = a;
        positions_[heap_[b]] = b;
    }

    void Swim(std::size_t at) noexcept
    {
        while (at != 0)
        {
            const auto parent = (at - 1) / 2;
            if (!Earlier(heap_[at], heap_[parent]))
            {
                break;
            }
            SwapEntries(at, parent);
            at = parent;
        }
    }

    void Sink(std::size_t at) noexcept
    {
        while (at < heap_.size() / 2)
        {
            auto child = 2 * at + 1;
            if (child + 1 < heap_.size() && Earlier(heap_[child + 1], heap_[child]))
            {
                ++child;
            }
            if (!Earlier(heap_[child], heap_[at]))
            {
                break;
            }
            SwapEntries(at, child);
            at = child;
        }
    }

    void Remove(std::size_t id) noexcept
    {
        const auto at = positions_[id];
        const auto last = heap_.back();
        heap_.pop_back();
        positions_[id] = kMissing;
        if (at == heap_.size())
        {
            return;
        }
        heap_[at] = last;
        positions_[last] = at;
        if (at != 0 && Earlier(last, heap_[(at - 1) / 2]))
        {
            Swim(at);
        }
        else
        {
            Sink(at);
        }
    }

    std::vector<std::size_t> positions_;
    std::vector<Key> keys_;
    std::vector<std::size_t> heap_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
