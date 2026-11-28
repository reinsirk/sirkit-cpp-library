// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t x{}, y{}, z{}, k{};
    verify_in >> x >> y >> z >> k;
    std::vector<std::vector<std::int64_t>> groups(3);
    groups[0].resize(x);
    groups[1].resize(y);
    groups[2].resize(z);
    for (auto &group : groups)
    {
        for (auto &score : group)
        {
            verify_in >> score;
        }
    }
    for (const auto score : sirkit::top_k_cartesian_sums(groups, k))
    {
        verify_out << score << '\n';
    }
}
