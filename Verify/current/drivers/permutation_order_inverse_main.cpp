// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n;
    std::uint64_t rank;
    verify_in >> rank >> n;
    const auto values = *sirkit::permutation_unrank(n, rank);
    std::vector<int> inverse(n);
    for (int position = 0; position < n; ++position)
    {
        inverse[values[position]] = position;
    }
    verify_out << *sirkit::permutation_rank(inverse) << '\n';
}
