// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>

int main()
{
    std::size_t n{};
    verify_in >> n;
    constexpr std::uint32_t kMod = 998244353;
    sirkit::RelaxedConvolution convolution;
    auto prefix = convolution.push(1, 1);
    for (std::size_t i = 0; i < n; ++i)
    {
        std::uint32_t weight{};
        verify_in >> weight;
        const auto current = static_cast<std::uint32_t>(std::uint64_t{prefix} * weight % kMod);
        verify_out << current << (i + 1 == n ? '\n' : ' ');
        prefix += convolution.push(current, current);
        prefix %= kMod;
    }
}
