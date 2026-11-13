// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Wavelet_Matrix.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    WaveletMatrix matrix(std::vector<std::int64_t>{-5, 7, 2, -5, 10});
    assert(matrix.kth_smallest(0, 5, 1) == -5);
    assert(matrix.count(0, 5, -5) == 2);
    assert(matrix.range_freq(1, 5, 0, 8) == 2);
    assert(matrix.predecessor(0, 5, 2) == -5);
    assert(matrix.successor(0, 5, 3) == 7);
    std::cout << matrix.kth_largest(0, 5, 0) << '\n';
}
