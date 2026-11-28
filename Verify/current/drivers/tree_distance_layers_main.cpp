// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

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
    const sirkit::TreeDistanceLayers layers(n, edges);
    int q{};
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        int vertex{}, distance{};
        verify_in >> vertex >> distance;
        const int answer = layers.vertex_at_distance(vertex - 1, distance);
        verify_out << (answer < 0 ? -1 : answer + 1) << '\n';
    }
}
