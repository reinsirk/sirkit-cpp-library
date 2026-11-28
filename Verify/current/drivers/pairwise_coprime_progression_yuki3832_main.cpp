// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    std::size_t tests{};
    verify_in >> tests;
    for (std::size_t i = 0; i < tests; ++i)
    {
        std::uint64_t length{}, step{};
        verify_in >> length >> step;
        verify_out << (length <= sirkit::max_pairwise_coprime_progression_length(step) ? "Yes\n" : "No\n");
    }
}
