// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_STERN_BROCOT_TREE_TEST_SOURCE
#define SIRKIT_STERN_BROCOT_TREE_TEST_SOURCE "../../../Math/Stern_Brocot_Tree.cpp"
#endif
#include SIRKIT_STERN_BROCOT_TREE_TEST_SOURCE
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
using Tree = sirkit::SternBrocotTree;
using Fraction = Tree::Fraction;

Fraction Mediant(Fraction a, Fraction b)
{
    return {a.first + b.first, a.second + b.second};
}

std::pair<Fraction, Fraction> Follow(const std::string &path)
{
    Fraction lower{0, 1}, upper{1, 0};
    for (char direction : path)
    {
        if (direction == 'L')
        {
            upper = Mediant(lower, upper);
        }
        else
        {
            lower = Mediant(lower, upper);
        }
    }
    return {lower, upper};
}

Fraction Value(const std::string &path)
{
    const auto [lower, upper] = Follow(path);
    return Mediant(lower, upper);
}

std::pair<Fraction, Fraction> EnumerateNeighbors(Fraction target, unsigned numerator_limit, unsigned denominator_limit)
{
    Fraction lower{0, 1}, upper{1, 0};
    const auto compare = [](Fraction a, Fraction b)
    {
        return a.first * b.second < a.second * b.first;
    };
    for (unsigned q = 1; q <= denominator_limit; ++q)
    {
        for (unsigned p = 1; p <= numerator_limit; ++p)
        {
            if (std::gcd(p, q) != 1)
            {
                continue;
            }
            const Fraction fraction{p, q};
            if (!compare(target, fraction) && compare(lower, fraction))
            {
                lower = fraction;
            }
            if (!compare(fraction, target) && compare(fraction, upper))
            {
                upper = fraction;
            }
        }
    }
    return {lower, upper};
}

template <class Exception, class Operation>
bool Throws(Operation operation)
{
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        return true;
    }
    return false;
}
} // namespace

int main()
{
    std::vector<std::string> paths{""};
    for (std::size_t i = 0; i < 511; ++i)
    {
        const std::string path = paths[i];
        paths.push_back(path + 'L');
        paths.push_back(path + 'R');
    }
    for (const auto &path : paths)
    {
        const Fraction fraction = Value(path);
        const auto runs = Tree::encode(fraction);
        std::string expanded;
        for (const auto &[direction, count] : runs)
        {
            assert(expanded.empty() || expanded.back() != direction);
            assert(count > 0);
            expanded.append(count, direction);
        }
        assert(expanded == path);
        assert(Tree::decode(runs) == fraction);
        assert(Tree::range(fraction) == Follow(path));
        for (std::size_t depth = 0; depth <= path.size(); ++depth)
        {
            assert(Tree::ancestor(fraction, depth) == Value(path.substr(0, depth)));
        }
        assert(!Tree::ancestor(fraction, path.size() + 1));
        for (std::size_t i = 0; i < paths.size(); i += 23)
        {
            const auto &other = paths[i];
            std::size_t common = 0;
            while (common < std::min(path.size(), other.size()) && path[common] == other[common])
            {
                ++common;
            }
            assert(Tree::lca(fraction, Value(other)) == Value(path.substr(0, common)));
        }
    }

    assert((Tree::decode(Tree::encode({10, 6})) == Fraction{5, 3}));
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    for (unsigned numerator_limit = 0; numerator_limit <= 15; ++numerator_limit)
    {
        for (unsigned denominator_limit = 0; denominator_limit <= 15; ++denominator_limit)
        {
            for (unsigned a = 1; a <= 19; ++a)
            {
                for (unsigned b = 1; b <= 19; ++b)
                {
                    const Fraction fraction{a, b};
                    assert(Tree::approximate(fraction, numerator_limit, denominator_limit) ==
                           EnumerateNeighbors(fraction, numerator_limit, denominator_limit));
                }
            }
        }
    }
    assert((Tree::approximate({kMax, 1}, kMax - 1, 1) ==
            std::pair{Fraction{kMax - 1, 1}, Fraction{1, 0}}));
    assert((Tree::approximate({1, kMax}, 1, kMax - 1) ==
            std::pair{Fraction{0, 1}, Fraction{1, kMax - 1}}));
    assert((Tree::approximate({kMax - 1, kMax}, kMax, kMax) ==
            std::pair{Fraction{kMax - 1, kMax}, Fraction{kMax - 1, kMax}}));
    const std::array largest{Tree::Run{'R', kMax - 1}};
    const std::array split{Tree::Run{'R', 1}, Tree::Run{'R', kMax - 2}};
    assert((Tree::decode(largest) == Fraction{kMax, 1}));
    assert(Tree::decode(split) == Tree::decode(largest));
    assert((Tree::ancestor({1, kMax}, kMax - 1) == Fraction{1, kMax}));
    assert(!Tree::ancestor({1, kMax}, kMax));
    assert(Throws<std::overflow_error>([kMax]
                                       {
                                           Tree::decode(std::array{Tree::Run{'R', kMax}});
                                       }));
    assert(Throws<std::overflow_error>([kMax]
                                       {
                                           Tree::decode(std::array{Tree::Run{'L', kMax - 1}, Tree::Run{'R', 2}});
                                       }));
#ifndef SKIP_RUNTIME_CHECKS
    assert(Throws<std::invalid_argument>([]
                                         {
                                             Tree::encode({0, 1});
                                         }));
    assert(Throws<std::invalid_argument>([]
                                         {
                                             Tree::range({1, 0});
                                         }));
    for (const auto &invalid : {Tree::Run{'X', 1}, Tree::Run{'L', 0}, Tree::Run{'R', 0}})
    {
        assert(Throws<std::invalid_argument>([&]
                                             {
                                                 Tree::decode(std::array{invalid});
                                             }));
    }
#endif
}
