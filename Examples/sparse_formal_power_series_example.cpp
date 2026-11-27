// SPDX-License-Identifier: CC0-1.0
#include "../Math/Sparse_Formal_Power_Series.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array linear{sirkit::SparseFpsTerm{0, 1}, sirkit::SparseFpsTerm{1, 998244352}};
    assert((sirkit::sparse_fps_inverse(4, linear) == std::vector<std::uint32_t>{1, 1, 1, 1}));
    const std::array monomial{sirkit::SparseFpsTerm{3, 2}};
    assert((sirkit::sparse_fps_exp(6, monomial) == std::vector<std::uint32_t>{1, 0, 0, 2, 0, 0}));
    assert((sirkit::sparse_fps_log(2, linear) == std::vector<std::uint32_t>{0, 998244352}));
    assert((sirkit::sparse_fps_pow(3, linear, 2) == std::vector<std::uint32_t>{1, 998244351, 1}));
    const std::array square{sirkit::SparseFpsTerm{0, 1}, sirkit::SparseFpsTerm{2, 1}, sirkit::SparseFpsTerm{1, 2}};
    assert((sirkit::sparse_fps_sqrt(3, square) == std::vector<std::uint32_t>{1, 1, 0}));
}
