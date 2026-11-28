// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto membership = sirkit::lis_membership(values);
    std::vector<int> mandatory;
    for (std::size_t i = 0; i < n; ++i)
    {
        if (membership[i] == sirkit::LISMembership::kAll)
        {
            mandatory.push_back(values[i]);
        }
    }
    verify_out << mandatory.size() << '\n';
    for (std::size_t i = 0; i < mandatory.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << mandatory[i];
    }
    verify_out << '\n';
}
