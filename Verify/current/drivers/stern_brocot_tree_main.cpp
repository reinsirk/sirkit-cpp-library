// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <string>
#include <vector>

int main()
{
    using Tree = SternBrocotTree;
    int queries{};
    verify_in >> queries;
    std::vector<Tree::Run> path;
    std::string operation;
    const auto read_fraction = []
    {
        Tree::Fraction fraction{};
        verify_in >> fraction.first >> fraction.second;
        return fraction;
    };
    const auto write_fraction = [](Tree::Fraction fraction, char separator = '\n')
    {
        verify_out << fraction.first << ' ' << fraction.second << separator;
    };
    while (queries-- > 0)
    {
        verify_in >> operation;
        if (operation == "ENCODE_PATH")
        {
            const auto encoded = Tree::encode(read_fraction());
            verify_out << encoded.size();
            for (const auto &[direction, count] : encoded)
            {
                verify_out << ' ' << direction << ' ' << count;
            }
            verify_out << '\n';
        }
        else if (operation == "DECODE_PATH")
        {
            int runs{};
            verify_in >> runs;
            path.resize(runs);
            for (auto &[direction, count] : path)
            {
                verify_in >> direction >> count;
            }
            write_fraction(Tree::decode(path));
        }
        else if (operation == "LCA")
        {
            const auto a = read_fraction();
            write_fraction(Tree::lca(a, read_fraction()));
        }
        else if (operation == "ANCESTOR")
        {
            std::uint64_t depth{};
            verify_in >> depth;
            const auto ancestor = Tree::ancestor(read_fraction(), depth);
            if (ancestor)
            {
                write_fraction(*ancestor);
            }
            else
            {
                verify_out << "-1\n";
            }
        }
        else
        {
            const auto [lower, upper] = Tree::range(read_fraction());
            write_fraction(lower, ' ');
            write_fraction(upper);
        }
    }
}
