// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

int main()
{
    int tests{};
    verify_in >> tests;
    std::array<std::uint64_t, 30> left{}, right{};
    for (int test = 0; test < tests; ++test)
    {
        std::size_t left_count{}, right_count{};
        verify_in >> left_count;
        for (std::size_t i = 0; i < left_count; ++i)
        {
            verify_in >> left[i];
        }
        verify_in >> right_count;
        for (std::size_t i = 0; i < right_count; ++i)
        {
            verify_in >> right[i];
        }
        const auto common = sirkit::xor_basis_intersection(std::span(left).first(left_count),
                                                           std::span(right).first(right_count));
        verify_out << common.size();
        for (const auto value : common)
        {
            verify_out << ' ' << value;
        }
        verify_out << '\n';
    }
}
