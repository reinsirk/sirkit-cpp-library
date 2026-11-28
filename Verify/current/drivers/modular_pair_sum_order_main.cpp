// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n = 0;
    std::uint64_t rank = 0, modulus = 0;
    verify_in >> n >> rank >> modulus;
    std::vector<std::uint64_t> left(n), right(n);
    for (auto &value : left)
    {
        verify_in >> value;
    }
    for (auto &value : right)
    {
        verify_in >> value;
    }
    const sirkit::ModularPairSumOrder order(left, right, modulus);
    verify_out << *order.kth_smallest(rank) << '\n';
}
