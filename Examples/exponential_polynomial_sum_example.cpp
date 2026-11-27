// SPDX-License-Identifier: CC0-1.0
#include "../Math/Exponential_Polynomial_Sum.cpp"
#include <cassert>

int main()
{
    assert(sirkit::sum_exponential_powers(2, 2, 4) == 90);
    assert(sirkit::sum_exponential_powers_limit(2, 1) == 2);
    assert(!sirkit::sum_exponential_powers_limit(1, 1));
}
