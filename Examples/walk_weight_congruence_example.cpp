// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Walk_Weight_Congruence.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::WalkCongruenceEdge> edges{{0, 1, 5}, {1, 0, -3}};
    const sirkit::WalkWeightCongruence weights(2, edges);
    assert(weights.period(0) == 2);
    const auto relation = weights.relation(0, 1);
    assert(relation && relation->matches(3, 4));
    assert(!relation->matches(0, 4));
}
