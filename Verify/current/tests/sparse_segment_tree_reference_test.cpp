// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Sparse_Segment_Tree.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Key = std::uint64_t;
constexpr Key kMod = 998244353;
constexpr Key kLargest = std::numeric_limits<Key>::max();
struct Affine
{
    Key a, b;
    bool operator==(const Affine &) const noexcept = default;
};
constexpr Affine Compose(const Affine &left, const Affine &right) noexcept
{
    return {right.a * left.a % kMod, (right.a * left.b + right.b) % kMod};
}
constexpr Affine Identity() noexcept
{
    return {1, 0};
}
using Tree = sirkit::SparseSegmentTree<Affine, Compose, Identity>;

Key Evaluate(Affine f, Key x)
{
    return (f.a * x + f.b) % kMod;
}

void Check(const Tree &tree, const std::map<Key, Affine> &reference, Key left, Key right)
{
    auto at_zero = Key{0};
    auto at_one = Key{1};
    for (auto at = reference.lower_bound(left); at != reference.end() && at->first < right; ++at)
    {
        at_zero = Evaluate(at->second, at_zero);
        at_one = Evaluate(at->second, at_one);
    }
    const auto actual = tree.prod(left, right);
    assert(actual.b == at_zero);
    assert((actual.a + actual.b) % kMod == at_one);
}

template <class Exception, class F>
void ExpectThrow(F action)
{
    bool thrown = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}

void CheckEmptyAndInvalid()
{
    const Tree empty;
    assert(empty.size() == 0);
    assert(empty.stored_points() == 0);
    assert(empty.prod(0, 0) == Identity());
    assert(empty.all_prod() == Identity());

    Tree actual(5);
    assert(actual.get(3) == Identity());
    assert(actual.prod(0, 4) == Identity());
    assert(!actual.erase(2));
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       actual.set(5, {1, 4});
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)actual.get(kLargest);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)actual.erase(5);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)actual.prod(5, 3);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)actual.prod(0, 6);
                                   });
#endif

    using sirkit::detail::NextSparseSegmentTreeCapacity;
    assert(NextSparseSegmentTreeCapacity(3, 6, 3, 7) == 6);
    assert(NextSparseSegmentTreeCapacity(6, 6, 1, 7) == 7);
    assert(NextSparseSegmentTreeCapacity(3, 3, 1, 10) == 6);
    assert(NextSparseSegmentTreeCapacity(0, 0, 2, 5) == 2);
    ExpectThrow<std::length_error>([]
                                   {
                                       (void)NextSparseSegmentTreeCapacity(6, 6, 2, 7);
                                   });
}

void CheckExtremes()
{
    Tree tree(kLargest);
    tree.set(12, {3, 2});
    assert(tree.get(11) == Identity());
    assert(!tree.erase(15));
    assert(tree.prod(11, 12) == Identity());
    assert(tree.prod(12, 13) == (Affine{3, 2}));
    assert(tree.prod(13, 14) == Identity());

    Tree comb(kLargest);
    std::map<Key, Affine> reference;
    const auto add = [&](Key p, Affine f)
    {
        comb.set(p, f);
        reference[p] = f;
    };
    add(0, {3, 7});
    for (auto i = 0; i < 64; ++i)
    {
        add(Key{1} << i, {static_cast<Key>(i + 2), static_cast<Key>(i + 4)});
    }
    add(kLargest - 1, {4, 1});
    for (auto i = 0; i < 64; ++i)
    {
        const auto at = Key{1} << i;
        Check(comb, reference, at - 1, at + 1);
        Check(comb, reference, 0, at);
        Check(comb, reference, at, kLargest);
    }
    Check(comb, reference, 0, kLargest);
    Check(comb, reference, kLargest - 2, kLargest - 2);
    Check(comb, reference, kLargest, kLargest);

    Tree copy = comb;
    Tree assigned;
    assigned = copy;
    assert(assigned.erase(kLargest - 1));
    assert(assigned.get(kLargest - 1) == Identity());
    assert(comb.get(kLargest - 1) == (Affine{4, 1}));
    Tree moved = std::move(copy);
    assert(copy.size() == 0 && copy.prod(0, 0) == Identity());
    assert(moved.all_prod() == comb.all_prod());
    copy = std::move(moved);
    assert(moved.size() == 0 && moved.all_prod() == Identity());
    assert(copy.all_prod() == comb.all_prod());

    for (const auto &[point, ignored] : reference)
    {
        (void)ignored;
        assert(comb.erase(point));
    }
    assert(comb.stored_points() == 0);
    assert(comb.all_prod() == Identity());
    comb.set(kLargest - 1, {0, 5}); // 全削除後にスロットを再利用
    comb.set(1, {7, 1});
    comb.set(kLargest - 1, {2, 9});
    assert(comb.prod(1, kLargest) == (Affine{14, 11}));
    comb.set(1, Identity());
    assert(comb.stored_points() == 1);
    assert(!comb.erase(1));
}

struct WithoutEquality
{
    int value;
    explicit WithoutEquality(int v) : value(v)
    {
    }
    WithoutEquality() = delete;
};
WithoutEquality Plus(const WithoutEquality &left, const WithoutEquality &right) noexcept
{
    return WithoutEquality(left.value + right.value);
}
WithoutEquality Zero()
{
    return WithoutEquality(0);
}

void CheckGenericValue()
{
    sirkit::SparseSegmentTree<WithoutEquality, Plus, Zero> tree(1000);
    tree.set(23, WithoutEquality(0));
    tree.set(42, WithoutEquality(5));
    assert(tree.stored_points() == 2);
    assert(tree.get(23).value == 0);
    assert(tree.prod(0, 51).value == 5);
    assert(tree.erase(23));
    assert(!tree.erase(23));
    assert(tree.stored_points() == 1);
}

void CheckRandom()
{
    std::mt19937_64 random(0x1cb0e58a0213a1ULL);
    for (const Key n : {Key{1}, Key{2}, Key{3}, Key{19}, Key{129}, Key{1} << 30, kLargest})
    {
        Tree tree(n);
        std::map<Key, Affine> reference;
        std::vector<Key> positions{0, n - 1};
        for (auto i = 0; i < 150; ++i)
        {
            positions.push_back(random() % n);
        }
        for (auto i = 0; i < 5'000; ++i)
        {
            const auto p = positions[random() % positions.size()];
            if (i % 9 == 0)
            {
                assert(tree.erase(p) == (reference.erase(p) != 0));
            }
            else if (i % 7 == 0)
            {
                tree.set(p, Identity());
                reference.erase(p);
            }
            else if (i % 3 == 0)
            {
                const Affine change{random() % kMod, random() % kMod};
                tree.set(p, change);
                reference[p] = change;
            }
            else
            {
                const auto a = positions[random() % positions.size()];
                const auto b = positions[random() % positions.size()];
                Check(tree, reference, std::min(a, b), std::max(a, b) + 1);
                if (i % 13 == 0)
                {
                    Check(tree, reference, 0, n);
                }
            }
            assert(tree.get(p) == (reference.contains(p) ? reference.at(p) : Identity()));
            assert(tree.stored_points() == reference.size());
        }
    }
}
}

int main()
{
    CheckEmptyAndInvalid();
    CheckExtremes();
    CheckGenericValue();
    CheckRandom();
}
