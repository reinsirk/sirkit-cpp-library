// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, width{}, q{};
    verify_in >> n >> width >> q;
    std::vector<std::int64_t> rents(n);
    std::vector<int> parking(n);
    for (auto &rent : rents)
    {
        verify_in >> rent;
    }
    for (auto &space : parking)
    {
        verify_in >> space;
    }
    const auto candidates = n - width + 1;
    std::vector<std::int64_t> values(candidates);
    std::vector<int> keys(candidates);
    for (std::size_t i = 0; i < width; ++i)
    {
        values[0] += rents[i];
        keys[0] += parking[i];
    }
    for (std::size_t i = 1; i < candidates; ++i)
    {
        values[i] = values[i - 1] + rents[i + width - 1] - rents[i - 1];
        keys[i] = keys[i - 1] + parking[i + width - 1] - parking[i - 1];
    }
    sirkit::RangeAddThresholdMin<int> windows(keys, values);
    for (std::size_t query = 0; query < q; ++query)
    {
        int type{};
        std::size_t position{};
        std::int64_t rent{};
        verify_in >> type >> position >> rent;
        if (type == 1)
        {
            --position;
            const auto left = position + 1 < width ? 0 : position + 1 - width;
            const auto right = std::min(candidates, position + 1);
            windows.add(left, right, rent - rents[position]);
            rents[position] = rent;
        }
        else if (const auto answer = windows.minimum_at_least(static_cast<int>(position)))
        {
            verify_out << *answer << '\n';
        }
        else
        {
            verify_out << "IMPOSSIBLE\n";
        }
    }
}
