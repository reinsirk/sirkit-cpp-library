// SPDX-License-Identifier: CC0-1.0
#include <atcoder/modint>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/wildcard_construction.hpp"

int main()
{
    std::string text, x, y;
    verify_in >> text >> x >> y;
    const auto automaton = verify_wildcard::automaton(x, y);
    std::vector<std::size_t> edges(2 * automaton.size());
    std::vector<std::uint8_t> accepting(automaton.size());
    for (std::size_t state = 0; state < automaton.size(); ++state)
    {
        accepting[state] = automaton.accepting(state);
        edges[2 * state] = automaton.transition(state, 0);
        edges[2 * state + 1] = automaton.transition(state, 1);
    }
    const auto minimal = minimize_dfa(2, edges, accepting);
    using Mint = atcoder::modint998244353;
    std::vector<Mint> current(minimal.size()), next(minimal.size());
    current[0] = 1;
    for (const auto mask : verify_wildcard::masks(text))
    {
        std::fill(next.begin(), next.end(), Mint{});
        for (std::size_t v = 0; v < minimal.size(); ++v)
        {
            for (std::size_t c = 0; c < 2; ++c)
            {
                if ((mask >> c & 1) != 0)
                {
                    next[minimal.transition(v, c)] += current[v];
                }
            }
        }
        current.swap(next);
    }
    Mint answer{};
    for (std::size_t state = 0; state < minimal.size(); ++state)
    {
        if (minimal.accepting(state))
        {
            answer += current[state];
        }
    }
    verify_out << answer.val() << '\n';
}
