// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<double> probabilities(n);
    for (auto &probability : probabilities)
    {
        verify_in >> probability;
    }
    verify_out.fixed(12) << sirkit::poisson_binomial_at_least(probabilities, n / 2 + 1) << '\n';
}
