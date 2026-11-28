// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint32_t> labels(n);
    for (auto &label : labels)
    {
        verify_in >> label;
    }
    std::uint64_t weight_sum = 0;
    for (const auto &edge : sirkit::minimum_lcm_spanning_tree(labels))
    {
        weight_sum += edge.weight;
    }
    verify_out << weight_sum << '\n';
}
