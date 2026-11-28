// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{

    std::size_t n{}, m{}, p{};
    verify_in >> n >> m >> p;
    std::vector<std::int64_t> left(n), right(m);
    for (auto &value : left)
    {
        verify_in >> value;
    }
    for (auto &value : right)
    {
        verify_in >> value;
    }
    std::vector<std::pair<std::size_t, std::size_t>> forbidden(p);
    for (auto &[i, j] : forbidden)
    {
        verify_in >> i >> j;
        --i;
        --j;
    }
    verify_out << sirkit::maximum_allowed_pair(left, right, forbidden)->sum << '\n';
}
