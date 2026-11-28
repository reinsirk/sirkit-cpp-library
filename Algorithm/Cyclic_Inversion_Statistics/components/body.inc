#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_CYCLIC_INVERSION_STATISTICS_DEFINED
#define SIRKIT_CYCLIC_INVERSION_STATISTICS_DEFINED

namespace reinsirk::detail
{
inline constexpr std::size_t kMaxCyclicInversionLength = 2'642'246;

inline void check_cyclic_inversion_length(std::size_t n)
{
    // n個の転倒数をすべて足した上限 n*n*(n-1)/2 が int64_t に収まる。
    if (n > kMaxCyclicInversionLength)
    {
        throw std::length_error("CyclicInversionStatistics: input too long");
    }
}
} // namespace reinsirk::detail

class CyclicInversionStatistics
{
    std::vector<int> ranks_;
    std::vector<int> rotation_changes_;
    std::int64_t inversions_ = 0;
    std::int64_t rotation_sum_ = 0;

    template <class Visitor>
    void visit_left_rotations(Visitor visit) const
    {
        auto current = inversions_;
        for (const auto rank : ranks_)
        {
            visit(current);
            current += rotation_changes_[static_cast<std::size_t>(rank)];
        }
    }

public:
    CyclicInversionStatistics() = default;

    template <class Range, class Compare = std::less<>>
        requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
            && std::indirect_strict_weak_order<Compare, std::ranges::iterator_t<const Range>>
    explicit CyclicInversionStatistics(const Range &values, Compare less = {})
    {
        const auto count = static_cast<std::size_t>(std::ranges::size(values));
        ::reinsirk::detail::check_cyclic_inversion_length(count);
        const int n = static_cast<int>(count);
        std::vector<int> positions(count);
        std::iota(positions.begin(), positions.end(), 0);
        ranks_.resize(count);
        rotation_changes_.reserve(count);
        const auto first = std::ranges::begin(values);
        const auto position_less = [&](int a, int b)
        {
            return std::invoke(less, first[a], first[b]);
        };
        std::sort(positions.begin(), positions.end(), position_less);
        for (std::size_t begin = 0; begin < count;)
        {
            std::size_t end = begin + 1;
            while (end < count && !position_less(positions[begin], positions[end]))
            {
                ++end;
            }
            const int rank = static_cast<int>(rotation_changes_.size());
            // この値を先頭から末尾へ移すと、前後が逆になるのは大小が異なる組だけ。
            rotation_changes_.push_back(n - static_cast<int>(end) - static_cast<int>(begin));
            for (std::size_t i = begin; i < end; ++i)
            {
                ranks_[static_cast<std::size_t>(positions[i])] = rank;
            }
            begin = end;
        }

        fenwick_tree<int> seen(static_cast<int>(rotation_changes_.size()));
        for (int i = 0; i < n; ++i)
        {
            const int rank = ranks_[static_cast<std::size_t>(i)];
            inversions_ += i - seen.sum(0, rank + 1);
            seen.add(rank, 1);
        }
        visit_left_rotations([&](std::int64_t value)
        {
            rotation_sum_ += value;
        });
    }

    CyclicInversionStatistics(const CyclicInversionStatistics &) = default;

    CyclicInversionStatistics(CyclicInversionStatistics &&other) noexcept
        : ranks_(std::move(other.ranks_)), rotation_changes_(std::move(other.rotation_changes_)),
          inversions_(std::exchange(other.inversions_, 0)), rotation_sum_(std::exchange(other.rotation_sum_, 0))
    {
        other.ranks_.clear();
        other.rotation_changes_.clear();
    }

    CyclicInversionStatistics &operator=(CyclicInversionStatistics other) noexcept
    {
        ranks_.swap(other.ranks_);
        rotation_changes_.swap(other.rotation_changes_);
        std::swap(inversions_, other.inversions_);
        std::swap(rotation_sum_, other.rotation_sum_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return ranks_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return ranks_.empty();
    }

    [[nodiscard]] std::int64_t inversion_count() const noexcept
    {
        return inversions_;
    }

    [[nodiscard]] std::int64_t rotation_inversion_sum() const noexcept
    {
        return rotation_sum_;
    }

    [[nodiscard]] std::vector<std::int64_t> left_rotation_counts() const
    {
        std::vector<std::int64_t> result;
        result.reserve(size());
        visit_left_rotations([&](std::int64_t value)
        {
            result.push_back(value);
        });
        return result;
    }

    // 隣り合う二つの回転差分の重みは1だけ異なる。
    void swap_adjacent(std::size_t position)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position >= size() || position + 1 >= size())
        {
            throw std::out_of_range("CyclicInversionStatistics: adjacent positions");
        }
#endif
        const auto first = ranks_[position];
        const auto second = ranks_[position + 1];
        const int difference = (first < second) - (first > second);
        inversions_ += difference;
        rotation_sum_ += static_cast<std::int64_t>(size()) * difference
            + rotation_changes_[static_cast<std::size_t>(second)] - rotation_changes_[static_cast<std::size_t>(first)];
        std::swap(ranks_[position], ranks_[position + 1]);
    }
};

#endif
