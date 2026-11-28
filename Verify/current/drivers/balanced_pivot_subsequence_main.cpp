// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t n{}, queries{};
    std::string text;
    verify_in >> n >> queries >> text;
    const sirkit::BalancedPivotSubsequenceIndex index(text, '1', '/', '2');
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        const auto result = index.query(left - 1, right);
        verify_out << (result ? result->length() : 0) << '\n';
    }
}
