// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <string>
#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::array<std::vector<int>, 26> by_first;
    std::vector<int> last(n);
    for (int i = 0; i < n; ++i)
    {
        std::string name;
        verify_in >> name;
        by_first[name[0] - 'A'].push_back(i);
        last[i] = name[1] - 'A';
    }
    std::vector<std::pair<int, int>> edges;
    for (int i = 0; i < n; ++i)
    {
        for (const int next : by_first[last[i]])
        {
            edges.emplace_back(i, next);
        }
    }
    verify_out << sirkit::minimum_reachability_chain_cover(n, edges).size() << '\n';
}
