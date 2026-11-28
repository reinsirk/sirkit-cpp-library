// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Krawtchouk_Transform.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr auto kMod = std::uint64_t{998244353};
using sirkit::FormalPowerSeries;

FormalPowerSeries direct(const FormalPowerSeries &input)
{
    if (input.empty())
    {
        return {};
    }
    const auto degree = input.size() - 1;
    std::vector<std::uint32_t> result(input.size());
    for (std::size_t i = 0; i <= degree; ++i)
    {
        std::vector<std::uint32_t> term(input.size());
        term[0] = input[i];
        for (std::size_t factor = 0; factor < degree; ++factor)
        {
            const auto sign = factor < i ? kMod - 1 : 1;
            for (auto j = factor + 1; j > 0; --j)
            {
                term[j] = static_cast<std::uint32_t>((term[j] + sign * term[j - 1]) % kMod);
            }
        }
        for (std::size_t j = 0; j < result.size(); ++j)
        {
            result[j] = static_cast<std::uint32_t>((result[j] + term[j]) % kMod);
        }
    }
    return FormalPowerSeries(std::move(result));
}
} // namespace

int main()
{
    using sirkit::inverse_krawtchouk_transform;
    using sirkit::krawtchouk_transform;
    assert(krawtchouk_transform({}).empty() && inverse_krawtchouk_transform({}).empty());
    assert(krawtchouk_transform({5}) == FormalPowerSeries({5}));
    assert(inverse_krawtchouk_transform({5}) == FormalPowerSeries({5}));
    assert(krawtchouk_transform({1, 0, 0}) == FormalPowerSeries({1, 2, 1}));
    assert(krawtchouk_transform({0, 1, 0}) == FormalPowerSeries({1, 0, -1}));
    assert(krawtchouk_transform({0, 0, 1}) == FormalPowerSeries({1, -2, 1}));

    bool rejected = false;
    try
    {
        static_cast<void>(krawtchouk_transform(FormalPowerSeries(FormalPowerSeries::max_precision() + 1)));
    }
    catch (const std::length_error &)
    {
        rejected = true;
    }
    assert(rejected);

    std::mt19937 rng(20261128);
    for (std::size_t trial = 0; trial < 160; ++trial)
    {
        std::vector<std::uint32_t> values(1 + rng() % 65);
        for (auto &v : values)
        {
            v = static_cast<std::uint32_t>(rng() % kMod);
        }
        values.back() = trial % 2 == 0 ? 0 : values.back();
        const FormalPowerSeries input(std::move(values));
        const auto actual = krawtchouk_transform(input);
        assert(actual == direct(input));
        assert(inverse_krawtchouk_transform(actual) == input);
        assert(krawtchouk_transform(inverse_krawtchouk_transform(input)) == input);
    }
}
