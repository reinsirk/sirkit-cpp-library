// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

int main()
{
    constexpr int kAlphabet = 52;
    constexpr int kVertices = kAlphabet * kAlphabet * kAlphabet;
    const auto id = [](std::string_view triple)
    {
        int encoded = 0;
        for (const char ch : triple)
        {
            const int value = ch >= 'a' ? 26 + ch - 'a' : ch - 'A';
            encoded = encoded * kAlphabet + value;
        }
        return encoded;
    };

    int n{};
    verify_in >> n;
    std::vector<std::pair<int, int>> edges;
    edges.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        std::string word;
        verify_in >> word;
        const std::string_view view(word);
        edges.emplace_back(id(view.substr(0, 3)), id(view.substr(view.size() - 3)));
    }
    using Outcome = sirkit::DirectedGameOutcome;
    const auto positions = sirkit::solve_directed_game(kVertices, edges);
    for (const auto &edge : edges)
    {
        switch (positions[edge.second].outcome)
        {
        case Outcome::lose:
            verify_out << "Takahashi\n";
            break;
        case Outcome::win:
            verify_out << "Aoki\n";
            break;
        case Outcome::draw:
            verify_out << "Draw\n";
            break;
        }
    }
}
