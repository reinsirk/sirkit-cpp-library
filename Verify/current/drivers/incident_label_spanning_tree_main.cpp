// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::LabeledSpanningEdge<char>> edges(m);
    for (auto &edge : edges)
    {
        verify_in >> edge.from >> edge.to >> edge.label;
        --edge.from;
        --edge.to;
    }
    std::string required;
    verify_in >> required;
    const auto answer = sirkit::incident_label_spanning_tree<char>(required, edges);
    if (!answer)
    {
        verify_out << "No\n";
        return 0;
    }
    verify_out << "Yes\n";
    for (const auto id : *answer)
    {
        verify_out << id + 1 << ' ';
    }
    verify_out << '\n';
}
