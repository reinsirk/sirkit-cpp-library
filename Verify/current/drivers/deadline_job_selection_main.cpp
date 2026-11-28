// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<sirkit::DeadlineJob> jobs(n);
    for (auto &[duration, deadline, reward] : jobs)
    {
        verify_in >> deadline >> duration >> reward;
    }
    verify_out << sirkit::maximize_deadline_job_reward(jobs).reward << '\n';
}
