// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{}, width{};
    verify_in >> n >> width;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto minimum = sirkit::sliding_window_extrema(values, width);
    for (std::size_t i = 0; i < minimum.size(); ++i)
    {
        verify_out << minimum[i] << (i + 1 == minimum.size() ? '\n' : ' ');
    }
}
