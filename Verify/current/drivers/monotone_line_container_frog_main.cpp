// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int n = 0;
    std::int64_t jump = 0, height = 0;
    verify_in >> n >> jump >> height;
    sirkit::MonotoneMinLineContainer hull;
    hull.reserve(n);
    hull.add(-2 * height, height * height);
    std::int64_t cost = 0;
    for (int i = 1; i < n; ++i)
    {
        verify_in >> height;
        cost = hull.query_monotone(height)->value + height * height + jump;
        hull.add(-2 * height, height * height + cost);
    }
    verify_out << cost << '\n';
}
