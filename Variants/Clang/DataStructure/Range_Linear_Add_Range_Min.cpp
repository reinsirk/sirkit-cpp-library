// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Range_Linear_Add_Range_Min.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_linear_add_range_min.md
//
// API一覧
// RangeLinearAddRangeMin(values): int64_tの列。O(n+1)時間・領域、n<=INT_MAX。
// range_add(l,r,b,c): 各i in [l,r)にb*i+cを加える。空区間は何もしない。O(√n+1)。
// range_min(l,r): [l,r)の最小値。空区間はnullopt。O((√n+1)log(n+2))。
// size(): 要素数。O(1)。添字は0始まり。
// 各操作後の要素はint64_tに収まること。中間の積や累積係数が64ビットを超えても可。
// 凸包の比較・値の計算は標準C++の整数演算で厳密に行う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_LINEAR_ADD_RANGE_MIN_DEFINED
#define SIRKIT_RANGE_LINEAR_ADD_RANGE_MIN_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckRangeLinearAddRangeMinSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("RangeLinearAddRangeMin: too many values");
    }
}
}

class RangeLinearAddRangeMin
{
public:
    explicit RangeLinearAddRangeMin(std::span<const std::int64_t> input = {})
    {
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckRangeLinearAddRangeMinSize(input.size());
        if (input.empty())
        {
            return;
        }
        values_.assign(input.begin(), input.end());
        const int n = size();
        block_size_ = static_cast<int>(std::sqrt(n));
        blocks_.reserve((n - 1) / block_size_ + 1);
        for (int begin = 0; begin < n;)
        {
            const int end = begin + std::min(block_size_, n - begin);
            auto &block = blocks_.emplace_back(begin, end);
            block.hull.reserve(end - begin);
            Rebuild(block);
            begin = end;
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(values_.size());
    }

    void range_add(int left, int right, std::int64_t b, std::int64_t c)
    {
        CheckRange(left, right);
        if (left == right)
        {
            return;
        }

        const auto step = static_cast<std::uint64_t>(b);
        const auto intercept = static_cast<std::uint64_t>(c);
        const int last = (right - 1) / block_size_;
        for (int i = left / block_size_; i <= last; ++i)
        {
            auto &block = blocks_[i];
            const int begin = std::max(left, block.begin);
            const int end = std::min(right, block.end);
            const auto first = step * static_cast<std::uint64_t>(begin) + intercept;
            if (begin == block.begin && end == block.end)
            {
                block.slope.add(b);
                block.offset += first;
            }
            else
            {
                if (block.slope.low != 0 || block.offset != 0)
                {
                    AddValues(block.begin, block.end, block.slope.low, block.offset);
                }
                block.slope = {};
                block.offset = 0;
                AddValues(begin, end, step, first);
                Rebuild(block);
            }
        }
    }

    [[nodiscard]] std::optional<std::int64_t> range_min(int left, int right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return std::nullopt;
        }

        auto result = std::numeric_limits<std::int64_t>::max();
        const int last = (right - 1) / block_size_;
        for (int i = left / block_size_; i <= last; ++i)
        {
            const auto &block = blocks_[i];
            const int begin = std::max(left, block.begin);
            const int end = std::min(right, block.end);
            if (begin == block.begin && end == block.end)
            {
                const auto after = std::upper_bound(
                    block.hull.begin() + 1, block.hull.end(), block.slope,
                    [](const WideSlope &slope, const HullVertex &vertex)
                    {
                        return vertex.cutoff < slope;
                    });
                const auto position = (after - 1)->position;
                result = std::min(result, ValueWithDelta(values_[position], DeltaAt(block, position)));
            }
            else
            {
                auto delta = DeltaAt(block, begin);
                for (int position = begin; position < end; ++position)
                {
                    result = std::min(result, ValueWithDelta(values_[position], delta));
                    delta += block.slope.low;
                }
            }
        }
        return result;
    }

private:
    struct WideSlope
    {
        std::uint64_t high = 0, low = 0;

        void add(std::int64_t value) noexcept
        {
            const auto previous = low;
            low += static_cast<std::uint64_t>(value);
            high += static_cast<std::uint64_t>(low < previous);
            high -= static_cast<std::uint64_t>(value < 0);
        }

        bool operator<(const WideSlope &other) const noexcept
        {
            if (high != other.high)
            {
                return std::bit_cast<std::int64_t>(high) < std::bit_cast<std::int64_t>(other.high);
            }
            return low < other.low;
        }
    };

    struct HullVertex
    {
        int position;
        WideSlope cutoff;
    };

    struct Block
    {
        Block(int begin, int end) : begin(begin), end(end)
        {
        }

        int begin, end;
        std::vector<HullVertex> hull;
        WideSlope slope{};
        std::uint64_t offset = 0;
    };

    std::vector<std::int64_t> values_;
    std::vector<Block> blocks_;
    int block_size_ = 1;

    void CheckRange(int left, int right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left < 0 || left > right || right > size())
        {
            throw std::out_of_range("RangeLinearAddRangeMin: invalid range");
        }
#else
        (void)left;
        (void)right;
#endif
    }

    static std::int64_t ValueWithDelta(std::int64_t value, std::uint64_t delta) noexcept
    {
        return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(value) + delta);
    }

    static std::uint64_t DeltaAt(const Block &block, int position) noexcept
    {
        return block.offset + block.slope.low * static_cast<std::uint64_t>(position - block.begin);
    }

    void AddValues(int begin, int end, std::uint64_t step, std::uint64_t delta) noexcept
    {
        for (int i = begin; i < end; ++i)
        {
            values_[i] = ValueWithDelta(values_[i], delta);
            delta += step;
        }
    }

    // moving側がfixed側以下になる最大の整数傾き。差は最大2^64-1。
    static WideSlope Cutoff(std::int64_t fixed, std::int64_t moving, std::uint64_t distance) noexcept
    {
        const auto a = static_cast<std::uint64_t>(fixed);
        const auto b = static_cast<std::uint64_t>(moving);
        if (fixed >= moving)
        {
            return {0, (a - b) / distance};
        }
        const auto difference = b - a;
        const auto ceiling = difference / distance + (difference % distance != 0);
        return {std::numeric_limits<std::uint64_t>::max(), std::uint64_t{0} - ceiling};
    }

    void Rebuild(Block &block)
    {
        auto &hull = block.hull;
        hull.clear();
        hull.push_back({block.begin, {}});
        for (int i = block.begin + 1; i < block.end; ++i)
        {
            auto previous = hull.back().position;
            auto cutoff = Cutoff(values_[previous], values_[i], i - previous);
            // 交点を降順にそろえ、整数傾きの範囲で最小にならない点を除く。
            while (hull.size() > 1 && !(cutoff < hull.back().cutoff))
            {
                hull.pop_back();
                previous = hull.back().position;
                cutoff = Cutoff(values_[previous], values_[i], i - previous);
            }
            hull.push_back({i, cutoff});
        }
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
