// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    using Point = std::pair<std::int64_t, std::int64_t>;
    const auto read_shape = [&]()
    {
        std::vector<Point> shape;
        for (int y = 0; y < n; ++y)
        {
            std::string row;
            verify_in >> row;
            for (int x = 0; x < n; ++x)
            {
                if (row[static_cast<std::size_t>(x)] == '#')
                {
                    shape.emplace_back(x, y);
                }
            }
        }
        return shape;
    };
    const auto first = read_shape();
    const auto second = read_shape();
    verify_out << (sirkit::lattice_shape_transforms(first, second).empty() ? "No" : "Yes") << '\n';
}
