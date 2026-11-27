// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>

int main()
{
    std::uint64_t n{};
    verify_in >> n;
    const auto quotients = enumerate_quotients(n);
    verify_out << quotients.size() << '\n';
    for (std::size_t i = 0; i < quotients.size(); ++i)
    {
        verify_out << quotients[i] << (i + 1 == quotients.size() ? '\n' : ' ');
    }
}
