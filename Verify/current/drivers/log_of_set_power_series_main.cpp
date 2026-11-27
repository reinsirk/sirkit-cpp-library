// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    unsigned n;
    verify_in >> n;
    std::vector<std::uint32_t> a(std::size_t{1} << n);
    for (auto &x : a)
    {
        verify_in >> x;
    }
    const auto result = set_series_log(a);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
