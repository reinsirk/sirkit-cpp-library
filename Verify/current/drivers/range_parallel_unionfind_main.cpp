// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    constexpr int kMod = 998244353;
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> sums(n);
    for (auto &value : sums)
    {
        verify_in >> value;
    }
    RangeParallelUnionFind uf(n);
    int answer = 0;
    const auto joined = [&](int keep, int absorbed)
    {
        const auto product = static_cast<std::int64_t>(sums[keep]) * sums[absorbed];
        answer = static_cast<int>((answer + product) % kMod);
        sums[keep] += sums[absorbed];
        if (sums[keep] >= kMod)
        {
            sums[keep] -= kMod;
        }
    };
    while (q-- > 0)
    {
        int length = 0, a = 0, b = 0;
        verify_in >> length >> a >> b;
        uf.merge_range(a, b, length, joined);
        verify_out << answer << '\n';
    }
}
