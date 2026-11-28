// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint64_t> source(n), target(n);
    for (auto &value : source)
    {
        verify_in >> value;
    }
    for (auto &value : target)
    {
        verify_in >> value;
    }
    for (const auto [shift, mask] : sirkit::cyclic_xor_matches(source, target))
    {
        verify_out << shift << ' ' << mask << '\n';
    }
}
