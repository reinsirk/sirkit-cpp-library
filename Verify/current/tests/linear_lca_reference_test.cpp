// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Linear_LCA.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
void check_rmq(const std::vector<int> &depths)
{
    sirkit::detail::LinearLcaRmq rmq;
    rmq.build(depths);
    for (int l = 0; l < static_cast<int>(depths.size()); ++l)
    {
        int smallest = l;
        for (int r = l; r < static_cast<int>(depths.size()); ++r)
        {
            if (depths[r] < depths[smallest])
            {
                smallest = r;
            }
            assert(rmq.index(l, r) == smallest);
            assert(rmq.depth(smallest) == depths[smallest]);
        }
    }
}

int ancestor(int u, int v, const std::vector<int> &parents, const std::vector<int> &depths)
{
    while (depths[u] > depths[v])
    {
        u = parents[u];
    }
    while (depths[v] > depths[u])
    {
        v = parents[v];
    }
    while (u != v)
    {
        u = parents[u];
        v = parents[v];
    }
    return u;
}

void check(const std::vector<int> &parents)
{
    const sirkit::LinearLCA lca(parents);
    const int n = static_cast<int>(parents.size());
    assert(lca.size() == n);
    std::vector<int> depth(n);
    for (int u = 0; u < n; ++u)
    {
        for (int v = parents[u]; v != -1; v = parents[v])
        {
            ++depth[u];
        }
        assert(lca.depth_of(u) == depth[u]);
    }
    for (int u = 0; u < n; ++u)
    {
        for (int v = 0; v < n; ++v)
        {
            const int expected = ancestor(u, v, parents, depth);
            assert(lca(u, v) == expected);
            assert(lca.get_distance(u, v) == (expected == -1 ? -1 : depth[u] + depth[v] - 2 * depth[expected]));
        }
    }
}

template <class Error, class Action>
void expect_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check({});
    check({-1});
    check({3, -1, 1, 1, 2, 2, -1, 6});
    check({-1, -1, -1, -1});
    check({-1, 0, 0, 0, 0, 0, 0});
    for (const int length : {1, 2, 3, 7, 15, 16, 17, 30, 35, 63, 64, 65, 129, 257, 513})
    {
        std::vector<int> repeated(length);
        for (int i = 0; i < length; ++i)
        {
            repeated[i] = i % 2;
        }
        check_rmq(repeated);
    }

    std::mt19937 random(739841234);
    for (int trial = 0; trial < 52; ++trial)
    {
        const int length = 2 + static_cast<int>(random() % 337);
        std::vector<int> depths(length, -80);
        for (int i = 1; i < length; ++i)
        {
            depths[i] = depths[i - 1] + ((random() % 2) ? 1 : -1);
        }
        check_rmq(depths);
    }
    for (int trial = 0; trial < 106; ++trial)
    {
        const int length = 1 + static_cast<int>(random() % 104);
        std::vector<int> order(length), parents(length, -1);
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), random);
        for (int v = 1; v < length; ++v)
        {
            if (random() % 6 != 0)
            {
                parents[order[v]] = order[random() % v];
            }
        }
        check(parents);
    }

    constexpr int kChain = 100003;
    std::vector<int> long_chain(kChain);
    std::iota(long_chain.begin(), long_chain.end(), -1);
    const sirkit::LinearLCA chain(long_chain);
    for (int v = 0; v < kChain; v += 313)
    {
        assert(chain.get(v, kChain - 1) == v);
        assert(chain.get(kChain - 1, v) == v);
        assert(chain.get_distance(v, kChain - 1) == kChain - 1 - v);
    }
    const sirkit::LinearLCA copy(chain);
    assert(copy.get(kChain - 1, kChain - 2) == kChain - 2);

    constexpr auto kAllowed = std::numeric_limits<int>::max() / 2;
    assert(sirkit::detail::checked_linear_lca_count(kAllowed) == kAllowed);
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::detail::checked_linear_lca_count(static_cast<std::size_t>(kAllowed) + 1);
                                    });
    for (const auto bad : {-2, 1})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array parent{bad};
                                            (void)sirkit::LinearLCA(parent);
                                        });
    }
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array cycle{-1, 2, 1};
                                            (void)sirkit::LinearLCA(cycle);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array self{0};
                                            (void)sirkit::LinearLCA(self);
                                        });
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)chain.get(-1, 0);
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)chain.get(0, kChain);
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)chain.depth_of(kChain);
                                    });
}
