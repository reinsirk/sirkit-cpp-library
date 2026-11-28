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
    std::vector<std::int64_t> left(size), right(size);
    for (auto &value : left)
    {
        verify_in >> value;
    }
    for (auto &value : right)
    {
        verify_in >> value;
    }
    const sirkit::PairProductOrder products(left, right);
    verify_out << *products.kth_smallest(rank) << '\n';
}
