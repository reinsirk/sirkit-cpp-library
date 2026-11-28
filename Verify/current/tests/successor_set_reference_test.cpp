// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Successor_Set.cpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    sirkit::SuccessorSet empty;
    assert(empty.size() == 0 && empty.remaining() == 0);
    assert(!empty.next(0) && !empty.take_cyclic(0) && !empty.erase(0));

    bool invalid = false, oversized = false;
    try
    {
        (void)empty.next(-1);
    }
    catch (const std::out_of_range &)
    {
        invalid = true;
    }
    try
    {
        (void)empty.erase(1);
    }
    catch (const std::out_of_range &)
    {
        oversized = true;
    }
    assert(invalid && oversized);
    bool length_error = false;
    try
    {
        (void)sirkit::SuccessorSet(std::numeric_limits<int>::max());
    }
    catch (const std::length_error &)
    {
        length_error = true;
    }
    assert(length_error);

    std::mt19937 random(536918);
    for (int trial = 0; trial < 420; ++trial)
    {
        const int n = static_cast<int>(random() % 81);
        sirkit::SuccessorSet values(n);
        std::vector<bool> free(n, true);
        const auto next = [&](int start) -> std::optional<int>
        {
            const auto it = std::find(free.begin() + start, free.end(), true);
            if (it == free.end())
            {
                return std::nullopt;
            }
            return static_cast<int>(it - free.begin());
        };
        for (int step = 0; step < 140; ++step)
        {
            const int index = static_cast<int>(random() % (n + 1));
            if (step % 4 == 0)
            {
                assert(values.erase(index) == (index < n && free[index]));
                if (index < n)
                {
                    free[index] = false;
                }
            }
            else
            {
                const bool cyclic = step % 4 == 1;
                auto expected = next(index);
                assert(values.next(index) == expected);
                if (cyclic && !expected)
                {
                    expected = next(0);
                }
                assert((cyclic ? values.take_cyclic(index) : values.take(index)) == expected);
                if (expected)
                {
                    free[*expected] = false;
                }
            }
            assert(values.remaining() == std::count(free.begin(), free.end(), true));
        }
    }

    std::vector<int> preferences{2, 1, 1, 3, 0};
    assert(sirkit::parking_slots(preferences) == std::vector<int>({2, 1, 3, 4, 0}));
    assert(sirkit::parking_slots({}).value().empty());
    for (const auto &bad : {std::vector<int>{1, 1}, std::vector<int>{-1}, std::vector<int>{1}})
    {
        assert(!sirkit::parking_slots(bad));
    }
}
