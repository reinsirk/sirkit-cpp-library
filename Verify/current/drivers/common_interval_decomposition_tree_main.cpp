// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<int> permutation(n);
    for (auto &value : permutation)
    {
        verify_in >> value;
    }
    const auto tree = sirkit::common_interval_decomposition(permutation);
    verify_out << tree.size() << '\n';
    for (const auto &interval : tree)
    {
        verify_out << interval.parent << ' ' << interval.left << ' ' << interval.right - 1
                   << (interval.kind == sirkit::CommonIntervalKind::prime ? " prime\n" : " linear\n");
    }
}
