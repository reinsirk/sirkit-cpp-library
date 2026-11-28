// SPDX-License-Identifier: CC0-1.0
#include "../Graph/MST_Sensitivity.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array edges{sirkit::SpanningTreeEdge{0, 1, 1},
                           sirkit::SpanningTreeEdge{1, 2, 2},
                           sirkit::SpanningTreeEdge{0, 2, 3}};
    const auto cuts = sirkit::minimum_spanning_forest_sensitivity(3, edges);
    std::cout << cuts.exchange[0].value() << '\n';
    std::cout << cuts.exchange[2].value() << '\n';
}
