#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_WEIGHTED_L1_STATISTICS_DEFINED
#define SIRKIT_WEIGHTED_L1_STATISTICS_DEFINED

namespace detail
{
struct L1Moment
{
    FractionWide value;

    L1Moment &operator+=(const L1Moment &other) noexcept
    {
        value = value + other.value;
        return *this;
    }

    friend L1Moment operator-(const L1Moment &a, const L1Moment &b) noexcept
    {
        return {a.value - b.value};
    }
};
}

class WeightedL1Statistics
{
    using Coordinate = std::int64_t;
    using Weight = std::uint64_t;
    using Moment = detail::FractionWide;
    static constexpr Weight kMax = std::numeric_limits<Weight>::max();

public:
    WeightedL1Statistics() = default;

    explicit WeightedL1Statistics(std::span<const Coordinate> coordinates)
        : coordinates_(coordinates.begin(), coordinates.end())
    {
        std::sort(coordinates_.begin(), coordinates_.end());
        coordinates_.erase(std::unique(coordinates_.begin(), coordinates_.end()), coordinates_.end());
        (void)detail::CheckedFenwickStorageSize(coordinates_.size(), coordinates_.max_size());
        const auto n = static_cast<int>(coordinates_.size());
        counts_ = fenwick_tree<Weight>(n);
        moments_ = fenwick_tree<detail::L1Moment>(n);
    }

    WeightedL1Statistics(const WeightedL1Statistics &) = default;

    WeightedL1Statistics(WeightedL1Statistics &&other) noexcept
    {
        swap(other);
    }

    WeightedL1Statistics &operator=(WeightedL1Statistics other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(WeightedL1Statistics &other) noexcept
    {
        coordinates_.swap(other.coordinates_);
        counts_.swap(other.counts_);
        moments_.swap(other.moments_);
        std::swap(total_, other.total_);
        std::swap(moment_, other.moment_);
    }

    void insert(Coordinate x, Weight weight = 1)
    {
        const int index = Index(x);
        if (weight > kMax - total_)
        {
            throw std::overflow_error("WeightedL1Statistics: total weight out of range");
        }
        Apply(index, weight, Moment::product(Encode(x), weight));
    }

    bool erase(Coordinate x, Weight weight = 1)
    {
        const auto index = Index(x);
        if (weight > counts_.get(index))
        {
            return false;
        }
        Apply(index, Weight{} - weight, Moment{} - Moment::product(Encode(x), weight));
        return true;
    }

    [[nodiscard]] Weight weight_at(Coordinate x) const
    {
        return counts_.get(Index(x));
    }

    [[nodiscard]] Weight total_weight() const noexcept
    {
        return total_;
    }

    [[nodiscard]] std::optional<Coordinate> kth(Weight rank) const
    {
        if (rank >= total_)
        {
            return std::nullopt;
        }
        return Select(rank);
    }

    [[nodiscard]] std::optional<std::pair<Coordinate, Coordinate>> minimizer_interval() const
    {
        if (total_ == 0)
        {
            return std::nullopt;
        }
        return std::pair{Select((total_ - 1) / 2), Select(total_ / 2)};
    }

    [[nodiscard]] Weight absolute_deviation(Coordinate x)
    {
        const int end = static_cast<int>(std::upper_bound(coordinates_.begin(), coordinates_.end(), x) - coordinates_.begin());
        const auto left_count = counts_.sum(0, end);
        const auto left_moment = moments_.sum(0, end).value;
        const auto encoded = Encode(x);
        const auto left = Moment::product(encoded, left_count) - left_moment;
        const auto right = (moment_ - left_moment) - Moment::product(encoded, total_ - left_count);
        const auto sum = left + right;
        if (!sum.fits(kMax))
        {
            throw std::overflow_error("WeightedL1Statistics: absolute deviation out of range");
        }
        return sum.low();
    }

private:
    std::vector<Coordinate> coordinates_;
    fenwick_tree<Weight> counts_;
    fenwick_tree<detail::L1Moment> moments_;
    Weight total_ = 0;
    Moment moment_;

    static Weight Encode(Coordinate x) noexcept
    {
        // 最小のint64_tを原点として、符号付き座標の距離を非負で保持する。
        return static_cast<Weight>(x) ^ (Weight{1} << 63);
    }

    [[nodiscard]] int Index(Coordinate x) const
    {
        const auto it = std::lower_bound(coordinates_.begin(), coordinates_.end(), x);
#ifndef SKIP_RUNTIME_CHECKS
        if (it == coordinates_.end() || *it != x)
        {
            throw std::out_of_range("WeightedL1Statistics: coordinate is not registered");
        }
#endif
        return static_cast<int>(it - coordinates_.begin());
    }

    [[nodiscard]] Coordinate Select(Weight rank) const
    {
        return coordinates_[counts_.lower_bound(rank + 1)];
    }

    void Apply(int index, Weight mass, Moment moment)
    {
        // 削除はそれぞれ2^64、2^128を法とする負の差分。更新後の実値は各型に収まる。
        counts_.add(index, mass);
        moments_.add(index, {moment});
        total_ += mass;
        moment_ = moment_ + moment;
    }
};

#endif
