#include <cstddef>

#ifndef SIRKIT_STRING_PERIODS_DEFINED
#define SIRKIT_STRING_PERIODS_DEFINED

class StringPeriods
{
public:
    explicit StringPeriods(std::string_view text) : border_(prefix_function(text))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return border_.size();
    }

    [[nodiscard]] std::size_t least_period(std::size_t length) const
    {
        return length - last_border(length);
    }

    [[nodiscard]] std::size_t primitive_root_length(std::size_t length) const
    {
        const auto period = least_period(length);
        if (period == 0 || length % period == 0)
        {
            return period;
        }
        return length;
    }

    [[nodiscard]] std::vector<std::size_t> proper_borders(std::size_t length) const
    {
        std::vector<std::size_t> result;
        for (auto border = last_border(length); border != 0; border = border_[border - 1])
        {
            result.push_back(border);
        }
        return result;
    }

    [[nodiscard]] std::vector<std::size_t> periods(std::size_t length) const
    {
        auto result = proper_borders(length);
        for (auto &border : result)
        {
            border = length - border;
        }
        if (length != 0)
        {
            result.push_back(length);
        }
        return result;
    }

private:
    std::vector<std::size_t> border_;

    [[nodiscard]] std::size_t last_border(std::size_t length) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (length > border_.size())
        {
            throw std::out_of_range("StringPeriods: prefix length");
        }
#endif
        return length == 0 ? 0 : border_[length - 1];
    }
};

#endif
