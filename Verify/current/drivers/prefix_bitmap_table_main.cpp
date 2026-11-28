// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>
#include <vector>

int main()
{
    std::size_t n{}, width{}, q{};
    verify_in >> n >> width >> q;
    std::vector<std::string> rows(n);
    for (auto &row : rows)
    {
        verify_in >> row;
    }
    sirkit::PrefixBitmapTable<> table(rows);
    std::string prefix;
    for (std::size_t query = 0; query < q; ++query)
    {
        int type{};
        verify_in >> type;
        if (type == 1)
        {
            std::size_t column{};
            char from{}, to{};
            verify_in >> column >> from >> to;
            table.replace(column - 1, from, to);
        }
        else
        {
            verify_in >> prefix;
            verify_out << table.prefix_count(prefix) << '\n';
        }
    }
}
