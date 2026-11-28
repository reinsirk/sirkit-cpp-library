// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Digit_DP.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>

int main()
{
    const auto count = sirkit::digit_dp_counts<std::uint64_t>("25", 2, 0, [](std::size_t seen, unsigned digit, bool leading_zero)
                                                              {
                                                                  return seen || (digit == 0 && !leading_zero);
                                                              });
    assert(count[0] == 24 && count[1] == 2);
}
