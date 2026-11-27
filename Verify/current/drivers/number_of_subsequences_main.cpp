// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint32_t> sequence(n);
    for (auto &value : sequence)
    {
        verify_in >> value;
    }
    verify_out << sirkit::count_distinct_subsequences(sequence) << '\n';
}
