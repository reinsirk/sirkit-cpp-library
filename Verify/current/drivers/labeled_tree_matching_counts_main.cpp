// SPDX-License-Identifier: CC0-1.0
#include <cstddef>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    std::size_t limit = 0;
    int type = 0;
    verify_in >> limit >> type;
    if (type == 1)
    {
        const auto counts = labeled_tree_exposable_root_counts(limit);
        for (std::size_t n = 2; n <= limit; ++n)
        {
            verify_out << counts[n] << '\n';
        }
    }
    else
    {
        const auto perfect = labeled_tree_perfect_matching_counts(limit);
        for (std::size_t n = 2; n <= limit; ++n)
        {
            const auto cayley = sirkit::detail::fps_power(static_cast<std::uint32_t>(n), n - 2);
            verify_out << sirkit::detail::fps_subtract(cayley, perfect[n]) << '\n';
        }
    }
}
