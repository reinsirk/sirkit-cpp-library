// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    std::size_t n{};
    std::uint64_t p{}, q{};
    verify_in >> n >> p >> q;
    const auto count = sirkit::prefix_dominant_composition_counts(n, p, q);
    for (std::size_t length = 1; length <= n; ++length)
    {
        verify_out << count[length] << '\n';
    }
}
