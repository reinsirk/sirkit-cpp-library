// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_GLOBAL_MIN_CUT_DEFINED
#define SIRKIT_GLOBAL_MIN_CUT_REFERENCE_SOURCE "../../../Graph/Global_Min_Cut.cpp"
#include SIRKIT_GLOBAL_MIN_CUT_REFERENCE_SOURCE
#undef SIRKIT_GLOBAL_MIN_CUT_REFERENCE_SOURCE
#endif
#include <array>
#include <cassert>
#include <random>

namespace
{
using Edge = sirkit::GlobalMinCutEdge;

void check(int n, const std::vector<Edge> &edges)
{
    const auto got = sirkit::global_min_cut(n, edges);
    if (n < 2)
    {
        assert(!got);
        return;
    }
    const auto limit = 1U << n;
    const auto weight = [&](unsigned mask)
    {
        std::uint64_t answer = 0;
        for (const auto [u, v, w] : edges)
        {
            if (((mask >> u) & 1U) != ((mask >> v) & 1U))
            {
                answer += w;
            }
        }
        return answer;
    };
    auto expected = std::numeric_limits<std::uint64_t>::max();
    for (unsigned mask = 1; mask + 1 < limit; ++mask)
    {
        expected = std::min(expected, weight(mask));
    }
    unsigned mask = 0;
    assert(got && got->weight == expected);
    for (const auto v : got->side)
    {
        assert(v >= 0 && v < n && ((mask >> v) & 1U) == 0);
        mask |= 1U << v;
    }
    assert(mask > 0 && mask + 1 < limit && weight(mask) == expected);
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
    using sirkit::global_min_cut;
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    check(0, {});
    check(1, {{0, 0, kMax}, {0, 0, kMax}});
    check(2, {{0, 1, kMax}, {0, 1, 0}});
    check(2, {});
    std::mt19937 random(391);
    for (int n = 2; n <= 9; ++n)
    {
        for (int trial = 0; trial < 150; ++trial)
        {
            std::vector<Edge> edges(random() % 35);
            for (auto &[u, v, weight] : edges)
            {
                u = static_cast<int>(random() % n);
                v = static_cast<int>(random() % n);
                weight = random() % 100;
            }
            check(n, edges);
        }
    }
    expect_error<std::overflow_error>([]
                                      {
                                          global_min_cut(2, std::array<Edge, 2>{{{0, 1, kMax}, {1, 0, 1}}});
                                      });
    expect_error<std::length_error>([]
                                    {
                                        sirkit::detail::GlobalCutMatrixSize(5, 24);
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([]
                                        {
                                            global_min_cut(-1, {});
                                        });
    for (const auto invalid : {Edge{-1, 0, 1}, Edge{2, 0, 1}, Edge{0, -1, 1}, Edge{0, 2, 1}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            global_min_cut(2, std::array{invalid});
                                        });
    }
#endif
}
