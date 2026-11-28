#include "../Algorithm/Subarray_Frequency_Windows.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array values{1, 2, 1, 2, 3};
    const auto result = sirkit::analyze_frequency_bounded_subarrays(
        values, {.maximum_distinct = 2, .maximum_occurrences = 1});
    assert(result.count == 9);
    assert(result.left == 0 && result.right == 2 && result.length() == 2);
}
