#include <algorithm>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_INTERVAL_COVER_SUBSETS_DEFINED
#define SIRKIT_INTERVAL_COVER_SUBSETS_DEFINED

template <class T>
struct IntervalCoverChoice
{
    std::int64_t left, right;
    T omit{1}, take{1};
};

namespace detail
{
template <class T>
struct IntervalCoverOperations
{
    static T sum(const T &a, const T &b)
    {
        return a + b;
    }

    static T zero()
    {
        return {};
    }

    static T product(const T &a, const T &b)
    {
        return a * b;
    }

    static T one()
    {
        return T{1};
    }

    using Scores = lazy_segtree<T, sum, zero, T, product, product, one>;
};
} // namespace detail

template <class T>
[[nodiscard]] T count_interval_cover_subsets(
    std::int64_t left, std::int64_t right, std::span<const IntervalCoverChoice<T>> choices)
{
    if (left > right)
    {
        throw std::invalid_argument("count_interval_cover_subsets: reversed target");
    }
    std::vector<IntervalCoverChoice<T>> active;
    std::vector<std::int64_t> endpoints{left, right};
    T outside{1};
    for (const auto &interval : choices)
    {
        if (interval.left > interval.right)
        {
            throw std::invalid_argument("count_interval_cover_subsets: reversed interval");
        }
        const auto first = std::max(left, interval.left);
        const auto last = std::min(right, interval.right);
        if (first < last)
        {
            active.push_back({first, last, interval.omit, interval.take});
            endpoints.push_back(last);
        }
        else
        {
            outside = outside * (interval.omit + interval.take);
        }
    }
    std::ranges::sort(active, {}, &IntervalCoverChoice<T>::left);
    std::ranges::sort(endpoints);
    endpoints.erase(std::unique(endpoints.begin(), endpoints.end()), endpoints.end());
    const int n = detail::CheckedSegmentArrayCount(endpoints.size(), "count_interval_cover_subsets: too many endpoints");
    typename detail::IntervalCoverOperations<T>::Scores states(n);
    states.set(0, T{1});
    for (const auto &interval : active)
    {
        const int from = static_cast<int>(std::lower_bound(endpoints.begin(), endpoints.end(), interval.left) - endpoints.begin());
        const int to = static_cast<int>(std::lower_bound(endpoints.begin(), endpoints.end(), interval.right) - endpoints.begin());
        const T extend = states.prod(from, to) * interval.take;
        states.apply(from, to, interval.omit);
        states.apply(to, n, interval.omit + interval.take);
        states.set(to, states.get(to) + extend);
    }
    return states.get(n - 1) * outside;
}

#endif
