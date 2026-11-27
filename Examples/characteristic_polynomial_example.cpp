// SPDX-License-Identifier: CC0-1.0
#include "../Math/Characteristic_Polynomial.cpp"
#include <cassert>

int main()
{
    const std::vector<std::vector<std::int64_t>> a{{1, 2}, {0, 3}};
    assert((sirkit::characteristic_polynomial(a) == std::vector<std::uint32_t>{3, 998244349, 1}));
}
