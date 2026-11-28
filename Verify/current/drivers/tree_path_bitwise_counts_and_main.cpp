// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<sirkit::TreePathBitwiseEdge> edges(n - 1);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
        --from;
        --to;
    }
    const auto result = sirkit::tree_path_bitwise_counts(n, edges);
    constexpr std::uint64_t kMod = 1000000007;
    std::uint64_t sum = 0;
    std::uint64_t power = 1;
    for (auto count : result.and_pairs)
    {
        sum = (sum + count % kMod * power) % kMod;
        power = 2 * power % kMod;
    }
    verify_out << sum << '\n';
}
