// SPDX-License-Identifier: CC0-1.0
#include "../String/Prefix_Bitmap_Table.cpp"

#include <array>
#include <cassert>
#include <string_view>

int main()
{
    const std::array<std::string_view, 4> rows{"aaa", "aba", "bbb", "ccc"};
    sirkit::PrefixBitmapTable<> table(rows);
    assert(table.prefix_count("aa") == 1);
    table.replace(1, 'b', 'a');
    assert(table.prefix_count("aa") == 2);
}
