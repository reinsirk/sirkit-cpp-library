// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Min_Cost_B_Flow.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 2> supply{1, -1};
    const std::array edges{sirkit::BFlowEdge{0, 1, 0, 2, 1}, sirkit::BFlowEdge{1, 0, 0, 1, -3}};
    const auto result = sirkit::min_cost_b_flow(supply, edges);
    assert(result && result->flow[0] == 2 && result->flow[1] == 1);
}
