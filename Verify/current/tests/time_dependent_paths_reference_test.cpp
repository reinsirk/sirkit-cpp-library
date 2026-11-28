// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Time_Dependent_Paths.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

namespace
{
constexpr int kHorizon = 9;
using Edge = std::pair<int, int>;
using Timetable = std::array<int, kHorizon + 1>;
using Time = std::optional<std::int64_t>;

std::vector<Time> brute(int n, std::span<const Edge> edges, std::span<const Timetable> schedules,
                        int start, int start_time, bool reversed)
{
    const auto state = [](int vertex, int time)
    {
        return vertex * (kHorizon + 1) + time;
    };
    std::vector<std::vector<int>> transitions(n * (kHorizon + 1));
    const auto add = [&](int from, int to)
    {
        if (reversed)
        {
            std::swap(from, to);
        }
        transitions[from].push_back(to);
    };
    for (int u = 0; u < n; ++u)
    {
        for (int t = 0; t < kHorizon; ++t)
        {
            add(state(u, t), state(u, t + 1));
        }
    }
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        for (int t = 0; t <= kHorizon; ++t)
        {
            const int duration = schedules[id][t];
            if (duration >= 0 && t + duration <= kHorizon)
            {
                add(state(u, t), state(v, t + duration));
            }
        }
    }

    std::vector<bool> seen(transitions.size());
    std::vector<int> pending{state(start, start_time)};
    seen[pending.front()] = true;
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        for (const int next : transitions[pending[i]])
        {
            if (!seen[next])
            {
                seen[next] = true;
                pending.push_back(next);
            }
        }
    }

    std::vector<Time> answer(n);
    for (int u = 0; u < n; ++u)
    {
        for (int t = 0; t <= kHorizon; ++t)
        {
            if (seen[state(u, t)])
            {
                if (!answer[u] || reversed)
                {
                    answer[u] = t;
                }
            }
        }
    }
    return answer;
}

void check(int n, const std::vector<Edge> &edges, const std::vector<Timetable> &schedules, int start, int start_time)
{
    const auto arrive = [&](std::size_t id, std::int64_t ready) -> Time
    {
        int best = kHorizon + 1;
        for (int depart = static_cast<int>(ready); depart <= kHorizon; ++depart)
        {
            const auto duration = schedules[id][depart];
            if (duration >= 0)
            {
                best = std::min(best, depart + duration);
            }
        }
        return best <= kHorizon ? Time(best) : std::nullopt;
    };
    const auto leave = [&](std::size_t id, std::int64_t deadline) -> Time
    {
        for (int depart = static_cast<int>(deadline); depart >= 0; --depart)
        {
            const auto duration = schedules[id][depart];
            if (duration >= 0 && depart + duration <= deadline)
            {
                return depart;
            }
        }
        return std::nullopt;
    };
    assert(sirkit::earliest_arrival_times(n, edges, start, start_time, arrive) ==
           brute(n, edges, schedules, start, start_time, false));
    assert(sirkit::latest_departure_times(n, edges, start, start_time, leave) ==
           brute(n, edges, schedules, start, start_time, true));
}
} // namespace

int main()
{
    Timetable slow, fast, absent;
    slow.fill(7);
    fast.fill(1);
    absent.fill(-1);
    check(4, {{0, 1}, {0, 1}, {1, 2}}, {slow, fast, absent}, 0, 0);
    check(4, {{0, 1}, {0, 1}, {1, 2}}, {slow, fast, absent}, 1, 9);
    check(2, {}, {}, 1, 0);

    std::mt19937 rng(5087427);
    for (int trial = 0; trial < 1050; ++trial)
    {
        const int n = 1 + rng() % 8;
        const int m = rng() % 16;
        std::vector<Edge> edges;
        std::vector<Timetable> schedules(m);
        for (auto &schedule : schedules)
        {
            edges.emplace_back(rng() % n, rng() % n);
            for (auto &duration : schedule)
            {
                duration = static_cast<int>(rng() % 7) - 2;
            }
        }
        check(n, edges, schedules, rng() % n, rng() % (kHorizon + 1));
    }

    constexpr auto kMinimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMaximum = std::numeric_limits<std::int64_t>::max();
    const auto same = [](std::size_t, std::int64_t time) -> Time
    {
        return time;
    };
    const std::array edges{Edge{0, 1}};
    assert(sirkit::earliest_arrival_times(2, edges, 0, kMaximum, same)[1] == kMaximum);
    assert(sirkit::latest_departure_times(2, edges, 1, kMinimum, same)[0] == kMinimum);

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    for (const auto &[from, to] : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        const Edge invalid{from, to};
        try
        {
            (void)sirkit::earliest_arrival_times(2, std::span(&invalid, 1), 0, 0, same);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    for (const int start : {-1, 2})
    {
        try
        {
            (void)sirkit::latest_departure_times(2, edges, start, 0, same);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    try
    {
        (void)sirkit::earliest_arrival_times(2, edges, 0, 0, [](std::size_t, std::int64_t) -> Time
                                             {
                                                 return -1;
                                             });
    }
    catch (const std::invalid_argument &)
    {
        ++rejected;
    }
    try
    {
        (void)sirkit::latest_departure_times(2, edges, 1, 0, [](std::size_t, std::int64_t) -> Time
                                             {
                                                 return 1;
                                             });
    }
    catch (const std::invalid_argument &)
    {
        ++rejected;
    }
    assert(rejected == 8);
#endif
}
