// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Weighted_Slope_Trick.cpp"

#include <cassert>

int main()
{
    sirkit::WeightedSlopeTrick cost;
    cost.add_abs(2, 3);
    cost.add_abs(5, 1);
    assert(cost.min_value() == 3);
    assert(cost.argmin().lower == 2);
    cost.shift(0, 4);
    assert(cost.argmin().upper == 6 && cost.eval(6) == 3);
}
