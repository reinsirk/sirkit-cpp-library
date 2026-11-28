// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <atcoder/modint>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::int64_t forbidden{};
    verify_in >> n >> forbidden;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto result = sirkit::count_partitions_avoiding_sums<atcoder::modint998244353>(values, forbidden);
    verify_out << result.val() << '\n';
}
