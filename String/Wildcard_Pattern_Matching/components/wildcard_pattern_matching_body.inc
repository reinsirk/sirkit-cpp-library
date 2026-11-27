#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SIRKIT_WILDCARD_PATTERN_MATCHING_DEFINED
#define SIRKIT_WILDCARD_PATTERN_MATCHING_DEFINED

namespace reinsirk::detail
{
inline std::vector<std::uint32_t> WildcardCodes(std::string_view characters)
{
    std::vector<std::uint32_t> values;
    values.reserve(characters.size());
    for (const char c : characters)
    {
        if (c == '*')
        {
            values.push_back(0);
            continue;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < 'a' || c > 'z')
        {
            throw std::invalid_argument("wildcard_pattern_matching: expected lowercase letters or '*'");
        }
#endif
        values.push_back(static_cast<std::uint32_t>(c - 'a' + 1));
    }
    return values;
}

inline std::vector<bool> WildcardScan(const std::vector<std::uint32_t> &text,
                                      const std::vector<std::uint32_t> &pattern)
{
    std::vector<bool> result(text.size() - pattern.size() + 1, true);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        for (std::size_t j = 0; j < pattern.size(); ++j)
        {
            const auto a = text[i + j], b = pattern[j];
            if (a != 0 && b != 0 && a != b)
            {
                result[i] = false;
                break;
            }
        }
    }
    return result;
}

template <class Feature>
std::vector<std::uint32_t> WildcardFeatures(const std::vector<std::uint32_t> &values, Feature feature)
{
    std::vector<std::uint32_t> result(values.size());
    std::ranges::transform(values, result.begin(), feature);
    return result;
}
} // namespace reinsirk::detail

inline std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern)
{
    constexpr std::size_t kMaxLength = std::size_t{1} << 19;
    if (std::max(text.size(), pattern.size()) > kMaxLength)
    {
        throw std::length_error("wildcard_pattern_matching: string too long");
    }
    auto a = reinsirk::detail::WildcardCodes(text);
    auto b = reinsirk::detail::WildcardCodes(pattern);
    if (a.size() < b.size())
    {
        return {};
    }
    if (b.size() <= 32)
    {
        return reinsirk::detail::WildcardScan(a, b);
    }

    std::ranges::reverse(b);
    const auto square = [](std::uint32_t x)
    {
        return x * x;
    };
    const auto present = [](std::uint32_t x)
    {
        return x != 0;
    };
    NTT<> ntt;
    const auto x = ntt.convolution_normalized(reinsirk::detail::WildcardFeatures(a, square),
                                              reinsirk::detail::WildcardFeatures(b, present));
    const auto y = ntt.convolution_normalized(reinsirk::detail::WildcardFeatures(a, present),
                                              reinsirk::detail::WildcardFeatures(b, square));
    const auto pattern_length = b.size();
    const auto alignments = a.size() - pattern_length + 1;
    const auto products = ntt.convolution_normalized(std::move(a), std::move(b));

    // 各位置で a²[非*のb] + [非*のa]b² − 2ab を合計する。各畳み込みの係数は法未満。
    static_assert(2 * 26 * 26 * kMaxLength < NTT<>::modulus());
    std::vector<bool> result(alignments);
    for (std::size_t i = 0; i < alignments; ++i)
    {
        const auto j = i + pattern_length - 1;
        result[i] = x[j] + y[j] == 2 * products[j];
    }
    return result;
}

#endif
