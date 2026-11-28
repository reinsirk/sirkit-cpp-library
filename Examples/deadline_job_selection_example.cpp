#include "../Algorithm/Deadline_Job_Selection.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const std::array<sirkit::DeadlineJob, 3> jobs{{
        {3, 7, 20},
        {2, 3, 10},
        {3, 5, 15},
    }};
    const auto result = sirkit::maximize_deadline_job_reward(jobs);
    assert(result.reward == 35);
    assert((result.jobs == std::vector<std::size_t>{2, 0}));
}
