// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t size{}, queries{};
    verify_in >> size >> queries;
    std::vector<std::uint32_t> values(size);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::SubsetXorSquareRange<> squares(values);
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::uint32_t type{}, value_or_right{};
        std::size_t index_or_left{};
        verify_in >> type >> index_or_left >> value_or_right;
        if (type == 1)
        {
            squares.set(index_or_left - 1, value_or_right);
        }
        else
        {
            verify_out << squares.sum_squares(index_or_left - 1, value_or_right) << '\n';
        }
    }
}
