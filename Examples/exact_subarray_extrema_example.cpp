#include "../Algorithm/Exact_Subarray_Extrema.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array values{1, 2, 3, 1};
    const auto result = sirkit::analyze_subarrays_with_extrema(values, 1, 3);
    assert(result.count == 4);
    assert(result.left == 0 && result.right == 4);
}
