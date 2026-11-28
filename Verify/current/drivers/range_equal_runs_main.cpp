// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::RangeEqualRuns<int> runs(values);
    for (std::size_t query = 0; query < q; ++query)
    {
        int type{};
        std::size_t x{}, y{};
        verify_in >> type >> x >> y;
        if (type == 1)
        {
            runs.set(x - 1, static_cast<int>(y));
        }
        else
        {
            verify_out << runs.query(x - 1, y).longest << '\n';
        }
    }
}
