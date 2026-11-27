// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Monge_Optimization.cpp"
#include "../../../Algorithm/Concave_Min_Plus_Convolution.cpp"
#include <array>
#include <cassert>
#include <deque>
#include <random>

namespace
{
constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
constexpr auto kMin = std::numeric_limits<std::int64_t>::min();

void check(std::span<const std::int64_t> a, std::span<const std::int64_t> b)
{
    const auto got = sirkit::min_plus_convolution_concave_arbitrary(a, b);
    if (a.empty() || b.empty())
    {
        assert(got.empty());
        return;
    }
    std::vector<std::int64_t> expected(a.size() + b.size() - 1, kMax);
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; j < b.size(); ++j)
        {
            expected[i + j] = std::min(expected[i + j], a[i] + b[j]);
        }
    }
    assert(got == expected);
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
    using sirkit::min_plus_convolution_concave_arbitrary;
    check({}, {});
    check(std::array<std::int64_t, 1>{0}, {});
    check(std::array<std::int64_t, 3>{kMin, 0, kMax}, std::array<std::int64_t, 2>{0, 0});
    check(std::array<std::int64_t, 3>{kMax, 0, kMin}, std::array<std::int64_t, 2>{0, 0});
    check(std::array<std::int64_t, 3>{0, kMax, 0}, std::array<std::int64_t, 2>{0, 0});
    std::mt19937 random(29481);
    for (int trial = 0; trial < 1700; ++trial)
    {
        std::vector<std::int64_t> a(random() % 80), b(random() % 85);
        int slope = 20;
        int value = -9;
        for (auto &x : a)
        {
            x = value;
            value += slope;
            slope -= static_cast<int>(random() % 4);
        }
        for (auto &x : b)
        {
            x = static_cast<int>(random() % 2000) - 1000;
        }
        check(a, b);
    }
    // a[i]=i のとき、各出力をb[j]-jのスライド最小値と照合できる。
    constexpr std::size_t kSize = 20'000;
    std::vector<std::int64_t> a(kSize), b(kSize);
    std::iota(a.begin(), a.end(), std::int64_t{0});
    for (auto &x : b)
    {
        x = static_cast<int>(random() % 2000);
    }
    const auto actual = min_plus_convolution_concave_arbitrary(a, b);
    std::deque<std::size_t> candidates;
    const auto value_at = [&](std::size_t j)
    {
        return b[j] - static_cast<std::int64_t>(j);
    };
    for (std::size_t k = 0; k < actual.size(); ++k)
    {
        if (k < b.size())
        {
            while (!candidates.empty() && value_at(candidates.back()) >= value_at(k))
            {
                candidates.pop_back();
            }
            candidates.push_back(k);
        }
        while (candidates.front() + a.size() <= k)
        {
            candidates.pop_front();
        }
        assert(actual[k] == static_cast<std::int64_t>(k) + value_at(candidates.front()));
    }
    expect_error<std::overflow_error>([]
                                      {
                                          min_plus_convolution_concave_arbitrary(std::array<std::int64_t, 2>{0, kMax}, std::array<std::int64_t, 1>{1});
                                      });
    expect_error<std::overflow_error>([]
                                      {
                                          min_plus_convolution_concave_arbitrary(std::array<std::int64_t, 1>{kMin}, std::array<std::int64_t, 1>{-1});
                                      });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([]
                                        {
                                            min_plus_convolution_concave_arbitrary(std::array<std::int64_t, 3>{0, kMin, 0}, std::array<std::int64_t, 1>{0});
                                        });
#endif
}
