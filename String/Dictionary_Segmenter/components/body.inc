#include <algorithm>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <vector>

#ifndef SIRKIT_DICTIONARY_SEGMENTER_DEFINED
#define SIRKIT_DICTIONARY_SEGMENTER_DEFINED

namespace reinsirk::detail
{
inline std::size_t DictionarySegmentationBoundaryCount(std::size_t length, std::size_t limit)
{
    if (length >= limit)
    {
        throw std::length_error("DictionarySegmenter: text too long");
    }
    return length + 1;
}

template <typename Words>
concept DictionaryWordRange = std::ranges::input_range<const Words> &&
                              std::convertible_to<std::ranges::range_reference_t<const Words>, std::string_view>;
} // namespace reinsirk::detail

template <std::size_t Alphabet = 26, unsigned First = 'a'>
class DictionarySegmenter
{
    struct Data
    {
        AhoCorasick<Alphabet, First> matcher;
        std::vector<std::size_t> word_ids;

        template <reinsirk::detail::DictionaryWordRange Words>
        explicit Data(const Words &words)
        {
            std::size_t id = 0;
            for (const std::string_view word : words)
            {
                if (!word.empty())
                {
                    matcher.add(word);
                    word_ids.push_back(id);
                }
                ++id;
            }
            matcher.build();
        }
    };

    struct Choice
    {
        std::size_t count = std::numeric_limits<std::size_t>::max();
        int last_word = -1;
    };

    std::shared_ptr<const Data> data_;

public:
    template <reinsirk::detail::DictionaryWordRange Words>
    explicit DictionarySegmenter(const Words &words) : data_(std::make_shared<const Data>(words))
    {
    }

    [[nodiscard]] std::optional<std::vector<std::size_t>> minimum_segmentation(std::string_view text) const
    {
        if (text.empty())
        {
            return std::vector<std::size_t>{};
        }
        if (!data_)
        {
            return std::nullopt;
        }
        std::vector<Choice> best;
        best.resize(reinsirk::detail::DictionarySegmentationBoundaryCount(text.size(), best.max_size()));
        best[0].count = 0;

        // 一致は終端の昇順。空語を除いたため、更新には確定済みの短い接頭辞だけを使う。
        data_->matcher.for_each_match(text, [&](int word, std::size_t begin, std::size_t end)
        {
            constexpr auto kUnreachable = std::numeric_limits<std::size_t>::max();
            if (best[begin].count == kUnreachable)
            {
                return;
            }
            const auto candidate = best[begin].count + 1;
            auto &choice = best[end];
            if (candidate < choice.count || (candidate == choice.count && word < choice.last_word))
            {
                choice = {candidate, word};
            }
        });
        if (best.back().last_word < 0)
        {
            return std::nullopt;
        }

        std::vector<std::size_t> result;
        result.reserve(best.back().count);
        for (std::size_t end = text.size(); end != 0;)
        {
            const int word = best[end].last_word;
            result.push_back(data_->word_ids[static_cast<std::size_t>(word)]);
            end -= data_->matcher.pattern_length(word);
        }
        std::ranges::reverse(result);
        return result;
    }
};

using ByteDictionarySegmenter = DictionarySegmenter<256, 0>;

#endif
