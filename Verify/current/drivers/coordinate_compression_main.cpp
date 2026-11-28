// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int h = 0, w = 0, n = 0;
    verify_in >> h >> w >> n;
    std::vector<int> rows(n), columns(n);
    for (int i = 0; i < n; ++i)
    {
        verify_in >> rows[i] >> columns[i];
    }
    const sirkit::CoordinateCompression row_ids(rows), column_ids(columns);
    for (int i = 0; i < n; ++i)
    {
        verify_out << row_ids.lower_bound(rows[i]) + 1 << ' ' << column_ids.lower_bound(columns[i]) + 1 << '\n';
    }
}
