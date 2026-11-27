// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n;
    std::uint64_t a, ratio;
    verify_in >> n >> a >> ratio;
    std::vector<std::uint32_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto result = *interpolate_geometric_polynomial(values, a, ratio);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
