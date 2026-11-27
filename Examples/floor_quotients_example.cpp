// SPDX-License-Identifier: CC0-1.0
#include "../Math/Floor_Quotients.cpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const auto values = sirkit::enumerate_quotients(10);
    assert((values == std::vector<std::uint64_t>{1, 2, 3, 5, 10}));
    std::uint64_t sum = 0;
    sirkit::for_each_floor_quotient(10, [&](auto first, auto last, auto quotient)
                                    {
                                        sum += (last - first + 1) * quotient;
                                    });
    assert(sum == 27);
}
