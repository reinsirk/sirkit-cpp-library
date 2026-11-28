// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Incident_Label_Spanning_Tree.cpp"

#include <array>
#include <iostream>

int main()
{
    using Edge = sirkit::LabeledSpanningEdge<char>;
    const std::array required{'R', 'R', 'B'};
    const std::array edges{Edge{0, 1, 'R'}, Edge{1, 2, 'B'}, Edge{0, 2, 'B'}};
    const auto result = sirkit::incident_label_spanning_tree<char>(required, edges).value();
    for (const auto edge : result)
    {
        std::cout << edge << ' ';
    }
    std::cout << '\n';
}
