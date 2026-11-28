// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_Roots.cpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Polynomial = sirkit::FormalPowerSeries;
constexpr auto kMod = Polynomial::modulus();

void Compare(std::vector<std::uint32_t> roots)
{
    // 3は原始根だから、x^2-3は法998244353で根を持たない。
    auto polynomial = Polynomial{-3, 0, 1}.scale(-27);
    for (const auto root : roots)
    {
        polynomial *= Polynomial{-static_cast<std::int64_t>(root), 1};
    }
    std::ranges::sort(roots);
    roots.erase(std::unique(roots.begin(), roots.end()), roots.end());
    assert(sirkit::polynomial_roots(std::move(polynomial), 14723) == roots);
}
}

int main()
{
    using sirkit::polynomial_roots;
    try
    {
        static_cast<void>(polynomial_roots({0, 0}, 1));
        assert(false);
    }
    catch (const std::domain_error &)
    {
    }
    try
    {
        Polynomial oversized((std::size_t{1} << 21) + 1);
        oversized.set(oversized.size() - 1, 1);
        static_cast<void>(polynomial_roots(std::move(oversized), 1));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    assert(polynomial_roots({4, 0}, 1).empty());
    assert((polynomial_roots({-28, 4}, 1) == std::vector<std::uint32_t>{7}));
    assert((polynomial_roots({0, 0, 27}, 1) == std::vector<std::uint32_t>{0}));
    Compare({});
    Compare({0, 0, 2, kMod - 1, kMod - 1, 9});
    for (std::uint64_t seed = 0; seed < 20; ++seed)
    {
        assert((polynomial_roots({0, -1, 1}, seed) == std::vector<std::uint32_t>{0, 1}));
    }
    std::mt19937 random(52389);
    for (const std::size_t count : {3, 13, 27, 66, 157})
    {
        std::vector<std::uint32_t> values(count);
        for (auto &root : values)
        {
            root = static_cast<std::uint32_t>(random() % kMod);
        }
        Compare(std::move(values));
    }
}
