// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <vector>

int main()
{
    int n{}, k{};
    verify_in >> n >> k;
    using Permutation = sirkit::PermutationGroup::Permutation;
    std::vector<Permutation> generators(k, Permutation(n));
    for (auto &generator : generators)
    {
        for (auto &value : generator)
        {
            verify_in >> value;
        }
    }
    const sirkit::PermutationGroup group(n, generators);
    verify_out << group.order_mod(998'244'353) << '\n';
}
