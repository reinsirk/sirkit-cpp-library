// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto histogram = sirkit::forest_subset_components<VerifyMatrixField>(n, edges);
    for (int k = 1; k <= n; ++k)
    {
        verify_out << histogram[k].get() << '\n';
    }
}
