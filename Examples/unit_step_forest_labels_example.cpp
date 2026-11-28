// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Unit_Step_Forest_Labels.cpp"

#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}};
    const std::vector<sirkit::IntegerLabelInterval> allowed{{4, 4}, {}, {6, 6}};
    const auto labels = sirkit::unit_step_forest_labels(3, edges, allowed);
    assert(labels && (*labels)[1] == 5);
}
