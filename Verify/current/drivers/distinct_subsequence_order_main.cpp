// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <string>

int main()
{
    std::string text;
    std::uint64_t rank = 0;
    verify_in >> text >> rank;
    const sirkit::DistinctSubsequenceOrder order(text);
    const auto answer = order.kth_smallest(rank);
    if (answer)
    {
        verify_out << *answer << '\n';
    }
    else
    {
        verify_out << "Eel\n";
    }
}
