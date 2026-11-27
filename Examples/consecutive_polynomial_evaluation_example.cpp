// SPDX-License-Identifier: CC0-1.0
#include "../Math/Consecutive_Polynomial_Evaluation.cpp"
#include <cassert>

int main()
{
    const std::vector<std::uint32_t> f{1, 3, 7};
    assert((sirkit::evaluate_consecutive_polynomial(f, 2, 4) == std::vector<std::uint32_t>{7, 13, 21, 31}));
}
