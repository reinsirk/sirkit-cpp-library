// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Majority.cpp"

#include <cassert>
#include <optional>
#include <vector>

int main()
{
    const std::vector values{2, 1, 2, 4};
    const std::vector<sirkit::RangeMajorityOperation<int>> operations{
        sirkit::RangeMajorityQuery{0, 4},
        sirkit::RangeMajoritySet<int>{1, 2},
        sirkit::RangeMajorityQuery{0, 4},
        sirkit::RangeMajorityQuery{3, 4},
    };
    const auto answers = sirkit::range_majority_queries(values, operations);
    assert((answers == std::vector<std::optional<int>>{std::nullopt, 2, 4}));
}
