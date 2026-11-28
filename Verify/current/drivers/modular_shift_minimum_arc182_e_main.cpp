// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint32_t modulus{}, count{};
    std::int64_t step{};
    verify_in >> n >> modulus >> step >> count;
    std::vector<std::int64_t> offsets(n);
    for (auto &offset : offsets)
    {
        verify_in >> offset;
    }
    verify_out << sirkit::sum_minimum_modular_shifts(count, modulus, step, offsets) << '\n';
}
