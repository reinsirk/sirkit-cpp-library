// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Weighted_Wavelet_Matrix.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<int> keys{3, 1, 3, -2};
    const std::vector<std::int64_t> weights{8, 10, -4, 7};
    const sirkit::WeightedWaveletMatrix<int> matrix(keys, weights);
    const auto [count, total] = matrix.count_sum_less_equal(0, 4, 1);
    assert(count == 2 && total == 17);
    assert(matrix.sum(0, 4) == 21);
}
