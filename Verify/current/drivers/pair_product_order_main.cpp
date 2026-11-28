// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t size = 0;
    std::uint64_t rank = 0;
    verify_in >> size >> rank;
    std::vector<std::int64_t> values(size);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::PairProductOrder products(values);
    verify_out << *products.kth_smallest(rank) << '\n';
}
