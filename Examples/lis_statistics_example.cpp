// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/LIS_Statistics.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector values{1, 4, 3, 2, 5};
    const auto [length, count] = sirkit::count_lis(values);
    assert(length == 3 && count == 3);
    const auto membership = sirkit::lis_membership(values);
    assert(membership.front() == sirkit::LISMembership::kAll);
    assert(membership[1] == sirkit::LISMembership::kSome);
}
