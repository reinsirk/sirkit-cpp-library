// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <atcoder/modint>
#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    using Mint = atcoder::modint998244353;
    std::size_t n{}, k{};
    verify_in >> n >> k;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[a, b] : edges)
    {
        verify_in >> a >> b;
        --a;
        --b;
    }
    const sirkit::TreeSubsetMedians medians(n, edges);
    std::vector<Mint> factorial(n + 1, 1), inverse(n + 1);
    for (std::size_t i = 1; i <= n; ++i)
    {
        factorial[i] = factorial[i - 1] * Mint(i);
    }
    inverse[n] = factorial[n].inv();
    for (std::size_t i = n; i != 0; --i)
    {
        inverse[i - 1] = inverse[i] * Mint(i);
    }
    const auto choose = [&](std::size_t size, std::size_t count)
    {
        return factorial[size] * inverse[count] * inverse[size - count];
    };
    verify_out << (medians.median_count_sum(k, choose) / choose(n, k)).val() << '\n';
}
