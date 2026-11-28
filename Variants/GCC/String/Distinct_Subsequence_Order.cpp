// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: String/Distinct_Subsequence_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/distinct_subsequence_order.md
//
// API一覧
// DistinctSubsequenceOrder(text): 空でない相異なる部分列を、符号なしバイトの辞書順で索引化する。O(n+1)時間・領域。
// size(): 元の列の長さ。count_distinct(): 空でない部分列の種類数。各O(1)。
// kth_smallest(rank): 辞書順で1始まりのrank番目の部分列を返す。範囲外ならnullopt。
// rank_of(pattern): 空でない部分列の辞書順順位。該当しなければnullopt。
// prefix_count(prefix): prefixで始まる空でない部分列の種類数。空のprefixなら全種類数。
// 個数とrank_ofの結果はUINT64_MAXで上限をとる。kth_smallestは表せる順位まで正確。
// byte種類数σ、入力または出力の長さkに対し、kth_smallest/rank_ofはO(σk log(n+2)+1)、prefix_countはO(k log(n+2)+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DISTINCT_SUBSEQUENCE_ORDER_DEFINED
#define SIRKIT_DISTINCT_SUBSEQUENCE_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr std::uint64_t kSubsequenceCountCap = std::numeric_limits<std::uint64_t>::max();

inline constexpr std::uint64_t add_subsequence_counts(std::uint64_t left, std::uint64_t right) noexcept
{
    return left + std::min(kSubsequenceCountCap - left, right);
}

inline std::size_t subsequence_slot_count(std::size_t size)
{
    if (size >= std::vector<std::uint64_t>().max_size())
    {
        throw std::length_error("DistinctSubsequenceOrder: text is too long");
    }
    return size + 1;
}
} // namespace reinsirk::detail

class DistinctSubsequenceOrder
{
private:
    static constexpr unsigned kAlphabetSize = 256;

    struct Index
    {
        explicit Index(std::string_view text) : suffix_count(detail::subsequence_slot_count(text.size()))
        {
            for (std::size_t i = 0; i < text.size(); ++i)
            {
                positions[static_cast<unsigned char>(text[i])].push_back(i);
            }
            for (unsigned code = 0; code < kAlphabetSize; ++code)
            {
                if (!positions[code].empty())
                {
                    alphabet.push_back(static_cast<unsigned char>(code));
                }
            }
            std::array<std::uint64_t, kAlphabetSize> starting_count{};
            for (std::size_t i = text.size(); i != 0;)
            {
                --i;
                const auto following = suffix_count[i + 1];
                if (following == detail::kSubsequenceCountCap)
                {
                    // 接尾辞を延ばしても、作れる部分列は減らない。
                    suffix_count[i] = following;
                    continue;
                }
                auto &branch = starting_count[static_cast<unsigned char>(text[i])];
                suffix_count[i] = detail::add_subsequence_counts(following - branch, following + 1);
                branch = following + 1;
            }
        }

        [[nodiscard]] std::size_t next_position(unsigned char symbol, std::size_t start) const
        {
            const auto &row = positions[symbol];
            const auto next = std::lower_bound(row.begin(), row.end(), start);
            return next == row.end() ? suffix_count.size() - 1 : *next;
        }

        [[nodiscard]] std::uint64_t continuation_count(std::size_t position) const noexcept
        {
            return detail::add_subsequence_counts(1, suffix_count[position + 1]);
        }

        std::array<std::vector<std::size_t>, kAlphabetSize> positions;
        std::vector<unsigned char> alphabet;
        std::vector<std::uint64_t> suffix_count;
    };

public:
    explicit DistinctSubsequenceOrder(std::string_view text) : index_(std::make_shared<Index>(text))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return index_ ? index_->suffix_count.size() - 1 : 0;
    }

    [[nodiscard]] std::uint64_t count_distinct() const noexcept
    {
        return index_ ? index_->suffix_count.front() : 0;
    }

    [[nodiscard]] std::optional<std::string> kth_smallest(std::uint64_t rank) const
    {
        if (rank == 0 || rank > count_distinct())
        {
            return std::nullopt;
        }
        std::string answer;
        std::size_t start = 0;
        while (rank != 0)
        {
            // 残りの順位は、いずれかのバイトで始まる枝に必ず含まれる。
            for (std::size_t i = 0;; ++i)
            {
                const auto symbol = index_->alphabet[i];
                const auto next = index_->next_position(symbol, start);
                if (next == size())
                {
                    continue;
                }
                const auto branch = index_->continuation_count(next);
                if (rank <= branch)
                {
                    answer.push_back(static_cast<char>(symbol));
                    start = next + 1;
                    --rank;
                    break;
                }
                rank -= branch;
            }
        }
        return answer;
    }

    [[nodiscard]] std::optional<std::uint64_t> rank_of(std::string_view pattern) const
    {
        if (pattern.empty() || !index_)
        {
            return std::nullopt;
        }
        std::size_t start = 0;
        std::uint64_t rank = 0;
        for (const char byte : pattern)
        {
            const auto symbol = static_cast<unsigned char>(byte);
            for (const auto smaller : index_->alphabet)
            {
                if (smaller >= symbol)
                {
                    break;
                }
                const auto next = index_->next_position(smaller, start);
                if (next != size())
                {
                    rank = detail::add_subsequence_counts(rank, index_->continuation_count(next));
                }
            }
            const auto next = index_->next_position(symbol, start);
            if (next == size())
            {
                return std::nullopt;
            }
            rank = detail::add_subsequence_counts(rank, 1);
            start = next + 1;
        }
        return rank;
    }

    [[nodiscard]] std::uint64_t prefix_count(std::string_view prefix) const
    {
        if (prefix.empty())
        {
            return count_distinct();
        }
        if (!index_)
        {
            return 0;
        }
        std::size_t start = 0;
        for (const char byte : prefix)
        {
            const auto next = index_->next_position(static_cast<unsigned char>(byte), start);
            if (next == size())
            {
                return 0;
            }
            start = next + 1;
        }
        return index_->continuation_count(start - 1);
    }

private:
    // 索引は読み取り専用。同じ文字列を索引化したコピーが共用する。
    std::shared_ptr<const Index> index_;
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
