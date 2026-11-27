// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t pairs{};
    verify_in >> pairs;
    const auto n = 2 * pairs;
    std::vector<std::vector<std::uint32_t>> matrix(n, std::vector<std::uint32_t>(n));
    for (auto &row : matrix)
    {
        for (auto &value : row)
        {
            verify_in >> value;
        }
    }
    verify_out << sirkit::pfaffian_mod(std::move(matrix)) << '\n';
}
