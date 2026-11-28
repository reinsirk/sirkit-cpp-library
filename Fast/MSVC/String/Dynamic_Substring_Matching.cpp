// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Dynamic_Substring_Matching.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_substring_matching.md
//
// API一覧
// DynamicSubstringMatching(text): 固定長バイト列を所有し、各バイトの出現位置を64ビットずつ記録する。
// size(): 元のバイト列の長さ。O(1)。
// set(position, byte): 0始まりの位置を1バイト置換する。O(1)。
// contains(pattern): patternが連続部分文字列として現れるかを返す。
// count(pattern): 重なりも含めた出現回数を返す。
// occurrences(pattern): 出現する0始まりの位置を昇順で返す。
// 空patternはn+1箇所に現れ、長すぎるpatternは0箇所。nは元の長さ、mはパターン長。
// 構築・保管O(n+1)。空でないm<=nの検索はO(m*(1+ceil((n-m+1)/64)))時間・O(m)追加領域。出力にはさらに結果の領域と時間が必要。
// コピーは独立。移動元は空として使える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_SUBSTRING_MATCHING_DEFINED
#define SIRKIT_DYNAMIC_SUBSTRING_MATCHING_DEFINED

class DynamicSubstringMatching
{
public:
    explicit DynamicSubstringMatching(std::string_view text) : text_(text)
    {
        const auto blocks = text.size() / kWordBits + (text.size() % kWordBits != 0);
        // 右にずらした窓を取り出すとき、末尾からも次の語を読めるようにする。
        for (auto &row : positions_)
        {
            row.resize(blocks + 1);
        }
        for (std::size_t i = 0; i < text.size(); ++i)
        {
            const auto byte = static_cast<unsigned char>(text[i]);
            ++frequency_[byte];
            positions_[byte][i / kWordBits] |= bit_at(i);
        }
    }

    DynamicSubstringMatching(const DynamicSubstringMatching &) = default;

    DynamicSubstringMatching(DynamicSubstringMatching &&other) noexcept
    {
        swap_state(other);
    }

    DynamicSubstringMatching &operator=(DynamicSubstringMatching other) noexcept
    {
        swap_state(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return text_.size();
    }

    void set(std::size_t position, char replacement)
    {
        const auto old = static_cast<unsigned char>(text_[position]);
        const auto next = static_cast<unsigned char>(replacement);
        if (old == next)
        {
            return;
        }
        const auto index = position / kWordBits;
        const auto bit = bit_at(position);
        positions_[old][index] &= ~bit;
        positions_[next][index] |= bit;
        --frequency_[old];
        ++frequency_[next];
        text_[position] = replacement;
    }

    [[nodiscard]] bool contains(std::string_view pattern) const
    {
        return pattern.empty() || scan(pattern, [](std::size_t, std::uint64_t) { return true; });
    }

    [[nodiscard]] std::size_t count(std::string_view pattern) const
    {
        if (pattern.empty())
        {
            return size() + 1;
        }
        std::size_t result = 0;
        scan(pattern, [&](std::size_t, std::uint64_t candidates) {
            result += static_cast<std::size_t>(std::popcount(candidates));
            return false;
        });
        return result;
    }

    [[nodiscard]] std::vector<std::size_t> occurrences(std::string_view pattern) const
    {
        if (pattern.empty())
        {
            std::vector<std::size_t> all(size() + 1);
            std::iota(all.begin(), all.end(), std::size_t{0});
            return all;
        }
        std::vector<std::size_t> result;
        scan(pattern, [&](std::size_t base, std::uint64_t candidates) {
            do
            {
                result.push_back(base + static_cast<std::size_t>(std::countr_zero(candidates)));
                candidates &= candidates - 1;
            } while (candidates != 0);
            return false;
        });
        return result;
    }

private:
    static constexpr unsigned kWordBits = 64;
    static constexpr unsigned kAlphabetSize = 256;

    void swap_state(DynamicSubstringMatching &other) noexcept
    {
        text_.swap(other.text_);
        positions_.swap(other.positions_);
        frequency_.swap(other.frequency_);
    }

    static constexpr std::uint64_t bit_at(std::size_t position) noexcept
    {
        return std::uint64_t{1} << (position % kWordBits);
    }

    struct Constraint
    {
        const std::uint64_t *words;
        unsigned shift;

        [[nodiscard]] std::uint64_t at(std::size_t block) const noexcept
        {
            const auto word = words[block];
            if (shift == 0)
            {
                return word;
            }
            return (word >> shift) | (words[block + 1] << (kWordBits - shift));
        }
    };

    template<class Consume>
    bool scan(std::string_view pattern, Consume consume) const
    {
        if (pattern.size() > size())
        {
            return false;
        }
        std::vector<Constraint> constraints;
        constraints.reserve(pattern.size());
        std::size_t rarest = 0;
        auto smallest_frequency = size();
        for (std::size_t i = 0; i < pattern.size(); ++i)
        {
            const auto byte = static_cast<unsigned char>(pattern[i]);
            if (frequency_[byte] == 0)
            {
                return false;
            }
            if (frequency_[byte] < smallest_frequency)
            {
                rarest = i;
                smallest_frequency = frequency_[byte];
            }
            constraints.push_back({positions_[byte].data() + i / kWordBits, static_cast<unsigned>(i % kWordBits)});
        }
        std::swap(constraints.front(), constraints[rarest]);

        const auto last_start = size() - pattern.size();
        const auto last_block = last_start / kWordBits;
        const auto tail_mask = ~std::uint64_t{0} >> ((kWordBits - 1) - last_start % kWordBits);
        for (std::size_t block = 0; block <= last_block; ++block)
        {
            auto candidates = block == last_block ? tail_mask : ~std::uint64_t{0};
            for (const auto &constraint : constraints)
            {
                candidates &= constraint.at(block);
                if (candidates == 0)
                {
                    break;
                }
            }
            if (candidates != 0 && consume(block * kWordBits, candidates))
            {
                return true;
            }
        }
        return false;
    }

    std::string text_;
    std::array<std::vector<std::uint64_t>, kAlphabetSize> positions_;
    std::array<std::size_t, kAlphabetSize> frequency_{};
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
