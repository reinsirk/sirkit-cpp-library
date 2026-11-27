// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Deque_Aggregation.cpp"

#include <cassert>
#include <cstdint>

namespace
{
struct Affine
{
    std::int64_t a, b;
};

struct Compose
{
    Affine operator()(Affine left, Affine right) const noexcept
    {
        return {right.a * left.a, right.a * left.b + right.b};
    }
};
}

int main()
{
    sirkit::DequeAggregation functions(Affine{1, 0}, Compose{});
    functions.push_back({2, 0});
    functions.push_front({1, 3});
    const auto both = functions.all_prod();
    assert(both.a == 2 && both.b == 6);
    functions.pop_back();
    const auto first = functions.all_prod();
    assert(first.a == 1 && first.b == 3);
}
