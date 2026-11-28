#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <vector>

#ifndef SIRKIT_DOMINANCE_COUNTS_DEFINED
#define SIRKIT_DOMINANCE_COUNTS_DEFINED

template <typename Point, typename XLess = std::less<>, typename YLess = std::less<>>
[[nodiscard]] std::vector<int> dominance_counts(const std::vector<Point> &points,
                                                bool strict_x = false, bool strict_y = false,
                                                XLess x_less = {}, YLess y_less = {})
{
    const auto count = points.size();
    static_cast<void>(detail::CheckedFenwickStorageSize(count, std::vector<int>().max_size()));
    std::vector<int> order(count);
    std::iota(order.begin(), order.end(), 0);
    const auto by_y = [&](int a, int b)
    {
        return std::invoke(y_less, points[static_cast<std::size_t>(a)].second,
                           points[static_cast<std::size_t>(b)].second);
    };
    std::ranges::sort(order, by_y);

    int values = 0;
    std::vector<int> ranks(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        if (i == 0 || by_y(order[i - 1], order[i]))
        {
            ++values;
        }
        ranks[static_cast<std::size_t>(order[i])] = values - 1;
    }

    const auto by_x = [&](int a, int b)
    {
        return std::invoke(x_less, points[static_cast<std::size_t>(a)].first,
                           points[static_cast<std::size_t>(b)].first);
    };
    std::ranges::sort(order, by_x);
    fenwick_tree<int> tree(values);
    std::vector<int> answer(count);
    std::size_t inserted = 0;
    for (std::size_t begin = 0, end = 0; begin < count; begin = end)
    {
        end = begin + 1;
        while (end < count && !by_x(order[begin], order[end]))
        {
            ++end;
        }
        // 狭義なら同じxの点を入れる前、広義なら入れた後に答える。
        const auto through = strict_x ? begin : end;
        while (inserted < through)
        {
            const auto index = static_cast<std::size_t>(order[inserted++]);
            tree.add(ranks[index], 1);
        }
        for (std::size_t i = begin; i < end; ++i)
        {
            const auto index = static_cast<std::size_t>(order[i]);
            answer[index] = tree.sum(0, ranks[index] + static_cast<int>(!strict_y));
        }
    }
    return answer;
}

#endif
