// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::int64_t> weights(n);
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    const sirkit::PathMatching<std::int64_t> matching(weights);
    for (const auto answer : matching.values().subspan(1))
    {
        verify_out << answer << '\n';
    }
}
