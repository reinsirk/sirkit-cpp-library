// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    std::uint64_t length{};
    int bound{};
    verify_in >> length >> bound;
    std::uint64_t sum = 0;
    for (const auto count : sirkit::divisibility_chain_end_counts(length, bound))
    {
        sum += count;
    }
    verify_out << sum % 998244353 << '\n';
}
