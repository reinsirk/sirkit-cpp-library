// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, m{};
    verify_in >> n >> m;
    std::vector<std::uint64_t> a(n), b(m);
    for (auto &entry : a)
    {
        verify_in >> entry;
    }
    for (auto &entry : b)
    {
        verify_in >> entry;
    }
    const auto result = sirkit::convolution_f2_64(a, b);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
