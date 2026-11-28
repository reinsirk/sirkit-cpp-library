// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    int n = 0;
    verify_in >> n;
    std::vector<int> values(n);
    for (int &value : values)
    {
        verify_in >> value;
    }
    std::vector<std::vector<int>> edges(n);
    for (int i = 1; i < n; ++i)
    {
        int first = 0, second = 0;
        verify_in >> first >> second;
        --first;
        --second;
        edges[first].push_back(second);
        edges[second].push_back(first);
    }

    struct Visit
    {
        int vertex;
        int parent;
        bool leave;
    };
    std::vector<Visit> stack;
    stack.reserve(n);
    stack.push_back({0, -1, false});
    sirkit::RollbackLIS<int> lis;
    lis.reserve(n);
    std::vector<std::size_t> answers(n);
    while (!stack.empty())
    {
        const auto [vertex, parent, leave] = stack.back();
        stack.pop_back();
        if (leave)
        {
            lis.pop_back();
            continue;
        }

        lis.push_back(values[vertex]);
        answers[vertex] = lis.length();
        stack.push_back({vertex, parent, true});
        for (int adjacent : edges[vertex])
        {
            if (adjacent != parent)
            {
                stack.push_back({adjacent, vertex, false});
            }
        }
    }
    for (auto answer : answers)
    {
        verify_out << answer << '\n';
    }
}
