// SPDX-License-Identifier: CC0-1.0
// BinaryMatrixを先に貼り付けた場合の共有bitset演算を検証する。
#include "../../../Math/Binary_Matrix.cpp"
#include "../../../DataStructure/Dynamic_Bitset.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

int main() {
    for (std::size_t size : {0, 127, 257, 4029, 4097, 8321}) {
        sirkit::DynamicBitset left(size), right(size);
        std::size_t total = 0, common = 0;
        for (std::size_t i = 0; i < size; ++i) {
            const bool a = i % 5 == 3;
            const bool b = i % 7 < 4;
            left.set(i, a);
            right.set(i, b);
            total += a;
            common += a && b;
        }
        assert(left.count() == total);
        assert(left.intersection_count(right) == common);
    }
    sirkit::BinaryMatrix matrix(1, 2);
    matrix.set(0, 0);
    const std::vector<std::uint8_t> rhs{1};
    auto solution = matrix.solve(rhs);
    assert(solution);
    sirkit::BinaryMatrix::LinearSolution empty;
    swap(*solution, empty);
    assert(solution->particular.empty());
    const std::vector<std::uint8_t> expected{1, 0};
    assert(empty.particular == expected);
    assert(matrix.row_words(0).front() == 1);
    assert(!(matrix == sirkit::BinaryMatrix(2, 2)));
    assert(!(matrix == sirkit::BinaryMatrix(1, 3)));
    assert(!(matrix == sirkit::BinaryMatrix(1, 2)));
    auto zero = matrix;
    zero ^= matrix;
    assert(zero == sirkit::BinaryMatrix(1, 2));
#ifndef SKIP_RUNTIME_CHECKS
    bool wrong_height = false;
    try {
        zero ^= sirkit::BinaryMatrix(2, 2);
    } catch (const std::invalid_argument&) {
        wrong_height = true;
    }
    assert(wrong_height && zero == sirkit::BinaryMatrix(1, 2));
#endif
    // 横幅が64ワードを超えていても、内積の結果は手計算できる。
    sirkit::BinaryMatrix left(1, 4097), right(4097, 1);
    left.set(0, 0);
    left.set(0, 4096);
    right.set(0, 0);
    assert((left * right).test(0, 0));
    right.set(4096, 0);
    assert(!(left * right).test(0, 0));
}
