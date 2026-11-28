// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    std::uint64_t length{};
    verify_in >> n >> m >> length;
    constexpr std::uint32_t kMod = 998244353;
    std::vector<sirkit::ExactLengthWalkArc<std::uint32_t>> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
        weight %= kMod;
    }
    const auto add = [](std::uint32_t a, std::uint32_t b)
    {
        return (a + b) % kMod;
    };
    const auto multiply = [](std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>(static_cast<std::uint64_t>(a) * b % kMod);
    };
    const auto answer = sirkit::fold_exact_length_walks(n, edges, 0, length, 0U, 1U, add, multiply);
    verify_out << answer.back() << '\n';
}
