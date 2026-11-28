// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    auto read = [](int edges)
    {
        std::vector<std::pair<int, int>> tree(edges);
        for (int i = 0; i < edges; ++i)
        {
            int parent{};
            verify_in >> parent;
            tree[i] = {i, parent - 1};
        }
        return tree;
    };
    const auto full = read(n);
    const auto smaller = read(n - 1);
    const auto answers = sirkit::tree_leaf_deletion_matches(n + 1, full, smaller);
    for (std::size_t i = 0; i < answers.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << answers[i] + 1;
    }
    verify_out << '\n';
}
