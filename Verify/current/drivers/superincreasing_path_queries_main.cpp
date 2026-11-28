// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

namespace
{
struct Residue
{
    static constexpr int kModulus = 1000000007;
    int value{};

    friend Residue operator+(Residue a, Residue b)
    {
        const int sum = a.value + b.value;
        return {sum >= kModulus ? sum - kModulus : sum};
    }

    friend Residue operator-(Residue a, Residue b)
    {
        const int difference = a.value - b.value;
        return {difference < 0 ? difference + kModulus : difference};
    }
};
} // namespace

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::SuperincreasingPathEdge<Residue>> edges(m);
    Residue power{2};
    for (auto &edge : edges)
    {
        verify_in >> edge.from >> edge.to;
        --edge.from;
        --edge.to;
        edge.value = power;
        power = power + power;
    }
    const sirkit::SuperincreasingPathQueries<Residue> queries(n, edges);
    int q{};
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        int u{}, v{};
        std::size_t removed{};
        verify_in >> u >> v >> removed;
        const auto answer = queries.path_sum_without_edge(u - 1, v - 1, removed - 1);
        verify_out << (answer ? answer->value : -1) << '\n';
    }
}
