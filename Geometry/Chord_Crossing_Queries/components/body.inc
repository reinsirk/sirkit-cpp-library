#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

#ifndef SIRKIT_CHORD_CROSSING_QUERIES_DEFINED
#define SIRKIT_CHORD_CROSSING_QUERIES_DEFINED

[[nodiscard]] inline std::vector<int> count_chord_crossings(
    std::span<const std::pair<std::int64_t, std::int64_t>> chords,
    std::span<const std::pair<std::int64_t, std::int64_t>> queries)
{
    using Label = std::int64_t;
    using Chord = std::pair<Label, Label>;
    static_cast<void>(detail::CheckedFenwickStorageSize(chords.size(), std::vector<int>().max_size()));

    std::vector<Chord> by_left, by_right;
    std::vector<Label> endpoints;
    by_left.reserve(chords.size());
    by_right.reserve(chords.size());
    endpoints.reserve(chords.size() * 2);
    for (auto [a, b] : chords)
    {
        if (a > b)
        {
            std::swap(a, b);
        }
        if (a == b)
        {
            continue;
        }
        by_left.emplace_back(a, b);
        by_right.emplace_back(b, a);
        endpoints.push_back(a);
        endpoints.push_back(b);
    }
    std::ranges::sort(by_left);
    std::ranges::sort(by_right);
    std::ranges::sort(endpoints);

    struct Query
    {
        Label left;
        Label right;
        std::size_t id;
    };
    std::vector<Query> order;
    order.reserve(queries.size());
    for (std::size_t id = 0; id < queries.size(); ++id)
    {
        const auto [a, b] = queries[id];
        order.push_back({std::min(a, b), std::max(a, b), id});
    }
    std::ranges::sort(order, {}, &Query::right);

    const int size = static_cast<int>(by_left.size());
    fenwick_tree<int> ends_inside_right(size);
    std::size_t next = 0;
    std::vector<int> answer(queries.size());
    for (const auto &[left, right, id] : order)
    {
        if (left == right)
        {
            continue;
        }
        while (next < by_right.size() && by_right[next].first < right)
        {
            const auto begin = std::ranges::lower_bound(by_left, by_right[next].second, {}, &Chord::first);
            ends_inside_right.add(static_cast<int>(begin - by_left.begin()), 1);
            ++next;
        }
        const auto open_left_rank = std::ranges::upper_bound(by_left, left, {}, &Chord::first) - by_left.begin();
        const auto both_inside = ends_inside_right.sum(static_cast<int>(open_left_rank), size);
        const auto inside = std::ranges::lower_bound(endpoints, right) - std::ranges::upper_bound(endpoints, left);

        // 開弧に端点が一つだけある弦から、境界の点を共有する弦を除く。
        const auto touches_left = std::ranges::lower_bound(by_left, Chord{left, right}) -
                                  std::ranges::lower_bound(by_left, Chord{left, std::numeric_limits<Label>::min()});
        const auto touches_right = std::ranges::upper_bound(by_right, Chord{right, std::numeric_limits<Label>::max()}) -
                                   std::ranges::upper_bound(by_right, Chord{right, left});
        answer[id] = static_cast<int>(inside - 2LL * both_inside - touches_left - touches_right);
    }
    return answer;
}

#endif
