// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
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
    std::vector<sirkit::IntegerLabelInterval> constraints(n);
    int fixed{};
    verify_in >> fixed;
    for (int i = 0; i < fixed; ++i)
    {
        int v{};
        std::int64_t value{};
        verify_in >> v >> value;
        constraints[v - 1] = {value, value};
    }
    const auto result = sirkit::unit_step_forest_labels(n, edges, constraints);
    if (!result)
    {
        verify_out << "No\n";
        return 0;
    }
    verify_out << "Yes\n";
    for (const auto value : *result)
    {
        verify_out << value << '\n';
    }
}
