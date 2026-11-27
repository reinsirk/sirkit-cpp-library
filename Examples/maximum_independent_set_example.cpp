// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Maximum_Independent_Set.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    assert((sirkit::maximum_independent_set(3, edges) == std::vector<int>{0, 2}));

    const std::array<std::int64_t, 3> weights{2, 5, 2};
    const auto optimum = sirkit::maximum_weight_independent_set(weights, edges);
    assert(optimum.weight == 5 && optimum.vertices == std::vector<int>{1});
}
