// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <functional>

int main()
{
    std::uint64_t count = 0, start = 0, modulus = 0;
    verify_in >> count >> start >> modulus;
    const auto next = [modulus](std::uint64_t state) { return state * state % modulus; };
    verify_out << sirkit::fold_state_sequence(start, count, next, std::identity{}, std::plus<>{}, std::uint64_t{0}) << '\n';
}
