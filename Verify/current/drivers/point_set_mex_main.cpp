// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::PointSetMex index(std::move(values));
    for (std::size_t k = 0; k < q; ++k)
    {
        std::size_t position{};
        int value{};
        verify_in >> position >> value;
        index.set(position - 1, value);
        verify_out << index.mex() << '\n';
    }
}
