// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <numeric>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    std::vector<int> initial(n);
    std::iota(initial.begin(), initial.end(), 1);
    sirkit::IntervalMap<int> colors(initial);
    std::vector<std::size_t> counts(n + 1, 1);
    for (std::size_t i = 0; i < q; ++i)
    {
        int type{};
        verify_in >> type;
        if (type == 1)
        {
            std::size_t x{};
            int color{};
            verify_in >> x >> color;
            const auto run = colors.run_at(x - 1);
            const auto length = run.right - run.left;
            colors.assign(run.left, run.right, color);
            counts[run.value] -= length;
            counts[color] += length;
        }
        else
        {
            int color{};
            verify_in >> color;
            verify_out << counts[color] << '\n';
        }
    }
}
