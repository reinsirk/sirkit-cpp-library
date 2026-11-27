// SPDX-License-Identifier: CC0-1.0
#include "../Math/Two_Square_Sum.cpp"
#include <cassert>

int main()
{
    const auto pairs = sirkit::two_square_sum(25);
    assert(pairs.size() == 4 && pairs[1].first == 3 && pairs[1].second == 4);
    assert(sirkit::two_square_sum(7).empty());
}
