// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Majority.cpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

int main()
{
    using sirkit::range_majority_queries;
    using sirkit::RangeMajorityOperation;
    using sirkit::RangeMajorityQuery;
    using sirkit::RangeMajoritySet;

    const auto brute = [](auto sequence, const auto &operations)
    {
        std::vector<std::optional<typename decltype(sequence)::value_type>> answers;
        for (const auto &op : operations)
        {
            if (const auto *set = std::get_if<RangeMajoritySet<typename decltype(sequence)::value_type>>(&op))
            {
                sequence[set->position] = set->value;
                continue;
            }
            const auto [left, right] = std::get<RangeMajorityQuery>(op);
            std::map<typename decltype(sequence)::value_type, std::size_t> counts;
            for (std::size_t i = left; i < right; ++i)
            {
                ++counts[sequence[i]];
            }
            auto &answer = answers.emplace_back();
            for (const auto &[value, count] : counts)
            {
                if (count > (right - left) / 2)
                {
                    answer = value;
                }
            }
        }
        return answers;
    };

    const std::vector<int> empty;
    {
        const RangeMajorityOperation<int> operations[]{RangeMajorityQuery{0, 0}, RangeMajorityQuery{0, 0}};
        const auto answers = range_majority_queries(empty, operations);
        assert(answers.size() == 2 && !answers[0] && !answers[1]);
        assert(range_majority_queries(empty, {}).empty());
    }
    {
        const std::vector sequence{0, 0, 1, 2, 3};
        const std::vector<RangeMajorityOperation<int>> operations{
            RangeMajorityQuery{0, 5},
            RangeMajoritySet<int>{3, 0},
            RangeMajorityQuery{0, 5},
            RangeMajoritySet<int>{3, 0},
            RangeMajorityQuery{0, 4},
            RangeMajoritySet<int>{3, 2},
            RangeMajorityQuery{0, 4},
            RangeMajoritySet<int>{3, 0},
            RangeMajorityQuery{3, 4},
            RangeMajorityQuery{5, 5},
        };
        assert((range_majority_queries(sequence, operations) ==
                std::vector<std::optional<int>>{std::nullopt, 0, 0, std::nullopt, 0, std::nullopt}));
        assert((sequence == std::vector{0, 0, 1, 2, 3}));
    }
    {
        const std::vector<std::string> sequence{"a", "b", "a"};
        const std::vector<RangeMajorityOperation<std::string>> operations{
            RangeMajorityQuery{0, 3},
            RangeMajoritySet<std::string>{2, "b"},
            RangeMajorityQuery{0, 3},
            RangeMajorityQuery{0, 2},
            RangeMajoritySet<std::string>{0, "c"},
            RangeMajorityQuery{0, 1},
        };
        assert(range_majority_queries(sequence, operations) == brute(sequence, operations));
    }

    std::mt19937 random(276139);
    for (const std::size_t alphabet : {1, 2, 3, 8})
    {
        for (std::size_t size = 1; size <= 20; ++size)
        {
            for (int trial = 0; trial < 25; ++trial)
            {
                std::vector<std::int64_t> sequence(size);
                for (auto &value : sequence)
                {
                    value = static_cast<std::int64_t>(random() % alphabet) - 4;
                }
                std::vector<RangeMajorityOperation<std::int64_t>> operations;
                for (int step = 0; step < 65; ++step)
                {
                    if (random() % 2 == 0)
                    {
                        operations.emplace_back(RangeMajoritySet<std::int64_t>{
                            random() % size, static_cast<std::int64_t>(random() % alphabet) - 4});
                    }
                    else
                    {
                        auto left = random() % (size + 1), right = random() % (size + 1);
                        if (left > right)
                        {
                            std::swap(left, right);
                        }
                        operations.emplace_back(RangeMajorityQuery{left, right});
                    }
                }
                assert(range_majority_queries(sequence, operations) == brute(sequence, operations));
            }
        }
    }

    sirkit::detail::CheckRangeMajorityCapacity(3, 2, 5, 6);
    for (const auto &[n, updates] : {std::pair{4, 0}, std::pair{3, 3}})
    {
        try
        {
            sirkit::detail::CheckRangeMajorityCapacity(n, updates, 5, 6);
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<int> sequence{1};
    for (const RangeMajorityOperation<int> operation :
         {RangeMajorityOperation<int>{RangeMajoritySet<int>{1, 2}},
          RangeMajorityOperation<int>{RangeMajorityQuery{1, 0}},
          RangeMajorityOperation<int>{RangeMajorityQuery{0, 2}}})
    {
        try
        {
            (void)range_majority_queries(sequence, {&operation, 1});
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
