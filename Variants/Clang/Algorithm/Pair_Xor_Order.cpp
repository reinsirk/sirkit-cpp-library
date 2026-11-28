// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Pair_Xor_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pair_xor_order.md
//
// API一覧
// PairXorOrder<T=uint64_t>(values): 符号なし整数列のコピーを整列する。O(n log(n+1))時間、O(n)領域。
// kth(k): i<jを満たす全組のvalues[i]^values[j]を重複込みで昇順に並べ、0始まりのk番目を返す。
// count_less(x): XOR値がx未満の組の個数。pair_count(): 全組の数、O(1)。
// 照会はO(n(1+B))時間・追加O(n)領域。Bは列の最小値と最大値のXORのビット幅。
// 要素数nはUINT32_MAX以下。型Tはbool以外の64ビット以下の符号なし整数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PAIR_XOR_ORDER_DEFINED
#define SIRKIT_PAIR_XOR_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{

inline void CheckPairXorSize(std::size_t size)
{
    if (size > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("PairXorOrder: too many values");
    }
}

}

template <std::unsigned_integral T = std::uint64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class PairXorOrder
{
    struct Range
    {
        std::uint32_t begin, end;

        [[nodiscard]] std::uint64_t size() const
        {
            return end - begin;
        }
    };

    struct Pair
    {
        Range first, second;
    };

    struct Bucket
    {
        std::vector<Pair> pairs;
        std::uint64_t count = 0;

        void add(Range first, Range second)
        {
            if (first.size() == 0 || second.size() == 0)
            {
                return;
            }
            const auto ways = first.begin == second.begin
                                  ? PairCount(first.size())
                                  : first.size() * second.size();
            if (ways != 0)
            {
                pairs.push_back({first, second});
                count += ways;
            }
        }

        void clear()
        {
            pairs.clear();
            count = 0;
        }
    };

public:
    explicit PairXorOrder(std::vector<T> values) : values_(std::move(values))
    {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckPairXorSize(values_.size());
        std::sort(values_.begin(), values_.end());
    }

    [[nodiscard]] std::uint64_t pair_count() const noexcept
    {
        return PairCount(values_.size());
    }

    [[nodiscard]] T kth(std::uint64_t k) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (k >= pair_count())
        {
            throw std::out_of_range("PairXorOrder: rank");
        }
#endif
        T value = 0;
        Search(BitWidth(), [&](T bit, std::uint64_t same)
               {
                   if (k < same)
                   {
                       return false;
                   }
                   k -= same;
                   value |= bit;
                   return true;
               });
        return value;
    }

    [[nodiscard]] std::uint64_t count_less(T bound) const
    {
        if (pair_count() == 0 || bound == 0)
        {
            return 0;
        }
        const int bits = BitWidth();
        if (std::bit_width(bound) > bits)
        {
            return pair_count();
        }
        std::uint64_t count = 0;
        Search(bits, [&](T bit, std::uint64_t same)
               {
                   if ((bound & bit) == 0)
                   {
                       return false;
                   }
                   count += same;
                   return true;
               });
        return count;
    }

private:
    [[nodiscard]] static std::uint64_t PairCount(std::uint64_t size) noexcept
    {
        return size * (size - 1) / 2;
    }

    [[nodiscard]] int BitWidth() const
    {
        return std::bit_width(static_cast<T>(values_.front() ^ values_.back()));
    }

    [[nodiscard]] std::pair<Range, Range> Split(Range range, T bit) const
    {
        auto it = std::partition_point(values_.begin() + range.begin, values_.begin() + range.end,
                                       [&](T value)
                                       {
                                           return (value & bit) == 0;
                                       });
        const auto middle = static_cast<std::uint32_t>(it - values_.begin());
        return {{range.begin, middle}, {middle, range.end}};
    }

    template <class ChooseOne>
    void Search(int bits, ChooseOne choose_one) const
    {
        const Range all{0, static_cast<std::uint32_t>(values_.size())};
        Bucket current, same, different;
        current.add(all, all);
        for (int level = bits - 1; level >= 0 && !current.pairs.empty(); --level)
        {
            const auto bit = static_cast<T>(T{1} << level);
            same.clear();
            different.clear();
            for (const auto &[first, second] : current.pairs)
            {
                const auto [a0, a1] = Split(first, bit);
                if (first.begin == second.begin)
                {
                    same.add(a0, a0);
                    same.add(a1, a1);
                    different.add(a0, a1);
                }
                else
                {
                    const auto [b0, b1] = Split(second, bit);
                    same.add(a0, b0);
                    same.add(a1, b1);
                    different.add(a0, b1);
                    different.add(a1, b0);
                }
            }
            Bucket &next = choose_one(bit, same.count) ? different : same;
            current.pairs.swap(next.pairs);
        }
    }

    std::vector<T> values_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
