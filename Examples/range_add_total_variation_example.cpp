#include "../DataStructure/Range_Add_Total_Variation.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 3> values{1, 2, 3};
    sirkit::RangeAddTotalVariation variation(values);
    variation.range_add(1, 3, 1);
    assert(variation.total_variation() == 3);
}
