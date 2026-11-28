// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/State_Cycle.cpp"
#include <cassert>
#include <functional>

int main()
{
    const auto next = [](int state) { return state == 3 ? 1 : state + 1; };
    const auto cycle = sirkit::find_state_cycle(0, next, 30);
    assert(cycle && cycle->prefix == 1 && cycle->period == 3);
    const int sum = sirkit::fold_state_sequence(0, 8, next, std::identity{}, std::plus<>{}, 0);
    assert(sum == 13);
}
