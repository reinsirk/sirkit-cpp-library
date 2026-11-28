// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <atcoder/modint>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Mint = atcoder::static_modint<1'000'000'007>;
using Parity = sirkit::LinearMatroidParity<Mint>;

void set_vector(std::vector<Parity::Term> &terms, std::size_t first, std::size_t last, Mint base)
{
    terms.clear();
    Mint power = 1;
    for (auto index = first; index < last; ++index)
    {
        terms.push_back({index, power});
        power *= base;
    }
}
}

int main()
{
    std::size_t tests{};
    verify_in >> tests;
    std::random_device entropy;
    std::mt19937_64 random((std::uint64_t{entropy()} << 32) | entropy());
    std::uniform_int_distribution<int> coefficient(0, Mint::mod() - 1);
    for (std::size_t test = 0; test < tests; ++test)
    {
        std::size_t n{}, m{};
        verify_in >> n >> m;
        std::vector<std::size_t> offsets(n + 1);
        std::size_t max_capacity = 0;
        for (std::size_t i = 0; i < n; ++i)
        {
            std::size_t cap{};
            verify_in >> cap;
            offsets[i + 1] = offsets[i] + cap;
            max_capacity = std::max(max_capacity, cap);
        }
        std::vector<std::pair<std::size_t, std::size_t>> edges(m);
        for (auto &[u, v] : edges)
        {
            verify_in >> u >> v;
            --u;
            --v;
        }

        std::vector<Parity::Term> left, right;
        left.reserve(max_capacity);
        right.reserve(max_capacity);
        const auto upper = std::min(m, offsets.back() / 2);
        std::size_t answer = 0;
        for (int trial = 0; trial < 2; ++trial)
        {
            Parity model(offsets.back());
            for (std::size_t i = 0; i < m; ++i)
            {
                const auto [u, v] = edges[i];
                const Mint base = i + 1;
                set_vector(left, offsets[u], offsets[u + 1], base);
                set_vector(right, offsets[v], offsets[v + 1], base);
                model.add_pair(left, right, Mint::raw(coefficient(random)));
            }
            answer = std::max(answer, model.sample_size());
            if (answer == upper)
            {
                break;
            }
        }
        verify_out << answer << '\n';
    }
}
