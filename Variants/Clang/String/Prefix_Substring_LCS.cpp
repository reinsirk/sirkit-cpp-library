// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Prefix_Substring_LCS.cpp
// 参考: A. Tiskin, Semi-local string comparison: Algorithmic techniques and applications
// (https://arxiv.org/abs/0707.3619) のseaweed法。コードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prefix_substring_lcs.md
//
// API一覧
// PrefixSubstringLCS(s, t): 文字列を前処理する。比較はバイト単位。空文字列も可。
// length(a, b, c): s[0,a)とt[b,c)の最長共通部分列の長さ。0始まり、空区間では0。
// 通常版で添字が範囲外、またはb>cならout_of_range。
// M=|s|、N=|t|が正なら、前処理O(MN log(N+1))時間・O(M(N/64+1) log(N+1))領域。
// 問い合わせはO(log(N+1)+1)。前処理の作業領域はO(N)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_SUBSTRING_LCS_DEFINED
#define SIRKIT_PREFIX_SUBSTRING_LCS_DEFINED

class PrefixSubstringLCS
{
    using Index = std::size_t;
    static constexpr Index kWordBits = 64;

    struct RankBlock
    {
        std::uint64_t bits = 0;
        Index before = 0;
    };

    struct Level
    {
        std::vector<RankBlock> blocks;
        Index zero_count;

        Level(const std::vector<Index> &values, std::vector<Index> &next, Index bit)
            : blocks(values.size() / kWordBits + 1)
        {
            for (Index i = 0; i < values.size(); ++i)
            {
                const auto one = static_cast<std::uint64_t>((values[i] >> bit) & 1);
                blocks[i / kWordBits].bits |= one << (i % kWordBits);
            }
            Index ones = 0;
            for (auto &block : blocks)
            {
                block.before = ones;
                ones += static_cast<Index>(std::popcount(block.bits));
            }
            zero_count = values.size() - ones;

            Index zero_position = 0, one_position = zero_count;
            for (const auto value : values)
            {
                auto &position = ((value >> bit) & 1) != 0 ? one_position : zero_position;
                next[position++] = value;
            }
        }

        Index Ones(Index end) const
        {
            const auto &block = blocks[end / kWordBits];
            const auto mask = (std::uint64_t{1} << (end % kWordBits)) - 1;
            return block.before + static_cast<Index>(std::popcount(block.bits & mask));
        }
    };

public:
    PrefixSubstringLCS(std::string_view s, std::string_view t) : prefix_size_(s.size()), substring_size_(t.size())
    {
        if (s.empty() || t.empty())
        {
            return;
        }
        std::vector<Index> thresholds(t.size()), ordered(t.size()), next(t.size());
        std::iota(thresholds.begin(), thresholds.end(), Index{1});
        const auto bits = std::bit_width(t.size());
        rows_.reserve(s.size());

        for (const char symbol : s)
        {
            Index left = 0;
            for (Index j = 0; j < t.size(); ++j)
            {
                // 左端bごとの横方向DP差分は、しきい値がb以下なら1。
                // 一致マスは交換し、不一致マスは小さいしきい値を同行の次のマスへ渡す。
                if (symbol == t[j] || left > thresholds[j])
                {
                    std::swap(left, thresholds[j]);
                }
            }

            // 各行をウェーブレット行列に変換し、区間内のしきい値を数える。
            auto &levels = rows_.emplace_back();
            levels.reserve(bits);
            ordered = thresholds;
            for (auto bit = bits; bit != 0; --bit)
            {
                levels.emplace_back(ordered, next, bit - 1);
                ordered.swap(next);
            }
        }
    }

    std::size_t length(std::size_t a, std::size_t b, std::size_t c) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (a > prefix_size_ || b > c || c > substring_size_)
        {
            throw std::out_of_range("PrefixSubstringLCS: invalid prefix or substring interval");
        }
#endif
        if (a == 0 || b == c)
        {
            return 0;
        }
        const auto &row = rows_[a - 1];
        const auto limit = b + 1;
        Index first = b, last = c, count = 0, bit = row.size();
        for (const auto &layer : row)
        {
            --bit;
            const auto first_one = layer.Ones(first), last_one = layer.Ones(last);
            if (((limit >> bit) & 1) != 0)
            {
                count += (last - first) - (last_one - first_one);
                first = layer.zero_count + first_one;
                last = layer.zero_count + last_one;
            }
            else
            {
                first -= first_one;
                last -= last_one;
            }
        }
        return count;
    }

private:
    [[maybe_unused]] std::size_t prefix_size_;
    [[maybe_unused]] std::size_t substring_size_;
    std::vector<std::vector<Level>> rows_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
