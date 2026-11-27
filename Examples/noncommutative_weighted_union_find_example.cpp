// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Noncommutative_Weighted_Union_Find.cpp"

#include <array>
#include <cassert>
#include <cstddef>

using Permutation = std::array<std::size_t, 3>;

constexpr Permutation compose(const Permutation &a, const Permutation &b)
{
    return {a[b[0]], a[b[1]], a[b[2]]};
}

constexpr Permutation identity()
{
    return {0, 1, 2};
}

constexpr Permutation inverse(const Permutation &value)
{
    Permutation result{};
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        result[value[i]] = i;
    }
    return result;
}

int main()
{
    using UF = sirkit::NoncommutativeWeightedUnionFind<Permutation, compose, identity, inverse>;
    UF uf(3);
    constexpr Permutation p{1, 0, 2}, q{0, 2, 1};
    assert(uf.constrain(0, 1, p));
    assert(uf.constrain(2, 1, inverse(q)));
    assert(uf.diff(0, 2) == compose(p, q));
    assert(uf.diff(2, 0) == compose(inverse(q), inverse(p)));
    assert(uf.unite_checked(0, 2, compose(q, p)) == UF::MergeResult::Contradiction);
}
