// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[from, to] : edges)
    {
        verify_in >> from >> to;
    }
    verify_out << sirkit::strong_connectivity_augmentation(n, edges).size() << '\n';
}
