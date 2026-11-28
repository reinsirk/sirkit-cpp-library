// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main()
{
    std::uint32_t prime{}, generator{}, count{};
    verify_in >> prime >> generator >> count;
    const sirkit::PrimeDiscreteLogarithm table(prime, generator, count);
    while (count-- != 0)
    {
        std::uint32_t value{};
        verify_in >> value;
        verify_out << table.log(value) << '\n';
    }
}
