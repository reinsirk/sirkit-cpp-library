// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Josephus.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

int main()
{
    const sirkit::fenwick_tree<std::uint32_t> weights(std::vector<std::uint32_t>{1, 0, 2});
    assert(weights.lower_bound(2) == 2);

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    for (int count = 1; count <= 108; ++count)
    {
        for (int case_id = 1; case_id <= 91; ++case_id)
        {
            const auto step = case_id <= 80 ? static_cast<std::uint64_t>(case_id) : kMax - static_cast<std::uint64_t>(case_id - 81);
            std::vector<int> alive(count);
            std::iota(alive.begin(), alive.end(), 0);
            std::vector<int> reference;
            std::uint64_t position = 0;
            while (!alive.empty())
            {
                position = (position + (step - 1) % alive.size()) % alive.size();
                reference.push_back(alive[position]);
                alive.erase(alive.begin() + static_cast<std::ptrdiff_t>(position));
            }
            assert(sirkit::josephus_order(count, step) == reference);
            assert(sirkit::josephus_survivor(count, step) == static_cast<std::uint64_t>(reference.back()));
        }
    }

    assert(sirkit::josephus_order(0, kMax).empty());
    assert(sirkit::josephus_survivor(kMax, 1) == kMax - 1);
    assert(sirkit::josephus_survivor(kMax, 2) == kMax - 1);
    // 不等式 p > (step-1)*n に達するまで p = ceil(p*step/(step-1)) を繰り返し、n*step-p を計算した値。
    assert(sirkit::josephus_survivor(kMax, 3) == 17307128502216671608ULL);
    assert(sirkit::josephus_survivor(kMax, 7) == 6782408088425766382ULL);
    assert(sirkit::josephus_survivor(kMax, 42) == 15757058196093042727ULL);
    for (const std::array<std::uint64_t, 2> args : {std::array<std::uint64_t, 2>{0, 4}, {1, 0}})
    {
        bool thrown = false;
        try
        {
            (void)sirkit::josephus_survivor(args[0], args[1]);
        }
        catch (const std::invalid_argument &)
        {
            thrown = true;
        }
        assert(thrown);
    }
    for (const std::array<int, 2> args : {std::array<int, 2>{-1, 2}, {1, 0}})
    {
        bool thrown = false;
        try
        {
            (void)sirkit::josephus_order(args[0], static_cast<std::uint64_t>(args[1]));
        }
        catch (const std::invalid_argument &)
        {
            thrown = true;
        }
        assert(thrown);
    }
}
