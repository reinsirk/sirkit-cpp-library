// SPDX-License-Identifier: CC0-1.0
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n, q;
    std::cin >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        std::cin >> value;
    }
    const sirkit::DisjointSubarrayMaxSum data(values);
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t count;
        std::cin >> count;
        std::cout << data.max_sum(count).value() << '\n';
    }
}
