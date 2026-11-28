// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::uint32_t count{}, prime{}, q{};
    verify_in >> count >> prime >> q;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries(count);
    std::uint32_t max_n = 0;
    for (auto &[n, k] : queries)
    {
        verify_in >> n >> k;
        max_n = std::max(max_n, n);
    }
    const sirkit::PrimeQBinomial coefficients(prime, q, max_n);
    for (const auto &[n, k] : queries)
    {
        verify_out << coefficients.COM(n, k) << '\n';
    }
}
