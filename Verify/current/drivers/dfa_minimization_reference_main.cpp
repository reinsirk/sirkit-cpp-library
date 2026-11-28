// SPDX-License-Identifier: CC0-1.0
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n = 0, alphabet = 0, start = 0, queries = 0;
    std::cin >> n >> alphabet >> start >> queries;
    std::vector<std::uint8_t> accepting(n);
    std::vector<std::size_t> transitions(n * alphabet);
    for (std::size_t state = 0; state < n; ++state)
    {
        int final = 0;
        std::cin >> final;
        accepting[state] = static_cast<std::uint8_t>(final);
        for (std::size_t c = 0; c < alphabet; ++c)
        {
            std::cin >> transitions[state * alphabet + c];
        }
    }
    const auto minimum = minimize_dfa(alphabet, transitions, accepting, start);
    std::cout << minimum.size() << '\n';
    const auto state_map = minimum.state_map();
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::size_t first = 0, second = 0;
        std::cin >> first >> second;
        std::cout << (state_map[first] == state_map[second] && state_map[first] != DfaMinimization::kUnreachable) << '\n';
    }
}
