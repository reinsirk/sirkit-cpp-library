#include "../Algorithm/Weighted_Interval_Stabbing.cpp"

#include <array>
#include <cassert>
#include <vector>

int main()
{
    const std::array<sirkit::StabbingInterval, 3> intervals{{
        {0, 3, 10},
        {1, 4, -10},
        {2, 5, 10},
    }};
    const auto result = sirkit::maximize_interval_stabbing_weight(5, intervals);
    assert(result.weight == 20);
    assert((result.points == std::vector<int>{0, 4}));
}
