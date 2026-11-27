// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>

int main()
{
    std::size_t queries{};
    verify_in >> queries;
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::uint64_t a{}, b{};
        verify_in >> a >> b;
        verify_out << sirkit::nim_product(a, b) << '\n';
    }
}
