// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <atcoder/modint>
#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n;
    std::size_t m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[from, to] : edges)
    {
        verify_in >> from >> to;
    }
    verify_out << count_eulerian_circuits<atcoder::modint998244353>(n, edges).val() << '\n';
}
