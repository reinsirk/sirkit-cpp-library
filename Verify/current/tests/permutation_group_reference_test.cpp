// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Permutation_Group.cpp"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <utility>
#include <vector>

using Permutation = sirkit::PermutationGroup::Permutation;

static Permutation identity(int n)
{
    Permutation p(n);
    std::iota(p.begin(), p.end(), 0);
    return p;
}

static std::set<Permutation> enumerate(int n, const std::vector<Permutation> &generators)
{
    std::set<Permutation> found{identity(n)};
    std::vector<Permutation> pending{identity(n)};
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        for (const auto &g : generators)
        {
            Permutation product(n);
            for (int j = 0; j < n; ++j)
            {
                product[j] = g[pending[i][j]];
            }
            if (found.insert(product).second)
            {
                pending.push_back(std::move(product));
            }
        }
    }
    return found;
}

static void check_small(int n, const std::vector<Permutation> &generators)
{
    const sirkit::PermutationGroup group(n, generators);
    const auto expected = enumerate(n, generators);
    assert(group.degree() == n);
    assert(group.order_mod(1) == 0);
    assert(group.order_mod(17) == expected.size() % 17);
    assert(group.order_mod(UINT32_MAX) == expected.size());
    auto current = identity(n);
    do
    {
        assert(group.contains(current) == expected.contains(current));
    } while (std::next_permutation(current.begin(), current.end()));

    const auto indices = group.stabilizer_indices();
    for (int base = 0; base < n; ++base)
    {
        const auto fixed = [&](const Permutation &p)
        {
            return std::equal(p.begin(), p.begin() + base, current.begin());
        };
        std::set<int> images;
        for (const auto &p : expected)
        {
            if (fixed(p))
            {
                images.insert(p[base]);
            }
        }
        assert(indices[base] == static_cast<int>(images.size()));
    }
}

template <class Func>
static void check_invalid(const Func &func)
{
    bool thrown = false;
    try
    {
        func();
    }
    catch (const std::invalid_argument &)
    {
        thrown = true;
    }
    assert(thrown);
}

int main()
{
    check_small(0, {{}});
    check_small(3, {});
    check_small(4, {{1, 2, 3, 0}, {0, 3, 2, 1}});
    check_small(5, {{1, 0, 3, 2, 4}});
    check_small(5, {{1, 0, 2, 3, 4}, {0, 1, 3, 2, 4}});
    std::mt19937 rng(93428);
    for (int n = 1; n <= 7; ++n)
    {
        for (int trial = 0; trial < 30; ++trial)
        {
            std::vector<Permutation> generators(static_cast<std::size_t>(rng() % 5), identity(n));
            for (auto &generator : generators)
            {
                std::shuffle(generator.begin(), generator.end(), rng);
            }
            check_small(n, generators);
        }
    }
    constexpr int kLargeDegree = 50;
    std::vector<Permutation> generators;
    for (int i = 1; i + 1 < kLargeDegree; ++i)
    {
        auto three_cycle = identity(kLargeDegree);
        three_cycle[0] = i;
        three_cycle[i] = i + 1;
        three_cycle[i + 1] = 0;
        generators.push_back(std::move(three_cycle));
    }
    const sirkit::PermutationGroup alternating(kLargeDegree, generators);
    assert(alternating.contains(generators.back()));
    auto transposition = identity(kLargeDegree);
    std::swap(transposition[0], transposition[1]);
    assert(!alternating.contains(transposition));
    constexpr std::uint32_t kMod = 998'244'353;
    std::uint64_t half_factorial = 1;
    for (int factor = 3; factor <= kLargeDegree; ++factor)
    {
        half_factorial = half_factorial * factor % kMod;
    }
    assert(alternating.order_mod(kMod) == half_factorial);
    generators.push_back(std::move(transposition));
    const sirkit::PermutationGroup symmetric(kLargeDegree, generators);
    assert(symmetric.order_mod(kMod) == 2 * half_factorial % kMod);
    assert(symmetric.contains(generators.back()));

    check_invalid([]
                  {
                      (void)sirkit::PermutationGroup(-1, {});
                  });
    check_invalid([]
                  {
                      (void)sirkit::PermutationGroup(2, std::vector<Permutation>{{-1, 0}});
                  });
    check_invalid([]
                  {
                      (void)sirkit::PermutationGroup(2, std::vector<Permutation>{{0, 2}});
                  });
    check_invalid([]
                  {
                      (void)sirkit::PermutationGroup(2, std::vector<Permutation>{{0, 0}});
                  });
    check_invalid([]
                  {
                      (void)sirkit::PermutationGroup(2, std::vector<Permutation>{{0}});
                  });
    check_invalid([&]
                  {
                      (void)symmetric.contains(Permutation{});
                  });
    check_invalid([&]
                  {
                      (void)symmetric.order_mod(0);
                  });
}
