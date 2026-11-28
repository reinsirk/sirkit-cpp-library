#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/point_set_range_frequency

// Library: DataStructure/Dynamic_Wavelet_Matrix.cpp
// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
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


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif


#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

struct Operation
{
    int first, last;
    std::uint32_t value;
    bool assign;
};

constexpr auto kAbsent = std::numeric_limits<std::uint32_t>::max();

template <int Bits>
void Solve(const std::vector<std::uint32_t> &values, const std::vector<Operation> &operations)
{
    sirkit::DynamicWaveletMatrix<std::uint32_t, Bits> matrix(values);
    for (const auto &op : operations)
    {
        if (op.assign)
        {
            matrix.set(op.first, op.value);
        }
        else
        {
            verify_out << (op.value == kAbsent ? 0 : matrix.count(op.first, op.last, op.value)) << '\n';
        }
    }
}

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<std::uint32_t> values(n);
    std::vector<std::uint32_t> keys;
    keys.reserve(static_cast<std::size_t>(n) + q);
    for (auto &value : values)
    {
        verify_in >> value;
        keys.push_back(value);
    }

    std::vector<Operation> operations(q);
    for (auto &op : operations)
    {
        unsigned type;
        verify_in >> type >> op.first;
        op.assign = type == 0;
        if (op.assign)
        {
            verify_in >> op.value;
            keys.push_back(op.value);
        }
        else
        {
            verify_in >> op.last >> op.value;
        }
    }

    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    const auto encode = [&](std::uint32_t value)
    {
        const auto it = std::lower_bound(keys.begin(), keys.end(), value);
        return it != keys.end() && *it == value ? static_cast<std::uint32_t>(it - keys.begin()) : kAbsent;
    };
    for (auto &value : values)
    {
        value = encode(value);
    }
    for (auto &op : operations)
    {
        op.value = encode(op.value);
    }
    if (keys.size() <= (1U << 8))
    {
        Solve<8>(values, operations);
    }
    else if (keys.size() <= (1U << 16))
    {
        Solve<16>(values, operations);
    }
    else
    {
        Solve<19>(values, operations);
    }
}
