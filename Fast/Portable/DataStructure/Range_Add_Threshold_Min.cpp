// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Add_Threshold_Min.cpp
// 参考（固定キーと区間加算の用途）: https://atcoder.jp/contests/awc0100/tasks/awc0100_k
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_add_threshold_min.md
//
// API一覧
// RangeAddThresholdMin<Key>(keys,values,b=0): 固定キーとint64_t値を保持。b=0はおよそsqrt(n)、空構築可。
// 構築はO(n*log(b+1)+1)時間・O(n+1)領域。keyのコピーと比較はO(1)とする。
// size()/empty()/get(i): 長さ/空か/値。O(1)。add(l,r,delta): 半開区間へ加算。O(b+n/b+1)。
// minimum_at_least(l,r,key): キーがkey以上の最小値。なければnullopt。O(b+(n/b+1)*log(b+1))。
// minimum_at_least(key): 全体版。O((n/b+1)*log(b+1))。区間と位置は0始まり。
// 検査が有効なら範囲外またはint64_tを超える更新で例外を送り、更新前の内容を保つ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_ADD_THRESHOLD_MIN_DEFINED
#define SIRKIT_RANGE_ADD_THRESHOLD_MIN_DEFINED

template <std::totally_ordered Key>
class RangeAddThresholdMin
{
    using Value = std::int64_t;
    using Offset = std::uint64_t;

    static Value shifted(Value base, Offset offset) noexcept
    {
        // 実値が表現可能な限り、差分そのものがint64_tを超えても同じ実値へ戻る。
        return static_cast<Value>(static_cast<Offset>(base) + offset);
    }

    struct Block
    {
        std::size_t first;
        std::vector<Key> keys;
        std::vector<Value> base;
        std::vector<std::size_t> order;
        std::vector<Value> suffix_min;
        Value raw_max = 0;
        Offset shift = 0;

        Block(std::size_t begin, std::span<const Key> key_part, std::span<const Value> value_part)
            : first(begin), keys(key_part.begin(), key_part.end()), base(value_part.begin(), value_part.end()),
              order(keys.size()), suffix_min(keys.size())
        {
            std::iota(order.begin(), order.end(), std::size_t{0});
            const auto &fixed = std::as_const(keys);
            std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b)
            {
                return fixed[a] < fixed[b];
            });
            refresh();
        }

        [[nodiscard]] std::size_t size() const noexcept
        {
            return base.size();
        }

        [[nodiscard]] Value get(std::size_t i) const noexcept
        {
            return shifted(base[i], shift);
        }

        void refresh() noexcept
        {
            auto minimum = std::numeric_limits<Value>::max();
            auto maximum = std::numeric_limits<Value>::min();
            for (auto cursor = order.size(); cursor != 0;)
            {
                --cursor;
                const auto raw = base[order[cursor]];
                minimum = std::min(minimum, raw);
                maximum = std::max(maximum, raw);
                suffix_min[cursor] = minimum;
            }
            raw_max = maximum;
        }

        void add_part(std::size_t left, std::size_t right, Offset delta) noexcept
        {
            if (shift != 0)
            {
                for (auto &value : base)
                {
                    value = shifted(value, shift);
                }
                shift = 0;
            }
            for (auto i = left; i < right; ++i)
            {
                base[i] = shifted(base[i], delta);
            }
            refresh();
        }
    };

    std::size_t size_ = 0;
    std::size_t width_ = 1;
    std::vector<Block> blocks_;

    void check_range([[maybe_unused]] std::size_t left, [[maybe_unused]] std::size_t right) const
    {
    }

    template <class Self, class Action>
    static void visit_slices(Self &self, std::size_t left, std::size_t right, Action action)
    {
        if (left == right)
        {
            return;
        }
        const auto last = (right - 1) / self.width_;
        for (auto id = left / self.width_; id <= last; ++id)
        {
            auto &block = self.blocks_[id];
            const auto from = std::max(left, block.first) - block.first;
            const auto to = std::min(right - block.first, block.size());
            action(block, from, to, from == 0 && to == block.size());
        }
    }

public:
    RangeAddThresholdMin() = default;

    RangeAddThresholdMin(std::span<const Key> keys, std::span<const Value> values, std::size_t block_width = 0)
        : size_(values.size()), width_(block_width)
    {
        if (keys.size() != values.size())
        {
            throw std::invalid_argument("RangeAddThresholdMin: key/value lengths");
        }
        if (width_ == 0)
        {
            width_ = std::max<std::size_t>(1, static_cast<std::size_t>(std::sqrt(static_cast<double>(size_))));
        }
        blocks_.reserve(size_ / width_ + (size_ % width_ != 0));
        for (std::size_t first = 0; first < size_;)
        {
            const auto count = std::min(width_, size_ - first);
            blocks_.emplace_back(first, keys.subspan(first, count), values.subspan(first, count));
            first += count;
        }
    }

    RangeAddThresholdMin(const RangeAddThresholdMin &) = default;

    RangeAddThresholdMin(RangeAddThresholdMin &&other) noexcept
        : size_(std::exchange(other.size_, 0)), width_(std::exchange(other.width_, 1)), blocks_(std::move(other.blocks_))
    {
        other.blocks_.clear();
    }

    RangeAddThresholdMin &operator=(RangeAddThresholdMin other) noexcept
    {
        std::swap(size_, other.size_);
        std::swap(width_, other.width_);
        blocks_.swap(other.blocks_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size_ == 0;
    }

    [[nodiscard]] Value get(std::size_t position) const
    {
        return blocks_[position / width_].get(position % width_);
    }

    void add(std::size_t left, std::size_t right, Value delta)
    {
        check_range(left, right);
        if (delta == 0)
        {
            return;
        }
        const auto offset = static_cast<Offset>(delta);
        visit_slices(*this, left, right, [&](Block &block, std::size_t from, std::size_t to, bool full)
        {
            if (full)
            {
                block.shift += offset;
            }
            else
            {
                block.add_part(from, to, offset);
            }
        });
    }

    [[nodiscard]] std::optional<Value> minimum_at_least(std::size_t left, std::size_t right, const Key &threshold) const
    {
        check_range(left, right);
        std::optional<Value> best;
        const auto include = [&](Value candidate)
        {
            if (!best || candidate < *best)
            {
                best = candidate;
            }
        };
        visit_slices(*this, left, right, [&](const Block &block, std::size_t from, std::size_t to, bool full)
        {
            if (full)
            {
                const auto key_less = [&](std::size_t i, const Key &target)
                {
                    return block.keys[i] < target;
                };
                const auto match = std::lower_bound(block.order.begin(), block.order.end(), threshold, key_less);
                if (match != block.order.end())
                {
                    include(shifted(block.suffix_min[static_cast<std::size_t>(match - block.order.begin())], block.shift));
                }
            }
            else
            {
                for (auto i = from; i < to; ++i)
                {
                    if (!(block.keys[i] < threshold))
                    {
                        include(block.get(i));
                    }
                }
            }
        });
        return best;
    }

    [[nodiscard]] std::optional<Value> minimum_at_least(const Key &threshold) const
    {
        return minimum_at_least(0, size(), threshold);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
