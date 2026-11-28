// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Dynamic_Wavelet_Matrix.cpp
// SplitMix64の乱数生成: https://prng.di.unimi.it/splitmix64.c（LICENSES/LicenseRef-SplitMix64-Public-Domain.txt）
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_wavelet_matrix.md
//
// API一覧
// DynamicWaveletMatrix<T,Bits>(values = {}, seed = 0): 符号なし整数列の構築。O(Bits * (n+1))。
// size()/empty(): 長さ・空判定。O(1)。
// access(i)/insert(i,x)/erase(i)/set(i,x): i番目の取得・直前への挿入・削除・置換。eraseは削除した値を返す。
// kth_smallest(l,r,k): [l,r)で小さい方からk番目の値。添字と順位は0始まり。
// count(l,r,x)/count_less(l,r,x): [l,r)のうちxに等しい・x未満の要素数。
// 値は下位Bitsビットに収める。検索値には型Tの全域を指定できる。
// 各更新・検索は期待償却O(Bits * log(n+2))。nは列の長さ。領域O(Bits * (1 + 最大長/64))ワード。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_WAVELET_MATRIX_DEFINED
#define SIRKIT_DYNAMIC_WAVELET_MATRIX_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{

class WaveletDynamicBits
{
    using Index = std::size_t;

    struct Block
    {
        std::uint64_t bits = 0;
        std::uint64_t priority = 0;
        Index left = 0;
        Index right = 0;
        Index length = 0;
        Index ones = 0;
        int width = 0;
        int local_ones = 0;
    };

    static constexpr int kBlockWidth = 64;
    static constexpr int kMinWidth = 16;
    static constexpr int kBuildWidth = 48;

    std::vector<Block> blocks_;
    Index root_ = 0;
    Index free_ = 0;
    std::uint64_t state_ = 0;

    static std::uint64_t Mask(int width)
    {
        if (width == kBlockWidth)
        {
            return ~std::uint64_t{0};
        }
        return (std::uint64_t{1} << width) - 1;
    }

    static int CountBits(std::uint64_t bits)
    {
        // The arithmetic form can be inlined when the target has no popcount instruction.
        bits -= (bits >> 1) & UINT64_C(0x5555555555555555);
        bits = (bits & UINT64_C(0x3333333333333333)) + ((bits >> 2) & UINT64_C(0x3333333333333333));
        bits = (bits + (bits >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f);
        return static_cast<int>((bits * UINT64_C(0x0101010101010101)) >> 56);
    }

    Block &Get(Index index)
    {
        return blocks_[index - 1];
    }

    const Block &Get(Index index) const
    {
        return blocks_[index - 1];
    }

    [[nodiscard]] Index Length(Index index) const
    {
        return index ? Get(index).length : 0;
    }

    [[nodiscard]] Index Ones(Index index) const
    {
        return index ? Get(index).ones : 0;
    }

    void Pull(Index index)
    {
        Block &block = Get(index);
        block.length = Length(block.left) + block.width + Length(block.right);
        block.ones = Ones(block.left) + block.local_ones + Ones(block.right);
    }

    std::uint64_t NextPriority()
    {
        std::uint64_t value = (state_ += UINT64_C(0x9e3779b97f4a7c15));
        value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
        return value ^ (value >> 31);
    }

    Index Allocate(std::uint64_t bits, int width)
    {
        const int ones = CountBits(bits);
        Block block{bits, NextPriority(), 0, 0, static_cast<Index>(width), static_cast<Index>(ones), width, ones};
        if (free_)
        {
            const Index index = free_;
            free_ = Get(index).left;
            Get(index) = block;
            return index;
        }
        blocks_.push_back(block);
        return blocks_.size();
    }

    void Release(Index index)
    {
        Get(index).left = free_;
        free_ = index;
    }

    Index Merge(Index left, Index right)
    {
        if (!left || !right)
        {
            return left ? left : right;
        }
        if (Get(left).priority > Get(right).priority)
        {
            Get(left).right = Merge(Get(left).right, right);
            Pull(left);
            return left;
        }
        Get(right).left = Merge(left, Get(right).left);
        Pull(right);
        return right;
    }

    static std::uint64_t InsertBit(std::uint64_t bits, int offset, bool value)
    {
        const auto lower = Mask(offset);
        return (bits & lower) | (std::uint64_t(value) << offset) | ((bits & ~lower) << 1);
    }

    Index Insert(Index index, Index position, bool value, Index &rank)
    {
        Block &block = Get(index);
        const Index left = Length(block.left);
        if (position < left)
        {
            const Index child = Insert(block.left, position, value, rank);
            if (Get(child).priority > block.priority)
            {
                block.left = Get(child).right;
                Get(child).right = index;
                Pull(index);
                Pull(child);
                return child;
            }
            block.left = child;
        }
        else if (position > left + block.width)
        {
            rank += Ones(block.left) + block.local_ones;
            const Index child = Insert(block.right, position - left - block.width, value, rank);
            if (Get(child).priority > block.priority)
            {
                block.right = Get(child).left;
                Get(child).left = index;
                Pull(index);
                Pull(child);
                return child;
            }
            block.right = child;
        }
        else
        {
            const int offset = static_cast<int>(position - left);
            rank += Ones(block.left) + CountBits(block.bits & Mask(offset));
            if (block.width == kBlockWidth)
            {
                auto lower = block.bits & Mask(kBlockWidth / 2);
                auto upper = block.bits >> (kBlockWidth / 2);
                const bool insert_lower = offset < kBlockWidth / 2;
                if (insert_lower)
                {
                    lower = InsertBit(lower, offset, value);
                }
                else
                {
                    upper = InsertBit(upper, offset - kBlockWidth / 2, value);
                }
                const Index right = block.right;
                block.width = kBlockWidth / 2 + insert_lower;
                block.bits = lower;
                block.local_ones = CountBits(lower);
                block.right = 0;
                Pull(index);
                return Merge(Merge(index, Allocate(upper, kBlockWidth / 2 + !insert_lower)), right);
            }
            block.bits = InsertBit(block.bits, offset, value);
            ++block.width;
            block.local_ones += value;
        }
        Pull(index);
        return index;
    }

    struct Deletion
    {
        Index rank = 0;
        Index first = 0;
        bool bit = false;
        bool repack = false;
    };

    Index Erase(Index index, Index position, Index preceding, Deletion &deleted)
    {
        Block &block = Get(index);
        const Index left = Length(block.left);
        if (position < left)
        {
            block.left = Erase(block.left, position, preceding, deleted);
        }
        else if (position >= left + block.width)
        {
            deleted.rank += Ones(block.left) + block.local_ones;
            block.right = Erase(block.right, position - left - block.width, preceding + left + block.width, deleted);
        }
        else
        {
            const int offset = static_cast<int>(position - left);
            const auto lower = Mask(offset);
            deleted.rank += Ones(block.left) + CountBits(block.bits & lower);
            deleted.first = preceding + left;
            deleted.bit = (block.bits >> offset) & 1;
            if (--block.width == 0)
            {
                const Index joined = Merge(block.left, block.right);
                Release(index);
                return joined;
            }
            block.bits = (block.bits & lower) | ((block.bits >> 1) & ~lower);
            block.local_ones -= deleted.bit;
            deleted.repack = block.width < kMinWidth;
        }
        Pull(index);
        return index;
    }

    bool ReplaceBit(Index index, Index position, bool value, Index &rank)
    {
        Block &block = Get(index);
        const Index left = Length(block.left);
        bool old;
        if (position < left)
        {
            old = ReplaceBit(block.left, position, value, rank);
        }
        else if (position >= left + block.width)
        {
            rank += Ones(block.left) + block.local_ones;
            old = ReplaceBit(block.right, position - left - block.width, value, rank);
        }
        else
        {
            const int offset = static_cast<int>(position - left);
            rank += Ones(block.left) + CountBits(block.bits & Mask(offset));
            old = (block.bits >> offset) & 1;
            block.bits ^= std::uint64_t(old != value) << offset;
            block.local_ones += int(value) - int(old);
        }
        block.ones = block.ones - old + value;
        return old;
    }

    // Used only with the starting index of a block.
    Index RemoveBlock(Index index, Index first)
    {
        Block &block = Get(index);
        const Index left = Length(block.left);
        if (first < left)
        {
            block.left = RemoveBlock(block.left, first);
        }
        else if (first > left)
        {
            block.right = RemoveBlock(block.right, first - left - block.width);
        }
        else
        {
            const Index joined = Merge(block.left, block.right);
            Release(index);
            return joined;
        }
        Pull(index);
        return index;
    }

    void ReplaceBlock(Index index, Index first, std::uint64_t bits, int width)
    {
        Block &block = Get(index);
        const Index left = Length(block.left);
        if (first < left)
        {
            ReplaceBlock(block.left, first, bits, width);
        }
        else if (first > left)
        {
            ReplaceBlock(block.right, first - left - block.width, bits, width);
        }
        else
        {
            block.bits = bits;
            block.width = width;
            block.local_ones = CountBits(bits);
        }
        Pull(index);
    }

    // A shortened block is paired with its successor, or its predecessor at the end.
    void Repack(Index first)
    {
        auto block = Locate(first);
        Index next = first + Get(block.index).width;
        if (next == size())
        {
            if (first == 0)
            {
                return;
            }
            next = first;
            block = Locate(first - 1);
            first = next - block.offset - 1;
        }
        const Block &left = Get(block.index);
        const Block &right = Get(Locate(next).index);
        const int total = left.width + right.width;
        std::uint64_t lower = left.bits;
        if (left.width != kBlockWidth)
        {
            lower |= right.bits << left.width;
        }
        if (total <= kBlockWidth)
        {
            root_ = RemoveBlock(root_, next);
            ReplaceBlock(root_, first, lower, total);
            return;
        }
        const auto high = right.bits >> (kBlockWidth - left.width);
        const int width = total / 2;
        ReplaceBlock(root_, first, lower & Mask(width), width);
        ReplaceBlock(root_, first + width, (lower >> width) | (high << (kBlockWidth - width)), total - width);
    }

    void PullTree(Index index)
    {
        if (index)
        {
            PullTree(Get(index).left);
            PullTree(Get(index).right);
            Pull(index);
        }
    }

public:
    struct Location
    {
        Index rank;
        bool bit;
        Index index;
        int offset;
    };

    [[nodiscard]] Location Locate(Index position) const
    {
        Index rank = 0;
        Index index = root_;
        while (index)
        {
            const Block &block = Get(index);
            const Index left = Length(block.left);
            if (position < left)
            {
                index = block.left;
                continue;
            }
            rank += Ones(block.left);
            position -= left;
            if (position < static_cast<Index>(block.width))
            {
                const int offset = static_cast<int>(position);
                return {rank + CountBits(block.bits & Mask(offset)), bool((block.bits >> offset) & 1), index, offset};
            }
            rank += block.local_ones;
            position -= block.width;
            index = block.right;
        }
        return {rank, false, 0, 0};
    }

    [[nodiscard]] Index size() const
    {
        return Length(root_);
    }

    [[nodiscard]] Index zeroes() const
    {
        return size() - Ones(root_);
    }

    void Build(std::size_t length, std::uint64_t seed, auto bit_at)
    {
        state_ = seed;
        const std::size_t block_count = length / kBuildWidth + (length % kBuildWidth != 0);
        blocks_.reserve(block_count);
        std::vector<Index> spine;
        for (std::size_t first = 0; first < length;)
        {
            const int width = static_cast<int>(std::min<std::size_t>(length - first, kBuildWidth));
            std::uint64_t bits = 0;
            for (int offset = 0; offset < width; ++offset)
            {
                bits |= std::uint64_t(bit_at(first + offset)) << offset;
            }
            const Index node = Allocate(bits, width);
            Index left = 0;
            while (!spine.empty() && Get(spine.back()).priority < Get(node).priority)
            {
                left = spine.back();
                spine.pop_back();
            }
            Get(node).left = left;
            if (spine.empty())
            {
                root_ = node;
            }
            else
            {
                Get(spine.back()).right = node;
            }
            spine.push_back(node);
            first += width;
        }
        PullTree(root_);
    }

    void PrepareInsert()
    {
        if (!free_ && blocks_.size() == blocks_.capacity())
        {
            const auto size = blocks_.size();
            const auto available = blocks_.max_size() - size;
            blocks_.reserve(size + std::max<std::size_t>(1, std::min(size, available)));
        }
    }

    Index insert(Index position, bool value)
    {
        if (!root_)
        {
            root_ = Allocate(value, 1);
            return 0;
        }
        Index rank = 0;
        root_ = Insert(root_, position, value, rank);
        return rank;
    }

    std::pair<Index, bool> replace_bit(Index position, bool value)
    {
        Index rank = 0;
        const bool old = ReplaceBit(root_, position, value, rank);
        return {rank, old};
    }

    std::pair<Index, bool> erase(Index position)
    {
        Deletion deleted;
        root_ = Erase(root_, position, 0, deleted);
        if (deleted.repack)
        {
            Repack(deleted.first);
        }
        return {deleted.rank, deleted.bit};
    }
};

}

template <std::unsigned_integral T = std::uint32_t, int Bits = std::numeric_limits<T>::digits>
    requires(!std::same_as<T, bool> && Bits > 0 && Bits <= std::numeric_limits<T>::digits)
class DynamicWaveletMatrix
{
    using Index = std::size_t;

public:
    explicit DynamicWaveletMatrix(std::span<const T> values = {}, std::uint64_t seed = 0)
    {
        for (T value : values)
        {
            CheckValue(value);
        }
        std::vector<T> current(values.begin(), values.end());
        std::vector<T> next(values.size());
        for (int level = Bits - 1; level >= 0; --level)
        {
            auto &row = rows_[level];
            row.Build(current.size(), seed + level, [&](Index index)
                      {
                          return (current[index] >> level) & T{1};
                      });
            if (level != 0)
            {
                Index zero = 0;
                Index one = row.zeroes();
                for (T value : current)
                {
                    auto &position = ((value >> level) & T{1}) ? one : zero;
                    next[position++] = value;
                }
                current.swap(next);
            }
        }
    }

    DynamicWaveletMatrix(const DynamicWaveletMatrix &) = default;
    DynamicWaveletMatrix(DynamicWaveletMatrix &&other) noexcept
    {
        rows_.swap(other.rows_);
    }
    DynamicWaveletMatrix &operator=(DynamicWaveletMatrix other) noexcept
    {
        rows_.swap(other.rows_);
        return *this;
    }

    [[nodiscard]] Index size() const
    {
        return rows_.back().size();
    }

    [[nodiscard]] bool empty() const
    {
        return size() == 0;
    }

    [[nodiscard]] T access(Index position) const
    {
        CheckPosition(position);
        T value = 0;
        for (int level = Bits - 1; level >= 0; --level)
        {
            const auto &row = rows_[level];
            const auto info = row.Locate(position);
            if (info.bit)
            {
                value |= T{1} << level;
            }
            position = Map(position, info.rank, row.zeroes(), info.bit);
        }
        return value;
    }

    void insert(Index position, T value)
    {
        CheckValue(value);
        PrepareInsert();
        InsertValue(position, value);
    }

    T erase(Index position)
    {
        CheckPosition(position);
        return EraseValue(position);
    }

    void set(Index position, T value)
    {
        CheckPosition(position);
        CheckValue(value);
        PrepareInsert();
        Index destination = position;
        for (int level = Bits - 1; level >= 0; --level)
        {
            auto &row = rows_[level];
            const bool bit = (value >> level) & T{1};
            const Index zeroes = row.zeroes();
            if (position == destination)
            {
                const auto [rank, old] = row.replace_bit(position, bit);
                position = Map(position, rank, zeroes, old);
                destination = Map(destination, rank, zeroes - !old, bit);
            }
            else
            {
                const auto [rank, old] = row.erase(position);
                position = Map(position, rank, zeroes, old);
                destination = Map(destination, row.insert(destination, bit), zeroes - !old, bit);
            }
        }
    }

    [[nodiscard]] T kth_smallest(Index first, Index last, Index k) const
    {
        CheckRange(first, last);
        T value = 0;
        for (int level = Bits - 1; level >= 0; --level)
        {
            const auto &row = rows_[level];
            const Index first_rank = row.Locate(first).rank;
            const Index last_rank = row.Locate(last).rank;
            const Index zero_count = (last - last_rank) - (first - first_rank);
            const bool bit = k >= zero_count;
            if (bit)
            {
                k -= zero_count;
                value |= T{1} << level;
            }
            first = Map(first, first_rank, row.zeroes(), bit);
            last = Map(last, last_rank, row.zeroes(), bit);
        }
        return value;
    }

    [[nodiscard]] Index count(Index first, Index last, T value) const
    {
        CheckRange(first, last);
        if (!Fits(value))
        {
            return 0;
        }
        for (int level = Bits - 1; level >= 0 && first != last; --level)
        {
            const auto &row = rows_[level];
            const bool bit = (value >> level) & T{1};
            first = Map(first, row.Locate(first).rank, row.zeroes(), bit);
            last = Map(last, row.Locate(last).rank, row.zeroes(), bit);
        }
        return last - first;
    }

    [[nodiscard]] Index count_less(Index first, Index last, T bound) const
    {
        CheckRange(first, last);
        if (!Fits(bound))
        {
            return last - first;
        }
        Index count = 0;
        for (int level = Bits - 1; level >= 0 && first != last; --level)
        {
            const auto &row = rows_[level];
            const bool bit = (bound >> level) & T{1};
            const Index first_rank = row.Locate(first).rank;
            const Index last_rank = row.Locate(last).rank;
            if (bit)
            {
                count += (last - last_rank) - (first - first_rank);
            }
            first = Map(first, first_rank, row.zeroes(), bit);
            last = Map(last, last_rank, row.zeroes(), bit);
        }
        return count;
    }

private:
    static Index Map(Index position, Index rank, Index zeroes, bool bit)
    {
        return bit ? zeroes + rank : position - rank;
    }

    static bool Fits(T value)
    {
        return (value >> (Bits - 1)) <= 1;
    }

    static void CheckValue([[maybe_unused]] T value)
    {
    }

    void CheckPosition([[maybe_unused]] Index position) const
    {
    }

    void CheckRange([[maybe_unused]] Index first, [[maybe_unused]] Index last) const
    {
    }

    void PrepareInsert()
    {
        for (auto &row : rows_)
        {
            row.PrepareInsert();
        }
    }

    void InsertValue(Index position, T value)
    {
        for (int level = Bits - 1; level >= 0; --level)
        {
            auto &row = rows_[level];
            const bool bit = (value >> level) & T{1};
            const Index zeroes = row.zeroes();
            position = Map(position, row.insert(position, bit), zeroes, bit);
        }
    }

    T EraseValue(Index position)
    {
        T value = 0;
        for (int level = Bits - 1; level >= 0; --level)
        {
            auto &row = rows_[level];
            const Index zeroes = row.zeroes();
            const auto [rank, bit] = row.erase(position);
            if (bit)
            {
                value |= T{1} << level;
            }
            position = Map(position, rank, zeroes, bit);
        }
        return value;
    }

    std::array</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::WaveletDynamicBits, Bits> rows_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
