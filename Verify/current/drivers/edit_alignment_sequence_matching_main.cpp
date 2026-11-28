// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <span>
#include <vector>

int main()
{
    int n = 0, m = 0;
    verify_in >> n >> m;
    std::vector<int> a(n), b(m);
    for (auto &value : a)
    {
        verify_in >> value;
    }
    for (auto &value : b)
    {
        verify_in >> value;
    }
    verify_out << sirkit::edit_alignment(std::span{a}, std::span{b}).distance << '\n';
}
