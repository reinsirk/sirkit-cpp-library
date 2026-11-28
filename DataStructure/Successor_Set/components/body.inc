#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_SUCCESSOR_SET_DEFINED
#define SIRKIT_SUCCESSOR_SET_DEFINED

class SuccessorSet
{
public:
    explicit SuccessorSet(std::size_t n = 0)
        : groups_(capacity_with_sentinel(n)), rightmost_(n + 1), remaining_(static_cast<int>(n))
    {
        std::iota(rightmost_.begin(), rightmost_.end(), 0);
    }

    [[nodiscard]] int size() const noexcept
    {
        return std::max(0, groups_.size() - 1);
    }

    [[nodiscard]] int remaining() const noexcept
    {
        return std::min(size(), remaining_);
    }

    [[nodiscard]] std::optional<int> next(int position)
    {
        check_position(position);
        if (position == size())
        {
            return std::nullopt;
        }
        const int index = rightmost_[groups_.root(position)];
        return index == size() ? std::nullopt : std::optional(index);
    }

    bool erase(int position)
    {
        check_position(position);
        if (position == size())
        {
            return false;
        }
        const int component = groups_.root(position);
        if (rightmost_[component] != position)
        {
            return false;
        }
        const int following = groups_.root(position + 1);
        const int successor = rightmost_[following];
        groups_.unite(component, following);
        rightmost_[groups_.root(component)] = successor;
        --remaining_;
        return true;
    }

    std::optional<int> take(int first)
    {
        const auto result = next(first);
        if (result)
        {
            erase(*result);
        }
        return result;
    }

    std::optional<int> take_cyclic(int first)
    {
        return take(first).or_else([this]
                                   {
                                       return take(0);
                                   });
    }

private:
    static std::size_t capacity_with_sentinel(std::size_t n)
    {
        if (n >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("SuccessorSet: capacity");
        }
        return n + 1;
    }

    void check_position(int position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position < 0 || position > size())
        {
            throw std::out_of_range("SuccessorSet: position");
        }
#else
        (void)position;
#endif
    }

    UnionFind groups_;
    std::vector<int> rightmost_;
    int remaining_;
};

inline std::optional<std::vector<int>> parking_slots(std::span<const int> preferences)
{
    SuccessorSet available(preferences.size());
    std::vector<int> answer;
    answer.reserve(preferences.size());
    for (const int preferred : preferences)
    {
        if (preferred < 0 || preferred >= available.size())
        {
            return std::nullopt;
        }
        const auto slot = available.take(preferred);
        if (!slot)
        {
            return std::nullopt;
        }
        answer.push_back(*slot);
    }
    return answer;
}

#endif
