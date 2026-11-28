// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint64_t min_step{}, max_step{};
    verify_in >> n >> min_step >> max_step;
    std::vector<std::int64_t> values(n), weights(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> values[i] >> weights[i];
    }
    verify_out << sirkit::bounded_zigzag_subsequence(values, weights, min_step, max_step).weight << '\n';
}
