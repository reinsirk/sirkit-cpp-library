// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const auto mul = sirkit::detail::fps_multiply;
    const auto pow = sirkit::detail::fps_power;
    constexpr auto kMod = sirkit::FormalPowerSeries::modulus();
    std::uint32_t n{};
    std::uint64_t steps{};
    verify_in >> n >> steps;
    std::vector<std::uint32_t> raw(static_cast<std::size_t>(n) + 1);
    const auto normalize = pow(1000000000 % kMod, kMod - 2);
    for (auto &coefficient : raw)
    {
        std::uint32_t frequency{};
        verify_in >> frequency;
        coefficient = mul(frequency % kMod, normalize);
    }
    auto spectral = sirkit::krawtchouk_transform(sirkit::FormalPowerSeries(std::move(raw)));
    const auto inverse_n = pow(n, kMod - 2);
    for (std::uint32_t j = 0; j <= n; ++j)
    {
        const auto eigenvalue = sirkit::detail::fps_subtract(1, mul(2 * j, inverse_n));
        spectral.set(j, mul(spectral[j], pow(eigenvalue, steps)));
    }
    const auto result = sirkit::inverse_krawtchouk_transform(spectral);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
