// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace
{
struct Count
{
    static constexpr std::uint64_t kMod = 1000000007;
    std::uint64_t value;
    explicit Count(std::uint64_t count) : value(count % kMod)
    {
    }
    friend Count operator*(Count a, Count b)
    {
        return Count(a.value * b.value);
    }
};
} // namespace

int main()
{
    int n{};
    std::uint64_t colors{};
    verify_in >> n >> colors;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    verify_out << sirkit::ForestColoring(n, edges).count_distance_two<Count>(colors).value << '\n';
}
