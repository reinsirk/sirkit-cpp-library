#include <string_view>

#ifndef SIRKIT_HAMMING_DISTANCE_MATCHING_DEFINED
#define SIRKIT_HAMMING_DISTANCE_MATCHING_DEFINED

namespace reinsirk::detail
{
inline std::size_t hamming_transform_size(std::size_t text_size, std::size_t pattern_size)
{
    constexpr auto kCapacity = NTT<>::max_length();
    if (text_size > kCapacity || pattern_size - 1 > kCapacity - text_size)
    {
        throw std::length_error("hamming_distances: input is too long");
    }
    return std::bit_ceil(text_size + (pattern_size - 1));
}
} // namespace detail

inline std::vector<std::size_t> hamming_distances(std::string_view text, std::string_view pattern)
{
    const auto n = text.size();
    const auto m = pattern.size();
    if (m > n)
    {
        return {};
    }
    if (m == 0)
    {
        return std::vector<std::size_t>(n + 1);
    }
    const auto transform_size = detail::hamming_transform_size(n, m);
    const auto alignments = n - m + 1;
    std::vector<std::size_t> distances(alignments);
    if (alignments <= 8 || alignments <= 4096 / m)
    {
        for (std::size_t start = 0; start < alignments; ++start)
        {
            for (std::size_t j = 0; j < m; ++j)
            {
                distances[start] += text[start + j] != pattern[j];
            }
        }
        return distances;
    }

    std::fill(distances.begin(), distances.end(), m);
    std::array<std::vector<std::size_t>, 256> by_text_byte, by_pattern_byte;
    for (std::size_t i = 0; i < n; ++i)
    {
        by_text_byte[static_cast<unsigned char>(text[i])].push_back(i);
    }
    for (std::size_t j = 0; j < m; ++j)
    {
        by_pattern_byte[static_cast<unsigned char>(pattern[j])].push_back(j);
    }
    const auto pair_limit = transform_size * std::bit_width(transform_size);
    NTT<> ntt;
    for (std::size_t byte = 0; byte < by_text_byte.size(); ++byte)
    {
        const auto &in_text = by_text_byte[byte];
        const auto &in_pattern = by_pattern_byte[byte];
        if (in_text.empty() || in_pattern.empty())
        {
            continue;
        }
        if (in_text.size() <= pair_limit / in_pattern.size())
        {
            for (const auto at : in_text)
            {
                for (const auto target : in_pattern)
                {
                    if (at >= target && at - target < alignments)
                    {
                        --distances[at - target];
                    }
                }
            }
        }
        else
        {
            std::vector<std::uint32_t> a(n), b(m);
            for (const auto at : in_text)
            {
                a[at] = 1;
            }
            for (const auto at : in_pattern)
            {
                b[m - 1 - at] = 1;
            }
            const auto matches = ntt.convolution_normalized(std::move(a), std::move(b));
            for (std::size_t start = 0; start < alignments; ++start)
            {
                distances[start] -= matches[start + m - 1];
            }
        }
    }
    return distances;
}

inline std::vector<std::size_t> approximate_match_positions(std::string_view text, std::string_view pattern,
                                                            std::size_t max_mismatches)
{
    const auto distances = hamming_distances(text, pattern);
    std::vector<std::size_t> positions;
    for (std::size_t i = 0; i < distances.size(); ++i)
    {
        if (distances[i] <= max_mismatches)
        {
            positions.push_back(i);
        }
    }
    return positions;
}

#endif
