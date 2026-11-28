// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <utility>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int q = 0;
    verify_in >> q;
    struct Query
    {
        int type;
        std::int64_t x, constant;
    };
    std::vector<Query> queries(q);
    std::vector<std::int64_t> coordinates;
    coordinates.reserve(q);
    for (auto &[type, x, constant] : queries)
    {
        verify_in >> type;
        if (type == 1)
        {
            verify_in >> x >> constant;
            coordinates.push_back(x);
        }
    }
    WeightedL1Statistics values(coordinates);
    std::int64_t constant = 0;
    for (const auto &query : queries)
    {
        if (query.type == 1)
        {
            values.insert(query.x);
            constant += query.constant;
        }
        else
        {
            const auto median = values.minimizer_interval()->first;
            verify_out << median << ' ' << static_cast<std::int64_t>(values.absolute_deviation(median)) + constant << '\n';
        }
    }
}
