// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int sets{}, n{};
    verify_in >> sets >> n;
    const auto graph = [&]
    {
        std::vector<std::vector<int>> data(sets);
        std::vector<sirkit::Hyperedge> groups;
        groups.reserve(sets);
        for (auto &members : data)
        {
            int size{};
            verify_in >> size;
            members.resize(size);
            for (auto &v : members)
            {
                verify_in >> v;
                --v;
            }
            groups.push_back({members});
        }
        return sirkit::HypergraphShortestPaths(n, groups);
    }();
    const int moves = graph.unit_distances(0).back();
    verify_out << (moves == -1 ? -1 : moves - 1) << '\n';
}
