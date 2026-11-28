// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint64_t> weights(n);
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    verify_out << sirkit::optimal_adjacent_merge(weights).cost << '\n';
}
