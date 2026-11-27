// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Knapsack.cpp"
#include <cassert>
#include <random>

namespace
{
std::vector<std::int64_t> exhaustive(std::size_t capacity, const std::vector<sirkit::KnapsackItem> &items)
{
    std::vector<std::int64_t> answer(capacity + 1, -1);
    const auto search = [&](auto &&self, std::size_t item, std::size_t weight, std::int64_t value) -> void
    {
        if (item == items.size())
        {
            answer[weight] = std::max(answer[weight], value);
            return;
        }
        const auto current = items[item];
        for (std::size_t count = 0; count <= current.count; ++count)
        {
            const auto total_weight = weight + count * current.weight;
            if (total_weight > capacity)
            {
                break;
            }
            self(self, item + 1, total_weight, value + static_cast<std::int64_t>(count) * current.value);
        }
    };
    search(search, 0, 0, 0);
    return answer;
}

template <class Exception, class Operation>
void expect_error(Operation run)
{
    try
    {
        run();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
}

int main()
{
    using sirkit::KnapsackItem;
    using sirkit::maximize_exact_knapsack;
    using sirkit::maximize_knapsack;
    std::mt19937 random(1157);
    assert(maximize_knapsack(0, {}) == 0);
    assert(maximize_exact_knapsack(1, {}) == std::nullopt);
    for (unsigned trial = 0; trial < 2500; ++trial)
    {
        const auto capacity = random() % 40;
        std::vector<KnapsackItem> items(random() % 8);
        for (auto &item : items)
        {
            item = {random() % 11, static_cast<std::int64_t>(random() % 13), random() % 5};
        }
        const auto expected = exhaustive(capacity, items);
        assert(maximize_knapsack(capacity, items) == *std::max_element(expected.begin(), expected.end()));
        const auto exact = maximize_exact_knapsack(capacity, items);
        assert(exact.value_or(-1) == expected.back());
    }
    const std::vector<KnapsackItem> max_case(100, {1, 10'000, 100'000});
    for (const auto capacity : {30'000U, 100'000U})
    {
        assert(maximize_exact_knapsack(capacity, max_case) == 10'000LL * capacity);
    }
    const auto inf = std::numeric_limits<std::int64_t>::max();
    assert(maximize_knapsack(0, std::vector<KnapsackItem>{{0, inf}}) == inf);
    assert(maximize_knapsack(1, std::vector<KnapsackItem>{{1, inf}, {1, 1}}) == inf);
    assert(maximize_exact_knapsack(4, std::vector<KnapsackItem>{{2, 0, 2}}) == 0);
    expect_error<std::overflow_error>([&]
                                      {
                                          maximize_knapsack(2, std::vector<KnapsackItem>{{1, inf, 2}});
                                      });
    expect_error<std::overflow_error>([&]
                                      {
                                          maximize_knapsack(0, std::vector<KnapsackItem>{{0, inf, 2}});
                                      });
    expect_error<std::overflow_error>([&]
                                      {
                                          maximize_exact_knapsack(2, std::vector<KnapsackItem>{{0, inf}, {1, 1}});
                                      });
    expect_error<std::overflow_error>([&]
                                      {
                                          maximize_exact_knapsack(2, std::vector<KnapsackItem>{{0, inf}, {1, 1}, {2, 0}});
                                      });
    expect_error<std::length_error>([&]
                                    {
                                        maximize_knapsack(std::numeric_limits<std::size_t>::max(), {});
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([&]
                                        {
                                            maximize_knapsack(3, std::vector<KnapsackItem>{{2, -1}});
                                        });
#endif
}
