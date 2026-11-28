#include "../Algorithm/Subarray_Sum_Counts.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 4> values{3, -1, 2, 4};
    const auto count = sirkit::count_subarrays_by_sum(
        values, {.minimum_sum = 3, .maximum_sum = 5, .maximum_length = 3});
    assert(count == 4);
}
