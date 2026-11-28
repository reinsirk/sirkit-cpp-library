// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    int n{};
    std::size_t m{};
    verify_in >> n >> m;
    std::vector<sirkit::StabbingInterval> intervals(m);
    for (auto &[left, right, weight] : intervals)
    {
        verify_in >> left >> right >> weight;
        --left;
    }
    verify_out << sirkit::maximize_interval_stabbing_weight(n, intervals).weight << '\n';
}
