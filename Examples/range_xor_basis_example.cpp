// SPDX-License-Identifier: CC0-1.0
#include "../Math/Range_Xor_Basis.cpp"
#include <cassert>

int main()
{
    const std::vector<std::uint64_t> a{3, 6, 9};
    const std::vector<sirkit::RangeXorQuery> qs{{0, 2, 5}};
    const auto ans = sirkit::range_xor_queries(a, qs);
    assert(ans[0].can_make && ans[0].maximum == 6 && ans[0].rank == 2);
}
