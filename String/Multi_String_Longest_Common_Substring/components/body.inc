#include <deque>

#ifndef SIRKIT_MULTI_STRING_LONGEST_COMMON_SUBSTRING_DEFINED
#define SIRKIT_MULTI_STRING_LONGEST_COMMON_SUBSTRING_DEFINED

struct LongestCommonSubstring
{
    std::size_t length = 0;
    std::vector<std::size_t> starts;
};

namespace detail
{
inline std::size_t shared_substring_extend_length(std::size_t current, std::size_t text_length)
{
    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max()) - 2;
    if (text_length >= kLimit - current)
    {
        throw std::length_error("longest_common_substring: total length");
    }
    return current + text_length + 1; // 入力ごとに異なる区切り文字を1つ入れる。
}
} // namespace detail

inline LongestCommonSubstring longest_common_substring(std::span<const std::string_view> texts)
{
    LongestCommonSubstring result{0, std::vector<std::size_t>(texts.size())};
    if (texts.size() < 2)
    {
        if (!texts.empty())
        {
            result.length = texts.front().size();
        }
        return result;
    }

    if (std::ranges::any_of(texts, &std::string_view::empty))
    {
        return result;
    }
    std::size_t encoded_length = 0;
    for (const auto text : texts)
    {
        encoded_length = detail::shared_substring_extend_length(encoded_length, text.size());
    }

    const auto alphabet_base = static_cast<int>(texts.size());
    std::vector<std::size_t> offsets(texts.size());
    std::vector<int> encoded;
    std::vector<int> owners;
    encoded.reserve(encoded_length);
    owners.reserve(encoded_length);
    for (std::size_t owner = 0; owner < texts.size(); ++owner)
    {
        offsets[owner] = encoded.size();
        for (const char byte : texts[owner])
        {
            encoded.push_back(alphabet_base + static_cast<unsigned char>(byte));
            owners.push_back(static_cast<int>(owner));
        }
        encoded.push_back(static_cast<int>(owner));
        owners.push_back(static_cast<int>(owner));
    }
    const auto suffixes = suffix_array(encoded, alphabet_base + 255);
    const auto lcp = lcp_array(encoded, suffixes);

    std::vector<int> frequencies(texts.size());
    std::deque<std::size_t> minima;
    std::size_t present = 0;
    // 区切り文字から始まる接尾辞は先頭texts.size()本を占める。
    std::size_t left = texts.size();
    std::size_t best_left = 0;
    std::size_t best_end = 0;
    for (std::size_t right = texts.size(); right < suffixes.size(); ++right)
    {
        if (right != texts.size())
        {
            while (!minima.empty() && lcp[minima.back()] >= lcp[right - 1])
            {
                minima.pop_back();
            }
            minima.push_back(right - 1);
        }
        const auto right_owner = owners[static_cast<std::size_t>(suffixes[right])];
        if (++frequencies[right_owner] == 1)
        {
            ++present;
        }
        while (present == texts.size())
        {
            const auto length = static_cast<std::size_t>(lcp[minima.front()]);
            if (length > result.length)
            {
                result.length = length;
                best_left = left;
                best_end = right + 1;
            }
            const auto left_owner = owners[static_cast<std::size_t>(suffixes[left])];
            if (--frequencies[left_owner] == 0)
            {
                --present;
            }
            if (minima.front() == left)
            {
                minima.pop_front();
            }
            ++left;
        }
    }

    for (std::size_t i = best_left; i < best_end; ++i)
    {
        const auto start = static_cast<std::size_t>(suffixes[i]);
        const auto owner = owners[start];
        result.starts[owner] = start - offsets[owner];
    }
    return result;
}

#endif
