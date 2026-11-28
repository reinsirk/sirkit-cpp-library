// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: DataStructure/Radix_Heap.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/radix_heap.md
//
// API一覧
// RadixHeap<Key,Value>(lower_bound=0): 空の単調最小キュー。Keyはbool以外の64ビット以下の符号なし整数型。
// Valueは非送出でムーブ構築できる型。キーの幅をWとしてO(W)。
// emplace(key,args...): keyに対応する値を構築して追加する。key>=lower_bound()が必要。償却O(1)。
// top(): 最小キーの要素をconst参照で返す。下限を変えない。走査するバケットの大きさをbとしてO(1+b)。
// pop(): 最小キーの要素を取り出し、下限をそのキーへ進める。挿入件数あたり償却O(W)、最悪O(W+b)。
// lower_bound()/size()/empty(): 挿入できる最小キー・要素数・空判定。O(1)。
// clear(lower_bound=0): 全要素を除き、下限を指定値に戻す。容量は再利用。O(W+n)。
// swap(other): キューを交換する。O(W)。同じキーの取り出し順は未規定。
// top/popは非空が前提。入力検査有効時に空ならout_of_range、下限未満の追加はinvalid_argument。
// topの参照は追加、削除、交換で無効になり得る。最大要素数をMとして領域O(W(M+1))。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RADIX_HEAP_DEFINED
#define SIRKIT_RADIX_HEAP_DEFINED

template <std::unsigned_integral Key, class Value>
    requires(!std::same_as<Key, bool> && std::numeric_limits<Key>::digits <= 64 &&
             std::is_nothrow_move_constructible_v<Value>)
class RadixHeap
{
public:
    using value_type = std::pair<Key, Value>;

    explicit RadixHeap(Key lower_bound = 0) noexcept : lower_bound_(lower_bound)
    {
    }

    RadixHeap(const RadixHeap &) = default;

    RadixHeap(RadixHeap &&other) noexcept
    {
        swap(other);
    }

    RadixHeap &operator=(RadixHeap other) noexcept
    {
        swap(other);
        return *this;
    }

    template <class... Args>
    void emplace(Key key, Args &&...args)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (key < lower_bound_)
        {
            throw std::invalid_argument("RadixHeap: key is below the lower bound");
        }
#endif
        const auto index = Bucket(key, lower_bound_);
        buckets_[index].emplace_back(std::piecewise_construct, std::forward_as_tuple(key),
                                     std::forward_as_tuple(std::forward<Args>(args)...));
        MarkOccupied(index);
        ++size_;
    }

    [[nodiscard]] const value_type &top() const
    {
        RequireNonempty();
        if (!buckets_[0].empty())
        {
            return buckets_[0].back();
        }
        const auto &bucket = buckets_[FirstOutsideLowerBound()];
        return bucket[MinIndex(bucket)];
    }

    [[nodiscard]] value_type pop()
    {
        RequireNonempty();
        if (buckets_[0].empty())
        {
            Refill();
        }
        auto &bucket = buckets_[0];
        value_type result = std::move(bucket.back());
        bucket.pop_back();
        --size_;
        return result;
    }

    [[nodiscard]] Key lower_bound() const noexcept
    {
        return lower_bound_;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size_ == 0;
    }

    void clear(Key lower_bound = 0) noexcept
    {
        for (auto &bucket : buckets_)
        {
            bucket.clear();
        }
        lower_bound_ = lower_bound;
        occupied_ = 0;
        size_ = 0;
    }

    void swap(RadixHeap &other) noexcept
    {
        buckets_.swap(other.buckets_);
        std::swap(lower_bound_, other.lower_bound_);
        std::swap(occupied_, other.occupied_);
        std::swap(size_, other.size_);
    }

private:
    static constexpr std::size_t kBucketCount = std::numeric_limits<Key>::digits + 1;

    [[nodiscard]] static std::size_t Bucket(Key key, Key lower_bound) noexcept
    {
        return static_cast<std::size_t>(std::bit_width(static_cast<std::uint64_t>(key) ^
                                                       static_cast<std::uint64_t>(lower_bound)));
    }

    void MarkOccupied(std::size_t index) noexcept
    {
        if (index != 0)
        {
            occupied_ |= std::uint64_t{1} << (index - 1);
        }
    }

    [[nodiscard]] std::size_t FirstOutsideLowerBound() const noexcept
    {
        return static_cast<std::size_t>(std::countr_zero(occupied_)) + 1;
    }

    [[nodiscard]] static std::size_t MinIndex(const std::vector<value_type> &bucket) noexcept
    {
        std::size_t best = 0;
        for (std::size_t index = 1; index < bucket.size(); ++index)
        {
            if (bucket[index].first < bucket[best].first)
            {
                best = index;
            }
        }
        return best;
    }

    void RequireNonempty() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("RadixHeap: empty queue");
        }
#endif
    }

    void Refill()
    {
        const auto index = FirstOutsideLowerBound();
        auto &source = buckets_[index];
        const Key next_bound = source[MinIndex(source)].first;
        std::array<std::size_t, kBucketCount> sizes{};
        for (const auto &item : source)
        {
            ++sizes[Bucket(item.first, next_bound)];
        }
        // 移動先はすべて下位の空バケット。確保が終わるまではキーの下限も変えない。
        for (std::size_t target = 0; target < index; ++target)
        {
            buckets_[target].reserve(sizes[target]);
        }
        lower_bound_ = next_bound;
        occupied_ &= ~(std::uint64_t{1} << (index - 1));
        for (auto &item : source)
        {
            const auto target = Bucket(item.first, lower_bound_);
            buckets_[target].push_back(std::move(item));
            MarkOccupied(target);
        }
        source.clear();
    }

    std::array<std::vector<value_type>, kBucketCount> buckets_;
    Key lower_bound_ = 0;
    std::uint64_t occupied_ = 0;
    std::size_t size_ = 0;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
