// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

int main()
{
    std::size_t n, target;
    verify_in >> n >> target;
    std::vector<std::size_t> difference(n);
    std::string faces(n, 'H');
    std::size_t baseline = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t front, back;
        verify_in >> front >> back;
        if (front > back)
        {
            faces[i] = 'T';
            std::swap(front, back);
        }
        baseline += front;
        difference[i] = back - front;
    }
    if (baseline <= target)
    {
        if (const auto chosen = subset_sum(difference, target - baseline))
        {
            for (auto i : *chosen)
            {
                faces[i] = faces[i] == 'H' ? 'T' : 'H';
            }
            verify_out << "Yes\n"
                       << faces << '\n';
            return 0;
        }
    }
    verify_out << "No\n";
}
