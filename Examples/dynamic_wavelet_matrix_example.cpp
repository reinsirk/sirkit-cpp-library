// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Dynamic_Wavelet_Matrix.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::uint32_t> values{8, 2, 5};
    sirkit::DynamicWaveletMatrix matrix{std::span<const std::uint32_t>(values)};
    matrix.insert(1, 3);
    assert(matrix.kth_smallest(0, 3, 1) == 3);
    assert(matrix.count_less(0, 4, 5) == 2);
    matrix.set(0, 3);
    assert(matrix.count(0, 4, 3) == 2);
    assert(matrix.erase(2) == 2);
    assert(matrix.access(2) == 5);
}
