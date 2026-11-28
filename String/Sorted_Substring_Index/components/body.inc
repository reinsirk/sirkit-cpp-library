#include <array>
#include <cstddef>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SIRKIT_SORTED_SUBSTRING_INDEX_DEFINED
#define SIRKIT_SORTED_SUBSTRING_INDEX_DEFINED

namespace reinsirk::detail
{
inline void CheckSortedSubstringLength(std::size_t length)
{
    if (length > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("SortedSubstringIndex: length");
    }
}

inline constexpr bool IsSortedSubstringDescent(char before, char after) noexcept
{
    return static_cast<unsigned char>(before) > static_cast<unsigned char>(after);
}
inline constexpr unsigned char JoinSortedDescents(unsigned char left, unsigned char right) noexcept
{
    return static_cast<unsigned char>(left | right);
}

inline constexpr unsigned char EmptySortedDescents() noexcept
{
    return 0;
}
} // namespace reinsirk::detail

template <std::size_t Alphabet = 26, unsigned First = 'a'>
    requires (1 <= Alphabet && Alphabet <= 256 && First <= 256 - Alphabet)
class SortedSubstringIndex
{
    std::string text_;
    std::array<std::set<int>, Alphabet> positions_;
    segtree<unsigned char, reinsirk::detail::JoinSortedDescents, reinsirk::detail::EmptySortedDescents> descents_;

    static std::string make_text(std::string_view text)
    {
        ::reinsirk::detail::CheckSortedSubstringLength(text.size());
        return std::string(text);
    }

    static std::vector<unsigned char> make_descents(std::string_view text)
    {
        std::vector<unsigned char> entries(text.empty() ? 0 : text.size() - 1);
        for (std::size_t i = 1; i < text.size(); ++i)
        {
            entries[i - 1] = ::reinsirk::detail::IsSortedSubstringDescent(text[i - 1], text[i]);
        }
        return entries;
    }

    static std::size_t symbol(char byte)
    {
        const auto value = static_cast<unsigned char>(byte);
#ifndef SKIP_RUNTIME_CHECKS
        if (value < First || static_cast<std::size_t>(value) - First >= Alphabet)
        {
            throw std::out_of_range("SortedSubstringIndex: byte");
        }
#endif
        return static_cast<std::size_t>(value) - First;
    }

    void check_position([[maybe_unused]] int position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position < 0 || position >= size())
        {
            throw std::out_of_range("SortedSubstringIndex: position");
        }
#endif
    }

    void check_range([[maybe_unused]] int left, [[maybe_unused]] int right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left < 0 || left > right || right > size())
        {
            throw std::out_of_range("SortedSubstringIndex: range");
        }
#endif
    }

public:
    SortedSubstringIndex() = default;

    explicit SortedSubstringIndex(std::string_view text) : text_(make_text(text)), descents_(make_descents(text_))
    {
        for (int i = 0; i < size(); ++i)
        {
            auto &group = positions_[symbol(text_[static_cast<std::size_t>(i)])];
            group.emplace_hint(group.end(), i);
        }
    }

    SortedSubstringIndex(const SortedSubstringIndex &) = default;

    SortedSubstringIndex(SortedSubstringIndex &&other)
        : text_(std::move(other.text_)), positions_(std::move(other.positions_)), descents_(std::move(other.descents_))
    {
        other.text_.clear();
        for (auto &group : other.positions_)
        {
            group.clear();
        }
    }

    SortedSubstringIndex &operator=(SortedSubstringIndex other) noexcept
    {
        text_.swap(other.text_);
        positions_.swap(other.positions_);
        descents_.swap(other.descents_);
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(text_.size());
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return text_.empty();
    }

    [[nodiscard]] char get(int position) const
    {
        check_position(position);
        return text_[static_cast<std::size_t>(position)];
    }

    void set(int position, char byte)
    {
        check_position(position);
        const auto index = static_cast<std::size_t>(position);
        const auto before = text_[index];
        const auto from = symbol(before);
        const auto to = symbol(byte);
        if (from == to)
        {
            return;
        }
        if (position != 0)
        {
            const char neighbor = text_[index - 1];
            descents_.set(position - 1, ::reinsirk::detail::IsSortedSubstringDescent(neighbor, byte));
        }
        if (position + 1 != size())
        {
            const char neighbor = text_[index + 1];
            descents_.set(position, ::reinsirk::detail::IsSortedSubstringDescent(byte, neighbor));
        }
        positions_[to].insert(positions_[from].extract(position));
        text_[index] = byte;
    }

    [[nodiscard]] bool is_nondecreasing(int left, int right) const
    {
        check_range(left, right);
        return left == right || descents_.prod(left, right - 1) == 0;
    }

    [[nodiscard]] bool is_sorted_slice(int left, int right) const
    {
        if (!is_nondecreasing(left, right))
        {
            return false;
        }
        if (left == right)
        {
            return true;
        }
        const auto begin = symbol(text_[static_cast<std::size_t>(left)]);
        const auto end = symbol(text_[static_cast<std::size_t>(right - 1)]);
        // 両端の文字は個数の一部を選べる。間の文字はすべて含まれていなければならない。
        for (auto byte = begin + 1; byte < end; ++byte)
        {
            const auto &group = positions_[byte];
            if (!group.empty() && (*group.begin() < left || *group.rbegin() >= right))
            {
                return false;
            }
        }
        return true;
    }
};

#endif
