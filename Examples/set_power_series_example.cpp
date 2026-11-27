// SPDX-License-Identifier: CC0-1.0
#include "../Math/Set_Power_Series.cpp"
#include <cassert>

int main()
{
    const std::vector<std::uint32_t> a{0, 2, 3, 4};
    assert((sirkit::set_series_exp(a) == std::vector<std::uint32_t>{1, 2, 3, 10}));
}
