// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

int main()
{
    int contests{};
    verify_in >> contests;
    std::vector<std::int64_t> ratings(500000);
    std::iota(ratings.begin(), ratings.end(), std::int64_t{1});
    sirkit::MonotoneValueShift changes(ratings);
    for (int i = 0; i < contests; ++i)
    {
        std::int64_t low{}, high{};
        verify_in >> low >> high;
        changes.shift_between(low, high, 1);
    }
    const auto final_ratings = changes.values();
    std::size_t q{};
    verify_in >> q;
    for (std::size_t query = 0; query < q; ++query)
    {
        std::size_t x{};
        verify_in >> x;
        verify_out << final_ratings[x - 1] << '\n';
    }
}
