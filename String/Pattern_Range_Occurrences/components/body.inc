#include <array>
#include <cstddef>
#include <memory>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

#ifndef SIRKIT_PATTERN_RANGE_OCCURRENCES_DEFINED
#define SIRKIT_PATTERN_RANGE_OCCURRENCES_DEFINED

namespace reinsirk::detail
{
inline std::size_t PatternRangeOffsetCount(std::size_t length, std::size_t capacity)
{
    if (capacity < 2 || length > capacity - 2)
    {
        throw std::length_error("PatternRangeOccurrences: text too long");
    }
    return length + 2;
}
} // namespace reinsirk::detail

class PatternRangeOccurrences
{
    struct Data
    {
        std::size_t length, pattern_length;
        std::vector<std::size_t> offsets, starts;

        Data(std::string_view text, std::string_view pattern)
            : length(text.size()), pattern_length(pattern.size())
        {
            offsets.resize(reinsirk::detail::PatternRangeOffsetCount(length, offsets.max_size()));
            starts = find_occurrences(text, pattern);
            for (const auto position : starts)
            {
                ++offsets[position + 1];
            }
            std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
        }
    };

    static constexpr std::array<std::size_t, 1> kEmptyBoundary{0};
    std::shared_ptr<const Data> data_;

public:
    PatternRangeOccurrences() = default;

    PatternRangeOccurrences(std::string_view text, std::string_view pattern)
        : data_(std::make_shared<const Data>(text, pattern))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return data_ ? data_->length : 0;
    }

    [[nodiscard]] std::size_t pattern_size() const noexcept
    {
        return data_ ? data_->pattern_length : 0;
    }

    [[nodiscard]] std::span<const std::size_t> positions(std::size_t left, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size())
        {
            throw std::out_of_range("PatternRangeOccurrences: invalid interval");
        }
#endif
        if (!data_)
        {
            return kEmptyBoundary;
        }
        const auto &data = *data_;
        if (data.pattern_length > right - left)
        {
            return {};
        }

        // offsets[k]はkより前に始まる出現数。最右の開始位置はright-pattern_length。
        const auto first = data.offsets[left];
        const auto stop = data.offsets[right - data.pattern_length + 1];
        return std::span<const std::size_t>(data.starts).subspan(first, stop - first);
    }

    [[nodiscard]] std::size_t count(std::size_t left, std::size_t right) const
    {
        return positions(left, right).size();
    }
};

#endif
