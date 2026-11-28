// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::uint32_t count{};
    verify_in >> count;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries(count);
    for (auto &[n, k] : queries)
    {
        verify_in >> n >> k;
    }
    for (const auto value : sirkit::binomial_prefix_sums(queries))
    {
        verify_out << value << '\n';
    }
}
