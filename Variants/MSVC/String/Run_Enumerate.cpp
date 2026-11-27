// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: String/Run_Enumerate.cpp
// 参考: H. Bannai et al., The "Runs" Theorem (https://arxiv.org/abs/1406.0263)。コードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/run_enumerate.md
//
// API一覧
// run_enumerate(s): 文字列sの極大な周期区間を重複なく列挙する。O(N log(N+1))時間・領域。
// StringRun{period, left, right}: 最小周期と0始まりの半開区間。長さは周期の2倍以上。
// 同じ周期で左右どちらにも延長できない区間を、(period, left, right)の昇順で返す。
// 文字は符号なしバイトとして扱う。空文字列を受け取れる。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <compare>
#include <cstddef>
#include <limits>
#include <numeric>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STRING_RUN_DEFINED
#define SIRKIT_STRING_RUN_DEFINED

struct StringRun
{
    std::size_t period;
    std::size_t left;
    std::size_t right;

    auto operator<=>(const StringRun &) const = default;
};

namespace detail
{
class RunBlockRanks
{
    using Index = std::size_t;

public:
    explicit RunBlockRanks(std::string_view text) : size_(text.size())
    {
        constexpr auto kByteAlphabet = static_cast<Index>(std::numeric_limits<unsigned char>::max()) + 1;
        ranks_.reserve(std::bit_width(size_));
        auto &first = ranks_.emplace_back(size_);
        for (Index i = 0; i < size_; ++i)
        {
            first[i] = static_cast<unsigned char>(text[i]) + Index{1};
        }

        // 長さ2^kごとに等しいブロックへ同じ順位を付ける。末尾の外は順位0で埋める。
        std::vector<Index> offsets(std::max(kByteAlphabet, size_) + 1);
        std::vector<Index> indices(size_);
        std::vector<Index> order(size_);
        std::iota(indices.begin(), indices.end(), Index{0});
        const auto stable_sort = [&](const std::vector<Index> &rank, Index kinds)
        {
            auto slots = std::span(offsets).first(kinds + 1);
            std::ranges::fill(slots, 0);
            for (const auto index : indices)
            {
                ++slots[rank[index]];
            }
            Index start = 0;
            for (auto &slot : slots)
            {
                const auto count = slot;
                slot = start;
                start += count;
            }
            for (const auto index : indices)
            {
                order[slots[rank[index]]++] = index;
            }
        };

        Index kinds = kByteAlphabet;
        stable_sort(first, kinds);
        for (Index width = 1; width <= size_ / 2; width *= 2)
        {
            const auto &previous = ranks_.back();
            const auto second = [&](Index i)
            {
                return i < size_ - width ? previous[i + width] : 0;
            };
            // 後半の順位順は直前の並びをwidthだけずらして得る。前半の順位で安定ソートする。
            Index count = 0;
            for (Index i = size_ - width; i < size_; ++i)
            {
                indices[count++] = i;
            }
            for (const auto i : order)
            {
                if (i >= width)
                {
                    indices[count++] = i - width;
                }
            }
            stable_sort(previous, kinds);

            std::vector<Index> rank(size_);
            kinds = 0;
            std::pair<Index, Index> last{0, 0};
            for (const auto i : order)
            {
                const std::pair key{previous[i], second(i)};
                kinds += key != last;
                rank[i] = kinds;
                last = key;
            }
            ranks_.push_back(std::move(rank));
        }
    }

    Index CommonPrefix(Index a, Index b) const
    {
        return CommonLength(size_ - b, [a, b](Index matched, Index)
                            {
                                return std::pair(a + matched, b + matched);
                            });
    }

    Index CommonSuffix(Index a, Index b) const
    {
        return CommonLength(a, [a, b](Index matched, Index width)
                            {
                                return std::pair(a - matched - width, b - matched - width);
                            });
    }

private:
    Index size_;
    std::vector<std::vector<Index>> ranks_;

    template <class Positions>
    Index CommonLength(Index limit, Positions positions) const
    {
        Index matched = 0;
        for (auto level = std::bit_width(limit); level != 0; --level)
        {
            const auto width = Index{1} << (level - 1);
            if (width <= limit - matched)
            {
                const auto [a, b] = positions(matched, width);
                if (ranks_[level - 1][a] == ranks_[level - 1][b])
                {
                    matched += width;
                }
            }
        }
        return matched;
    }
};
} // namespace detail

inline std::vector<StringRun> run_enumerate(std::string_view text)
{
    const auto n = text.size();
    if (n < 2)
    {
        return {};
    }
    const detail::RunBlockRanks ranks(text);
    std::vector<StringRun> result;
    std::vector<std::size_t> suffixes;
    suffixes.reserve(n);
    for (const bool reversed_alphabet : {false, true})
    {
        suffixes.clear();
        for (auto i = n; i != 0;)
        {
            --i;
            std::size_t right = 0;
            while (!suffixes.empty())
            {
                const auto j = suffixes.back();
                right = ranks.CommonPrefix(i, j);
                if (j + right == n ||
                    (static_cast<unsigned char>(text[j + right]) < static_cast<unsigned char>(text[i + right])) !=
                        reversed_alphabet)
                {
                    break;
                }
                suffixes.pop_back();
            }
            if (!suffixes.empty())
            {
                // 次に小さい接尾辞との間隔がLyndon根の長さ。これを周期として両側へ延長する。
                const auto j = suffixes.back();
                const auto period = j - i;
                const auto left = ranks.CommonSuffix(i, j);
                if (left + right >= period)
                {
                    result.push_back({period, i - left, j + right});
                }
            }
            suffixes.push_back(i);
        }
    }
    std::ranges::sort(result);
    const auto duplicates = std::ranges::unique(result);
    result.erase(duplicates.begin(), duplicates.end());
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
