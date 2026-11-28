// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Top_K_Cartesian_Sums.cpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::vector<std::int64_t>> groups{{1, 4}, {2, 5}};
    assert((sirkit::top_k_cartesian_sums(groups, 3) == std::vector<std::int64_t>{9, 6, 6}));
    assert((sirkit::top_k_cartesian_sums(groups, 3, sirkit::CartesianSumOrder::smallest) == std::vector<std::int64_t>{3, 6, 6}));
}
