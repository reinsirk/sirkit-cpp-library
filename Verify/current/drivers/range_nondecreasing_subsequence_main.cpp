// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>
#include <vector>

int main()
{
    int n{}, q{};
    std::string text;
    verify_in >> n >> q >> text;
    std::vector<int> symbols(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
    {
        symbols[static_cast<std::size_t>(i)] = text[static_cast<std::size_t>(i)] - 'A';
    }
    sirkit::RangeNondecreasingSubsequence<4> range(symbols);
    for (int query = 0; query < q; ++query)
    {
        int type{}, left{};
        verify_in >> type >> left;
        if (type == 1)
        {
            char symbol{};
            verify_in >> symbol;
            range.set(left - 1, symbol - 'A');
        }
        else
        {
            int right{};
            verify_in >> right;
            verify_out << range.replacement_count(left - 1, right) << '\n';
        }
    }
}
