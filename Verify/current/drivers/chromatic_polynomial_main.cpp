// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }
    const auto polynomial = sirkit::chromatic_polynomial(n, edges);
    verify_out << polynomial[0];
    for (std::size_t i = 1; i < polynomial.size(); ++i)
    {
        verify_out << ' ' << polynomial[i];
    }
    verify_out << '\n';
}
