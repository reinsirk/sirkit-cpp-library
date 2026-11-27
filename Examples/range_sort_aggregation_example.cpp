// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Sort_Aggregation.cpp"

#include <cassert>
#include <vector>

struct Affine
{
    int a, b;
};

constexpr Affine compose(Affine left, Affine right) noexcept
{
    return {right.a * left.a, right.a * left.b + right.b};
}

constexpr Affine identity() noexcept
{
    return {1, 0};
}

int main()
{
    using Data = sirkit::RangeSortAggregation<Affine, compose, identity>;
    const std::vector<Data::Item> items{{3, {2, 0}}, {1, {1, 1}}};
    Data data(items);
    const auto before = data.prod(0, 2);
    data.sort(0, 2);
    const auto after = data.prod(0, 2);
    assert(before.a == 2 && before.b == 1);
    assert(after.a == 2 && after.b == 2);
    data.sort(0, 2, true);
    assert(data.all_prod().b == 1);
}
