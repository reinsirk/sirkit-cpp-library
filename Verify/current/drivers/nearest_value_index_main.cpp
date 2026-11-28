// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::NearestValueIndex points(values);
    for (std::size_t query = 0; query < q; ++query)
    {
        std::int64_t center{};
        std::size_t rank{};
        verify_in >> center >> rank;
        verify_out << points.kth_nearest(center, rank)->distance << '\n';
    }
}
