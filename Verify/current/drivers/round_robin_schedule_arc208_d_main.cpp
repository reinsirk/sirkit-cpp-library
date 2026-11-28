// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace
{
void solve()
{
    int n{};
    verify_in >> n;
    const auto size = static_cast<std::size_t>(n);
    std::vector<int> requested(size), fixed;
    for (int &partner : requested)
    {
        verify_in >> partner;
        --partner;
    }
    for (int i = 0; i < n; ++i)
    {
        const int partner = requested[static_cast<std::size_t>(i)];
        if (requested[static_cast<std::size_t>(partner)] != i)
        {
            verify_out << "No\n";
            return;
        }
        if (partner == i)
        {
            fixed.push_back(i);
        }
    }
    if (n % 2 != 0 && fixed.size() != 1)
    {
        verify_out << "No\n";
        return;
    }
    verify_out << "Yes\n";
    if (n == 1)
    {
        verify_out << "1\n";
        return;
    }

    const sirkit::RoundRobinSchedule schedule(n);
    std::vector<std::pair<int, int>> pairs;
    for (int i = 0; i < n; ++i)
    {
        const int j = schedule.partner(0, i);
        if (i < j)
        {
            pairs.emplace_back(i, j);
        }
    }

    std::size_t chosen = 0;
    std::vector<int> renumbered(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        const auto j = static_cast<std::size_t>(requested[i]);
        if (i < j)
        {
            const auto [a, b] = pairs[chosen++];
            renumbered[i] = a;
            renumbered[j] = b;
        }
    }
    if (n % 2 != 0)
    {
        renumbered[static_cast<std::size_t>(fixed.front())] = 0;
    }
    else
    {
        for (std::size_t i = 0; i < fixed.size(); i += 2)
        {
            const auto [a, b] = pairs[chosen++];
            renumbered[static_cast<std::size_t>(fixed[i])] = a;
            renumbered[static_cast<std::size_t>(fixed[i + 1])] = b;
        }
    }

    for (std::size_t u = 0; u < size; ++u)
    {
        for (std::size_t v = 0; v < size; ++v)
        {
            int value{};
            if (u == v)
            {
                value = n % 2 != 0 ? renumbered[u] : n - 1;
                if (requested[u] == static_cast<int>(u))
                {
                    value = 0;
                }
            }
            else
            {
                value = schedule.round_of(renumbered[u], renumbered[v]);
                if (value == 0 && requested[u] == static_cast<int>(u))
                {
                    value = n - 1;
                }
            }
            verify_out << value + 1 << (v + 1 == size ? '\n' : ' ');
        }
    }
}
} // namespace

int main()
{
    int cases{};
    verify_in >> cases;
    for (int i = 0; i < cases; ++i)
    {
        solve();
    }
}
