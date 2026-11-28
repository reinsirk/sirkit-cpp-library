// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::PostmanEdge> edges(m);
    for (auto &edge : edges)
    {
        verify_in >> edge.from >> edge.to >> edge.weight;
        --edge.from;
        --edge.to;
    }
    verify_out << sirkit::minimum_postman_route(n, edges, 0)->cost << '\n';
}
