// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Assign_Range_Product.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

namespace
{
struct Affine
{
    std::int64_t a, b;
};

constexpr Affine Compose(Affine left, Affine right) noexcept
{
    return {right.a * left.a, right.a * left.b + right.b};
}

constexpr Affine Identity() noexcept
{
    return {1, 0};
}
}

int main()
{
    const std::vector<Affine> values{{1, 1}, {2, 0}, {1, 4}};
    sirkit::RangeAssignRangeProduct<Affine, Compose, Identity> data(values);
    const auto before = data.prod(0, 3);
    assert(before.a == 2 && before.b == 6);

    data.assign(1, 3, {3, 0});
    const auto after = data.prod(0, 3);
    assert(after.a == 9 && after.b == 9);
}
