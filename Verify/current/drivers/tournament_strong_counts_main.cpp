// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    int n{};
    verify_in >> n;
    const auto probabilities = sirkit::tournament_sink_size_probabilities(n);
    constexpr unsigned long long kMod = 998244353;
    std::uint64_t expected = 0;
    for (std::size_t k = 1; k < probabilities.size(); ++k)
    {
        expected = (expected + k * static_cast<std::uint64_t>(probabilities[k])) % kMod;
    }
    verify_out << expected << '\n';
}
