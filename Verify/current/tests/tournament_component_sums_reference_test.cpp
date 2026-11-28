// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tournament_Component_Sums.cpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
struct Mod
{
    static constexpr std::uint64_t kMod = 998244353;
    std::uint64_t value;
    explicit Mod(std::uint64_t x) : value(x % kMod)
    {
    }
    friend Mod operator+(Mod a, Mod b)
    {
        return Mod(a.value + b.value);
    }
    friend Mod operator*(Mod a, Mod b)
    {
        return Mod(a.value * b.value);
    }
};

std::vector<std::uint64_t> brute(int n)
{
    std::vector<std::pair<int, int>> edges;
    for (int u = 0; u < n; ++u)
    {
        for (int v = u + 1; v < n; ++v)
        {
            edges.emplace_back(u, v);
        }
    }
    std::vector<std::uint64_t> answer(edges.size() + 1);
    for (unsigned orientation = 0; orientation < (1U << edges.size()); ++orientation)
    {
        std::array<unsigned, 6> reach{};
        for (int u = 0; u < n; ++u)
        {
            reach[u] = 1U << u;
        }
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto [u, v] = edges[id];
            if ((orientation >> id) & 1U)
            {
                reach[u] |= 1U << v;
            }
            else
            {
                reach[v] |= 1U << u;
            }
        }
        for (int via = 0; via < n; ++via)
        {
            for (int u = 0; u < n; ++u)
            {
                if ((reach[u] >> via) & 1U)
                {
                    reach[u] |= reach[via];
                }
            }
        }
        unsigned components = 0;
        for (int u = 0; u < n; ++u)
        {
            bool representative = true;
            for (int v = 0; v < u; ++v)
            {
                if (((reach[u] >> v) & 1U) && ((reach[v] >> u) & 1U))
                {
                    representative = false;
                    break;
                }
            }
            components += representative;
        }
        answer[std::popcount(orientation)] += components;
    }
    return answer;
}

std::uint64_t power(std::uint64_t base, int exponent)
{
    std::uint64_t answer = 1;
    while (exponent > 0)
    {
        if (exponent % 2)
        {
            answer = answer * base % Mod::kMod;
        }
        base = base * base % Mod::kMod;
        exponent /= 2;
    }
    return answer;
}
} // namespace

int main()
{
    bool caught = false;
    try
    {
        (void)sirkit::tournament_component_sums<int>(-1);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
    for (int n = 0; n <= 6; ++n)
    {
        assert(sirkit::tournament_component_sums<std::uint64_t>(n) == brute(n));
    }
    const auto colors = sirkit::tournament_component_sums<Mod>(25);
    assert(colors[156].value == 902739687);

    constexpr int kSize = 30;
    constexpr int kEdges = kSize * (kSize - 1) / 2;
    const auto counts = sirkit::tournament_component_sums<Mod>(kSize);
    assert(counts.size() == kEdges + 1);
    assert(counts.front().value == kSize && counts.back().value == kSize);
    std::uint64_t total = 0;
    for (int i = 0; i <= kEdges; ++i)
    {
        assert(counts[i].value == counts[kEdges - i].value);
        total = (total + counts[i].value) % Mod::kMod;
    }

    std::uint64_t expected = power(2, kEdges);
    std::uint64_t choose = 1;
    for (int split = 1; split < kSize; ++split)
    {
        choose = choose * (kSize - split + 1) / split;
        expected = (expected + choose * power(2, kEdges - split * (kSize - split))) % Mod::kMod;
    }
    assert(total == expected);
}
