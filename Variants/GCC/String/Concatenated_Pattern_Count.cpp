// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: String/Concatenated_Pattern_Count.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/concatenated_pattern_count.md
//
// API一覧
// [ConcatenatedPatternCounter] 固定パターンの出現回数を、文字列を展開せずに管理する。重なった出現も数える。
// ConcatenatedPatternCounter(pattern,modulus=998244353): 空でないバイト列と正の法を指定。O(m*a)時間・領域。
// summarize(text): 文字列の要約Summaryを構築。空文字列は連結の単位元。O(m*(|text|+1))時間・O(m)領域。
// concatenate(left,right): 連結left+rightの要約。入力は変更しない。O(m)時間・領域。
// count(summary): パターンの出現回数を法modulusで返す。O(1)時間・領域。
// 要約は作成した個体とそのコピーで使える。mはパターン長、aはパターンの相異なるバイト数（1..256）。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CONCATENATED_PATTERN_COUNT_DEFINED
#define SIRKIT_CONCATENATED_PATTERN_COUNT_DEFINED

namespace detail
{
inline std::size_t concatenated_pattern_table_size(std::size_t states, std::size_t symbols,
                                                  std::size_t capacity)
{
    const auto limit = std::min<std::size_t>(std::numeric_limits<std::uint32_t>::max(), capacity / symbols);
    if (states > limit)
    {
        throw std::length_error("ConcatenatedPatternCounter: pattern too long");
    }
    return states * symbols;
}
} // namespace detail

class ConcatenatedPatternCounter
{
    struct Transition
    {
        std::uint32_t state = 0;
        std::uint32_t matches = 0;
    };

    struct Profile
    {
        static constexpr auto kMissingSymbol = std::numeric_limits<std::uint16_t>::max();

        Profile(std::string_view pattern, std::uint32_t modulus_value)
            : length(pattern.size()), modulus(modulus_value)
        {
            if (pattern.empty() || modulus == 0)
            {
                throw std::invalid_argument("ConcatenatedPatternCounter: empty pattern or zero modulus");
            }
            column.fill(kMissingSymbol);
            for (const char character : pattern)
            {
                const auto byte = static_cast<unsigned char>(character);
                if (column[byte] == kMissingSymbol)
                {
                    column[byte] = symbol_count++;
                }
            }

            transitions.resize(detail::concatenated_pattern_table_size(length, symbol_count, transitions.max_size()));
            // 行iは長さiの接頭辞が一致した状態。異なるバイトは、直前の最長の真の接頭尾辞と同じように処理する。
            const auto single_match = std::uint32_t{1} % modulus;
            std::uint32_t border = 0;
            for (std::size_t i = 0; i < length; ++i)
            {
                const auto row = i * symbol_count;
                if (i != 0)
                {
                    const auto fallback_row = static_cast<std::size_t>(border) * symbol_count;
                    std::copy_n(transitions.begin() + fallback_row, symbol_count, transitions.begin() + row);
                }
                const auto byte = static_cast<unsigned char>(pattern[i]);
                auto &match = transitions[row + column[byte]];
                border = match.state;
                match = i + 1 == length ? Transition{border, single_match}
                                        : Transition{static_cast<std::uint32_t>(i + 1), 0};
            }
        }

        [[nodiscard]] Transition next(std::uint32_t state, unsigned char byte) const noexcept
        {
            const auto symbol = column[byte];
            if (symbol == kMissingSymbol)
            {
                return {};
            }
            return transitions[static_cast<std::size_t>(state) * symbol_count + symbol];
        }

        [[nodiscard]] std::uint32_t add(std::uint32_t a, std::uint32_t b) const noexcept
        {
            const auto sum = static_cast<std::uint64_t>(a) + b;
            return static_cast<std::uint32_t>(sum < modulus ? sum : sum - modulus);
        }

        std::size_t length;
        std::uint32_t modulus;
        std::uint16_t symbol_count = 0;
        std::array<std::uint16_t, 256> column;
        std::vector<Transition> transitions;
    };

  public:
    class Summary
    {
      public:
        Summary(const Summary &) = default;
        Summary(Summary &&) noexcept = default;

        Summary &operator=(Summary other) noexcept
        {
            transitions_.swap(other.transitions_);
            profile_.swap(other.profile_);
            return *this;
        }

      private:
        friend class ConcatenatedPatternCounter;

        explicit Summary(const std::shared_ptr<const Profile> &profile)
            : transitions_(profile->length), profile_(profile)
        {
        }

        std::vector<Transition> transitions_;
        std::shared_ptr<const Profile> profile_;
    };

    explicit ConcatenatedPatternCounter(std::string_view pattern, std::uint32_t modulus = 998'244'353)
        : profile_(std::make_shared<const Profile>(pattern, modulus))
    {
    }

    ConcatenatedPatternCounter(const ConcatenatedPatternCounter &) noexcept = default;
    ConcatenatedPatternCounter &operator=(const ConcatenatedPatternCounter &) noexcept = default;

    [[nodiscard]] Summary summarize(std::string_view text) const
    {
        Summary result(profile_);
        for (std::size_t i = 0; i < profile_->length; ++i)
        {
            Transition combined{static_cast<std::uint32_t>(i), 0};
            for (const char character : text)
            {
                const auto step = profile_->next(combined.state, static_cast<unsigned char>(character));
                combined = {step.state, profile_->add(combined.matches, step.matches)};
            }
            result.transitions_[i] = combined;
        }
        return result;
    }

    [[nodiscard]] Summary concatenate(const Summary &left, const Summary &right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        check(left);
        check(right);
#endif
        Summary result(profile_);
        for (std::size_t i = 0; i < profile_->length; ++i)
        {
            const auto a = left.transitions_[i];
            const auto b = right.transitions_[a.state];
            result.transitions_[i] = {b.state, profile_->add(a.matches, b.matches)};
        }
        return result;
    }

    [[nodiscard]] std::uint32_t count(const Summary &summary) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        check(summary);
#endif
        return summary.transitions_.front().matches;
    }

  private:
#ifndef SKIP_RUNTIME_CHECKS
    void check(const Summary &summary) const
    {
        if (summary.profile_ != profile_)
        {
            throw std::invalid_argument("ConcatenatedPatternCounter: summary was created by another profile");
        }
    }
#endif

    std::shared_ptr<const Profile> profile_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
