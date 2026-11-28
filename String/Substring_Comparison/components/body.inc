#include <compare>
#include <memory>

#ifndef SIRKIT_SUBSTRING_COMPARISON_DEFINED
#define SIRKIT_SUBSTRING_COMPARISON_DEFINED

class SubstringComparison
{
    struct Data
    {
        std::vector<int> suffix_rank;
        StaticRangeMinimum<int> adjacent_lcp;
    };

    std::shared_ptr<const Data> data_;

    static Data prepare(std::string_view text)
    {
        const auto suffixes = suffix_array(text);
        std::vector<int> rank(suffixes.size());
        for (std::size_t i = 0; i < suffixes.size(); ++i)
        {
            rank[suffixes[i]] = static_cast<int>(i);
        }
        return {std::move(rank), StaticRangeMinimum<int>(lcp_array(text, suffixes))};
    }

    void check_range(std::size_t first, std::size_t length) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (first > size() || length > size() - first)
        {
            throw std::out_of_range("SubstringComparison: invalid range");
        }
#else
        (void)first;
        (void)length;
#endif
    }

    std::size_t common_prefix_unchecked(std::size_t first, std::size_t second) const
    {
        if (first == second || first == size() || second == size())
        {
            return size() - std::max(first, second);
        }
        const auto [left, right] = std::minmax(data_->suffix_rank[first], data_->suffix_rank[second]);
        return static_cast<std::size_t>(data_->adjacent_lcp.min(static_cast<std::size_t>(left),
                                                               static_cast<std::size_t>(right)));
    }

public:
    explicit SubstringComparison(std::string_view text) : data_(std::make_shared<const Data>(prepare(text)))
    {
    }

    SubstringComparison(const SubstringComparison &) noexcept = default;
    SubstringComparison &operator=(const SubstringComparison &) noexcept = default;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return data_->suffix_rank.size();
    }

    [[nodiscard]] std::size_t common_prefix(std::size_t first, std::size_t second) const
    {
        check_range(first, 0);
        check_range(second, 0);
        return common_prefix_unchecked(first, second);
    }

    [[nodiscard]] bool equal(std::size_t first, std::size_t second, std::size_t length) const
    {
        check_range(first, length);
        check_range(second, length);
        return common_prefix_unchecked(first, second) >= length;
    }

    [[nodiscard]] std::strong_ordering compare(std::size_t first, std::size_t first_length,
                                               std::size_t second, std::size_t second_length) const
    {
        check_range(first, first_length);
        check_range(second, second_length);
        if (common_prefix_unchecked(first, second) >= std::min(first_length, second_length))
        {
            return first_length <=> second_length;
        }
        return data_->suffix_rank[first] <=> data_->suffix_rank[second];
    }
};

#endif
