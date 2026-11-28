#include "../../../Algorithm/Deadline_Job_Selection.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::DeadlineJob;
using sirkit::maximize_deadline_job_reward;

std::int64_t brute(std::span<const DeadlineJob> jobs)
{
    const unsigned subsets = 1U << jobs.size();
    std::vector<std::size_t> times(subsets);
    std::vector<std::int64_t> rewards(subsets);
    std::vector<bool> possible(subsets);
    possible[0] = true;
    std::int64_t best = 0;
    for (unsigned subset = 1; subset < subsets; ++subset)
    {
        const auto first = static_cast<std::size_t>(std::countr_zero(subset));
        const auto rest = subset ^ (1U << first);
        times[subset] = times[rest] + jobs[first].duration;
        rewards[subset] = rewards[rest] + jobs[first].reward;
        for (std::size_t last = 0; last < jobs.size(); ++last)
        {
            const unsigned bit = 1U << last;
            if ((subset & bit) && possible[subset ^ bit] && times[subset] <= jobs[last].deadline)
            {
                possible[subset] = true;
                best = std::max(best, rewards[subset]);
                break;
            }
        }
    }
    return best;
}

void verify(std::span<const DeadlineJob> jobs)
{
    const auto schedule = maximize_deadline_job_reward(jobs);
    const std::int64_t expected = brute(jobs);
    assert(schedule.reward == expected);
    std::vector<bool> seen(jobs.size());
    std::size_t time = 0;
    std::int64_t reward = 0;
    for (const auto id : schedule.jobs)
    {
        assert(id < jobs.size() && !seen[id]);
        seen[id] = true;
        const auto &[duration, deadline, value] = jobs[id];
        time += duration;
        reward += value;
        assert(time <= deadline);
    }
    assert(reward == expected);
}

template <typename Exception, typename Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    verify({});
    verify(std::array{DeadlineJob{0, 0, 4}, DeadlineJob{0, 0, 3}, DeadlineJob{1, 1, 3}});
    verify(std::array{DeadlineJob{1, 0, 99}, DeadlineJob{0, 0, -10}, DeadlineJob{0, 0, 0}});
    verify(std::array{DeadlineJob{4, 4, 6}, DeadlineJob{1, 4, 2}, DeadlineJob{3, 4, 5}, DeadlineJob{1, 6, 1}});
    verify(std::array{DeadlineJob{3, 10, 7}, DeadlineJob{4, 10, 9}, DeadlineJob{1, 10, 20}});

    std::mt19937 random(6716011);
    for (int trial = 0; trial < 1600; ++trial)
    {
        std::vector<DeadlineJob> jobs(random() % 12);
        for (auto &job : jobs)
        {
            job.duration = random() % 9;
            job.deadline = random() % 18;
            job.reward = static_cast<std::int64_t>(random() % 25) - 8;
        }
        verify(jobs);
    }

    constexpr auto high = std::numeric_limits<std::int64_t>::max();
    const auto at_limit = maximize_deadline_job_reward(std::array{DeadlineJob{0, 0, high}});
    assert(at_limit.reward == high && at_limit.jobs.size() == 1 && at_limit.jobs[0] == 0);
    constexpr auto size_max = std::numeric_limits<std::size_t>::max();
    expect_exception<std::length_error>([] {
        (void)maximize_deadline_job_reward(std::array{DeadlineJob{size_max, size_max, 1}});
    });
    expect_exception<std::length_error>([] { (void)sirkit::detail::DeadlineJobTableWidth(0, size_max); });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::overflow_error>([] {
        (void)maximize_deadline_job_reward(std::array{DeadlineJob{0, 0, high}, DeadlineJob{1, 1, 1}});
    });
#endif
}
