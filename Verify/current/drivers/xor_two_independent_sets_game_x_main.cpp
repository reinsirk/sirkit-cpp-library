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
        std::size_t count{};
        std::uint64_t target{};
        verify_in >> count >> target;
        sirkit::XorTwoIndependentSets groups;
        for (std::size_t j = 0; j < count; ++j)
        {
            std::uint64_t value{};
            verify_in >> value;
            groups.insert(value);
        }
        groups.insert(target);
        verify_out << (groups.insert(target) ? "Bob\n" : "Alice\n");
    }
}
