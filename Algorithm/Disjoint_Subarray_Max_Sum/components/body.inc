#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#ifndef SIRKIT_DISJOINT_SUBARRAY_MAX_SUM_DEFINED
#define SIRKIT_DISJOINT_SUBARRAY_MAX_SUM_DEFINED

template <class Score = std::int64_t>
class DisjointSubarrayMaxSum
{
public:
    template <class Range>
        requires std::ranges::input_range<const Range>
    explicit DisjointSubarrayMaxSum(const Range &values)
        : matching_(path_weights(values))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::optional<Score> max_sum(std::size_t count) const
    {
        if (count > size_)
        {
            return std::nullopt;
        }
        return total_ + matching_.sums()[size_ - count];
    }

    [[nodiscard]] std::optional<std::vector<std::pair<std::size_t, std::size_t>>> intervals(std::size_t count) const
    {
        if (count > size_)
        {
            return std::nullopt;
        }
        const auto edges = matching_.positions(size_ - count);
        std::vector<std::pair<std::size_t, std::size_t>> result;
        result.reserve(count);
        std::size_t left = 0;
        const auto add_endpoint = [&](std::size_t vertex)
        {
            if (vertex % 2 == 0)
            {
                left = vertex / 2;
            }
            else
            {
                result.emplace_back(left, vertex / 2 + 1);
            }
        };
        std::size_t vertex = 0;
        for (const auto edge : *edges)
        {
            for (; vertex < edge; ++vertex)
            {
                add_endpoint(vertex);
            }
            vertex = edge + 2;
        }
        for (; vertex < 2 * size_; ++vertex)
        {
            add_endpoint(vertex);
        }
        return result;
    }

private:
    std::size_t size_ = 0;
    Score total_{};
    NonadjacentSelection<Score> matching_;

    template <class Range>
    std::vector<Score> path_weights(const Range &values)
    {
        std::vector<Score> weights;
        for (const auto &element : values)
        {
            if (!weights.empty())
            {
                weights.push_back(Score{});
            }
            const auto value = static_cast<Score>(element);
            weights.push_back(Score{} - value);
            total_ += value;
            ++size_;
        }
        return weights;
    }
};

#endif
