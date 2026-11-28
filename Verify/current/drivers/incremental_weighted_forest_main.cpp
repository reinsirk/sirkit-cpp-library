// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int n{}, x{}, q{};
    verify_in >> n >> x >> q;
    sirkit::IncrementalWeightedForest forest(n);
    for (int i = 0; i < q; ++i)
    {
        int type{}, v{};
        verify_in >> type >> v;
        if (type == 1)
        {
            std::uint64_t weight{};
            verify_in >> weight;
            forest.link(x, v, weight);
        }
        else if (type == 2)
        {
            int u{};
            verify_in >> u;
            if (const auto distance = forest.distance(u, v))
            {
                verify_out << *distance << '\n';
                x = static_cast<int>((x + *distance) % n);
            }
            else
            {
                verify_out << -1 << '\n';
            }
        }
        else if (type == 3)
        {
            verify_out << forest.diameter(v).distance << '\n';
        }
        else
        {
            x = (x + v) % n;
        }
    }
}
