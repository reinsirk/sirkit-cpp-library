// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Deadline_Job_Selection.cpp
// 参考（締切付き仕事の選択）: https://atcoder.jp/contests/typical90/tasks/typical90_k
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/deadline_job_selection.md
//
// API一覧
// DeadlineJob{duration,deadline,reward}: 所要時間・完了の締切（時刻0から測る）・報酬。所要時間0も可。
// DeadlineJobSchedule{reward,jobs}: 最大報酬と、時刻0から実行する仕事の0始まりの添字列。
// maximize_deadline_job_reward(jobs): 各仕事を高々1回だけ続けて実行し、時間を重ねずに締切を満たす最大報酬を求める。
// 全件を見送ってもよい。最大報酬はint64_tに収まること。
// n=件数、D=min(最大締切,所要時間総和)。O(1+n log(n+1)+n(D+1))時間、O(n+D+1)語とO(n(D+1))ビット。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DEADLINE_JOB_SELECTION_DEFINED
#define SIRKIT_DEADLINE_JOB_SELECTION_DEFINED

struct DeadlineJob
{
    std::size_t duration;
    std::size_t deadline;
    std::int64_t reward;
};

struct DeadlineJobSchedule
{
    std::int64_t reward;
    std::vector<std::size_t> jobs;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t DeadlineJobTableWidth(std::size_t horizon, std::size_t rows)
{
    if (horizon >= std::vector<std::int64_t>{}.max_size())
    {
        throw std::length_error("maximize_deadline_job_reward: time horizon too large");
    }
    const std::size_t width = horizon + 1;
    if (rows > std::vector<bool>{}.max_size() / width)
    {
        throw std::length_error("maximize_deadline_job_reward: schedule table too large");
    }
    return width;
}

inline std::int64_t AddDeadlineJobReward(std::int64_t reward, std::int64_t increment)
{
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(reward) + static_cast<std::uint64_t>(increment));
}
} // namespace reinsirk::detail

[[nodiscard]] inline DeadlineJobSchedule maximize_deadline_job_reward(std::span<const DeadlineJob> jobs)
{
    std::vector<std::size_t> order;
    std::size_t largest_deadline = 0;
    for (std::size_t i = 0; i < jobs.size(); ++i)
    {
        if (jobs[i].duration <= jobs[i].deadline && jobs[i].reward > 0)
        {
            order.push_back(i);
            largest_deadline = std::max(largest_deadline, jobs[i].deadline);
        }
    }
    if (order.empty())
    {
        return {0, {}};
    }

    std::ranges::sort(order, [&](std::size_t left, std::size_t right)
    {
        return std::pair{jobs[left].deadline, left} < std::pair{jobs[right].deadline, right};
    });
    std::size_t horizon = 0;
    for (const std::size_t id : order)
    {
        horizon += std::min(jobs[id].duration, largest_deadline - horizon);
    }
    const std::size_t width = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::DeadlineJobTableWidth(horizon, order.size());
    std::vector<std::int64_t> reward(width, -1);
    reward[0] = 0;
    std::vector<bool> used(width * order.size());

    std::size_t available = 0;
    for (std::size_t row = 0; row < order.size(); ++row)
    {
        const auto &[duration, deadline, value] = jobs[order[row]];
        available += std::min(duration, horizon - available);
        const std::size_t last = std::min(deadline, available);

        // 終了時刻を降順に更新し、各仕事を同じ行で二度選ばない。
        for (std::size_t before = last - duration + 1; before != 0;)
        {
            --before;
            const std::size_t finish = before + duration;
            if (reward[before] < 0)
            {
                continue;
            }
            const auto candidate = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddDeadlineJobReward(reward[before], value);
            if (candidate > reward[finish])
            {
                reward[finish] = candidate;
                used[row * width + finish] = true;
            }
        }
    }

    const auto best = std::ranges::max_element(reward);
    DeadlineJobSchedule result{*best, {}};
    auto finish = static_cast<std::size_t>(best - reward.begin());
    for (std::size_t row = order.size(); row != 0;)
    {
        --row;
        if (used[row * width + finish])
        {
            const auto id = order[row];
            result.jobs.push_back(id);
            finish -= jobs[id].duration;
        }
    }
    std::ranges::reverse(result.jobs);
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
