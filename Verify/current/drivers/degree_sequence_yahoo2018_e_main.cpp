// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <numeric>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<int> degrees(n);
    for (auto &degree : degrees)
    {
        verify_in >> degree;
    }
    if (sirkit::is_graphical_degree_sequence(degrees))
    {
        verify_out << "YES\n";
        return 0;
    }

    const auto total = std::accumulate(degrees.begin(), degrees.end(), 0LL);
    if ((total & 1u) != 0)
    {
        // 1増やして作れることと、最大次数を1減らして作れることは同値。
        --degrees.back();
        if (sirkit::is_graphical_degree_sequence(degrees))
        {
            verify_out << "NO\n";
            return 0;
        }
    }
    verify_out << "ABSOLUTELY NO\n";
}
