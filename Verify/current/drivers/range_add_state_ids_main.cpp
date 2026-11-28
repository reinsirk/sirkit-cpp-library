// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <unordered_map>

int main()
{
    std::uint64_t n{};
    int q{};
    verify_in >> n >> q;
    sirkit::RangeAddStateIds states(n);
    std::unordered_map<sirkit::RangeAddStateIds::state_type, int> first{{states.id(), 0}};
    for (int step = 1; step <= q; ++step)
    {
        char kind{};
        verify_in >> kind;
        if (kind == '!')
        {
            std::uint64_t left{}, right{};
            std::int64_t delta{};
            verify_in >> left >> right >> delta;
            first.try_emplace(states.add(left, right, delta), step);
        }
        else
        {
            verify_out << first.at(states.id()) << '\n';
        }
    }
}
