// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Path_Matching.cpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Score = std::int64_t;

void check(std::vector<Score> weights)
{
    const auto n = weights.size();
    const sirkit::PathMatching<Score> matching(weights);
    const auto bound = (n + 1) / 2;
    assert(matching.max_cardinality() == bound);
    assert(matching.values().size() == bound + 1);
    std::vector<Score> brute(bound + 1, std::numeric_limits<Score>::min());
    for (unsigned chosen = 0; chosen < (1U << n); ++chosen)
    {
        if (chosen & (chosen >> 1))
        {
            continue;
        }
        Score score = 0;
        for (std::size_t i = 0; i < n; ++i)
        {
            if ((chosen >> i) & 1U)
            {
                score += weights[i];
            }
        }
        const auto k = std::popcount(chosen);
        brute[k] = std::max(brute[k], score);
    }
    for (std::size_t k = 0; k <= bound; ++k)
    {
        assert(matching.best(k) == brute[k]);
        assert(matching.values()[k] == brute[k]);
        assert(sirkit::maximum_path_matching_weight<Score>(weights, k) == brute[k]);
        const auto edges = matching.edge_indices(k);
        assert(edges.size() == k);
        Score witness = 0;
        for (std::size_t i = 0; i < k; ++i)
        {
            assert(edges[i] < n);
            assert(i == 0 || edges[i - 1] + 1 < edges[i]);
            witness += weights[edges[i]];
        }
        assert(witness == brute[k]);
    }
}

template <class Action>
void expect_out_of_range(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check({});
    check({-10});
    check({1000000000000, -1000000000000, 1000000000001});
    check({-2, -3, -2});
    check({1, 9, 2, 9, 1});
    check({2, 2, 2, 2, 2, 2, 2, 2});
    check({10, -9, -5, -6, -4, -1, 10, 10, 9, 30, 30, 20});
    const std::array small{1, 5, 1};
    const sirkit::PathMatching<int> matching(small);
    expect_out_of_range([&]
                        {
                            (void)matching.best(3);
                        });
    expect_out_of_range([&]
                        {
                            (void)matching.edge_indices(3);
                        });
    expect_out_of_range([&]
                        {
                            (void)sirkit::maximum_path_matching_weight<int>(small, 3);
                        });

    for (int pattern = 0; pattern < 6561; ++pattern)
    {
        std::vector<Score> weights(8);
        int digits = pattern;
        for (auto &weight : weights)
        {
            weight = digits % 3 - 1;
            digits /= 3;
        }
        check(weights);
    }
    std::mt19937 random(77202616);
    for (int trial = 0; trial < 600; ++trial)
    {
        std::vector<Score> weights(random() % 13 + 1);
        for (auto &weight : weights)
        {
            weight = static_cast<int>(random() % 61) - 30;
        }
        check(weights);
    }

    constexpr int kSize = 199999;
    std::vector<Score> weights(kSize);
    for (int i = 0; i < kSize; ++i)
    {
        weights[i] = i % 2 == 0 ? 1 : 1000000000;
    }
    const sirkit::PathMatching<Score> deep(weights);
    assert(deep.best(kSize / 2) == (kSize / 2) * Score{1000000000});
    assert(deep.best(kSize / 2 + 1) == kSize / 2 + 1);
    const auto witness = deep.edge_indices(kSize / 2 + 1);
    assert(witness.size() == kSize / 2 + 1);
    for (std::size_t i = 0; i < witness.size(); ++i)
    {
        assert(witness[i] == i * 2);
    }
}
