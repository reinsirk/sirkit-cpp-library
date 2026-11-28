// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::uint64_t power(std::uint64_t base, std::uint64_t exponent)
{
    std::uint64_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if (exponent & 1U)
        {
            result = result * base % kMod;
        }
        base = base * base % kMod;
    }
    return result;
}
} // namespace

int main()
{
    int h{}, w{};
    verify_in >> h >> w;
    std::vector<std::pair<int, int>> edges;
    std::vector<int> above(w, -1);
    int n = 0;
    for (int row = 0; row < h; ++row)
    {
        std::string grid;
        verify_in >> grid;
        int left = -1;
        for (int col = 0; col < w; ++col)
        {
            int current = -1;
            if (grid[col] == '#')
            {
                current = n++;
                if (left != -1)
                {
                    edges.emplace_back(left, current);
                }
                if (above[col] != -1)
                {
                    edges.emplace_back(above[col], current);
                }
            }
            left = above[col] = current;
        }
    }
    const auto report = sirkit::vertex_deletion_components(n, edges);
    const auto total = std::accumulate(report.after_deleting.begin(), report.after_deleting.end(), std::uint64_t{0});
    verify_out << total % kMod * power(n, kMod - 2) % kMod << '\n';
}
