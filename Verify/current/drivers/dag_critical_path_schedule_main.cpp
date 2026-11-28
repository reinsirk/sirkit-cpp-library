// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::DagScheduleEdge> edges(m);
    for (auto &[u, v, duration] : edges)
    {
        verify_in >> u >> v >> duration;
    }
    const auto schedule = sirkit::dag_critical_path_schedule(n, edges, 0, n - 1).value();
    int flexible = 0;
    for (int u = 0; u < n; ++u)
    {
        if (!schedule.earliest[u] || schedule.earliest[u] != schedule.latest[u])
        {
            ++flexible;
        }
    }
    verify_out << schedule.duration << ' ' << flexible << '/' << n << '\n';
}
