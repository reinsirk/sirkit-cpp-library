#include <optional>

#ifndef SIRKIT_SUBSTRING_INDEX_DEFINED
#define SIRKIT_SUBSTRING_INDEX_DEFINED

class SubstringIndex
{
    struct Data
    {
        explicit Data(std::string_view source)
        {
            detail::suffix_length_check(source.size());
            text.assign(source);
            suffixes = suffix_array(text);
            const auto adjacent = lcp_array(text, suffixes);
            distinct_prefix.resize(suffixes.size());
            std::uint64_t total = 0;
            for (std::size_t i = 0; i < suffixes.size(); ++i)
            {
                const auto already_seen = i == 0 ? 0 : adjacent[i - 1];
                // 直前の接尾辞との共通接頭辞以外が、この接尾辞から初めて現れる。
                total += text.size() - static_cast<std::size_t>(suffixes[i] + already_seen);
                distinct_prefix[i] = total;
            }
        }

        std::string text;
        std::vector<int> suffixes;
        std::vector<std::uint64_t> distinct_prefix;
    };

  public:
    explicit SubstringIndex(std::string_view text) : data_(std::make_shared<const Data>(text))
    {
    }

    SubstringIndex(const SubstringIndex &) noexcept = default;
    SubstringIndex &operator=(const SubstringIndex &) noexcept = default;

    [[nodiscard]] std::uint64_t distinct_substring_count() const noexcept
    {
        return data_->distinct_prefix.empty() ? 0 : data_->distinct_prefix.back();
    }

    [[nodiscard]] std::optional<std::string_view> kth_distinct_substring(std::uint64_t rank) const noexcept
    {
        if (rank == 0 || rank > distinct_substring_count())
        {
            return std::nullopt;
        }
        const auto &totals = data_->distinct_prefix;
        const auto found = std::lower_bound(totals.begin(), totals.end(), rank);
        const auto slot = static_cast<std::size_t>(found - totals.begin());
        const auto start = static_cast<std::size_t>(data_->suffixes[slot]);
        const auto tail = static_cast<std::size_t>(*found - rank);
        return std::string_view(data_->text).substr(start, data_->text.size() - start - tail);
    }

    [[nodiscard]] std::span<const int> occurrences(std::string_view pattern) const
    {
        const auto [left, right] = SuffixArrayEqualRange(data_->text, data_->suffixes, pattern);
        return std::span<const int>(data_->suffixes).subspan(static_cast<std::size_t>(left),
                                                            static_cast<std::size_t>(right - left));
    }

  private:
    std::shared_ptr<const Data> data_;
};

#endif
