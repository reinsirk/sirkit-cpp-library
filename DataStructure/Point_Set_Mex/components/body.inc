#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifndef SIRKIT_POINT_SET_MEX_DEFINED
#define SIRKIT_POINT_SET_MEX_DEFINED

template <std::integral T = int>
    requires (sizeof(T) <= sizeof(std::uintmax_t))
class PointSetMex
{
    std::vector<T> values_;
    std::vector<std::size_t> counts_;
    HierarchicalBitset missing_;
    std::size_t mex_ = 0;

    [[nodiscard]] bool tracked(T value) const noexcept
    {
        if constexpr (std::signed_integral<T>)
        {
            if (value < 0)
            {
                return false;
            }
        }
        return static_cast<std::uintmax_t>(value) < counts_.size();
    }

    void check_position([[maybe_unused]] std::size_t position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position >= size())
        {
            throw std::out_of_range("PointSetMex: position");
        }
#endif
    }

public:
    PointSetMex() = default;

    explicit PointSetMex(std::vector<T> values)
        : values_(std::move(values)), counts_(values_.size() + 1)
    {
        for (T value : values_)
        {
            if (tracked(value))
            {
                ++counts_[static_cast<std::size_t>(value)];
            }
        }
        std::string marks(counts_.size(), '0');
        for (std::size_t i = 0; i < counts_.size(); ++i)
        {
            marks[i] = counts_[i] == 0 ? '1' : '0';
        }
        missing_ = HierarchicalBitset(marks);
        mex_ = *missing_.next(0);
    }

    PointSetMex(const PointSetMex &) = default;

    PointSetMex(PointSetMex &&other) noexcept
        : values_(std::move(other.values_)), counts_(std::move(other.counts_)), missing_(std::move(other.missing_)),
          mex_(std::exchange(other.mex_, 0))
    {
        other.values_.clear();
        other.counts_.clear();
    }

    PointSetMex &operator=(PointSetMex other) noexcept
    {
        values_.swap(other.values_);
        counts_.swap(other.counts_);
        std::swap(missing_, other.missing_);
        std::swap(mex_, other.mex_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    [[nodiscard]] T get(std::size_t position) const
    {
        check_position(position);
        return values_[position];
    }

    [[nodiscard]] std::size_t mex() const noexcept
    {
        return mex_;
    }

    void set(std::size_t position, T value)
    {
        check_position(position);
        const T previous = values_[position];
        if (previous == value)
        {
            return;
        }
        const auto removed = static_cast<std::size_t>(previous);
        if (tracked(previous) && --counts_[removed] == 0)
        {
            missing_.insert(removed);
            mex_ = std::min(mex_, removed);
        }
        const auto added = static_cast<std::size_t>(value);
        if (tracked(value) && counts_[added]++ == 0)
        {
            missing_.erase(added);
            if (mex_ == added)
            {
                // n個の値で0..nのすべてを埋めることはできない。
                mex_ = *missing_.next(added + 1);
            }
        }
        values_[position] = value;
    }
};

#endif
