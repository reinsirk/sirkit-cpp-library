// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Rollback_Lazy_Segment_Tree.cpp"

#include <array>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
void Expect(bool value)
{
    if (!value)
    {
        std::abort();
    }
}

template <class Error, class F>
void ExpectThrow(F method)
{
    try
    {
        method();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

struct Affine
{
    long long scale;
    long long shift;
    bool fail = false;
};

long long Add(long long a, long long b)
{
    return a + b;
}

long long Zero()
{
    return 0;
}

long long Transform(const Affine &f, long long sum, std::size_t length)
{
    if (f.fail && length == 1)
    {
        throw std::runtime_error("action fails on a leaf");
    }
    return f.scale * sum + f.shift * static_cast<long long>(length);
}

Affine Compose(const Affine &f, const Affine &g)
{
    return {f.scale * g.scale, f.scale * g.shift + f.shift, f.fail || g.fail};
}

bool Identity(const Affine &f, long long, std::size_t)
{
    return f.scale == 1 && f.shift == 0 && !f.fail;
}

using Tree = sirkit::RollbackLazySegmentTree<long long, Affine, Add, Zero, Transform, Compose, Identity>;

void Check(const Tree &tree, const std::vector<long long> &values)
{
    const auto mark = tree.snapshot();
    Expect(tree.size() == values.size());
    Expect(tree.all_prod() == std::accumulate(values.begin(), values.end(), 0LL));
    Expect(tree.prod(0, tree.size()) == tree.all_prod());
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        Expect(tree.get(i) == values[i]);
    }
    Expect(tree.snapshot() == mark);
}

void Random()
{
    std::mt19937 rng(60051123);
    for (int trial = 0; trial < 90; ++trial)
    {
        const std::size_t n = 1 + rng() % 37;
        std::vector<long long> values(n);
        for (auto &value : values)
        {
            value = static_cast<int>(rng() % 71) - 35;
        }
        Tree tree(values);
        std::vector<std::pair<Tree::Snapshot, std::vector<long long>>> saved;
        for (int i = 0; i < 150; ++i)
        {
            std::size_t left = rng() % (n + 1), right = rng() % (n + 1);
            if (left > right)
            {
                std::swap(left, right);
            }
            switch (rng() % 6)
            {
            case 0:
            {
                const Affine f{static_cast<int>(rng() % 3) - 1, static_cast<int>(rng() % 19) - 9};
                tree.apply(left, right, f);
                for (std::size_t j = left; j < right; ++j)
                {
                    values[j] = f.scale * values[j] + f.shift;
                }
                break;
            }
            case 1:
            {
                const auto at = rng() % n;
                values[at] = static_cast<int>(rng() % 300) - 150;
                tree.set(at, values[at]);
                break;
            }
            case 2:
                saved.emplace_back(tree.snapshot(), values);
                break;
            case 3:
                if (!saved.empty())
                {
                    const std::size_t chosen = rng() % saved.size();
                    const auto &[mark, old] = saved[chosen];
                    tree.rollback(mark);
                    values = old;
                    saved.resize(chosen + 1);
                }
                break;
            case 4:
                Expect(tree.prod(left, right) == std::accumulate(values.begin() + left, values.begin() + right, 0LL));
                break;
            case 5:
                tree.clear_history();
                saved.clear();
                break;
            }
            Check(tree, values);
        }
    }
}

std::string Concat(const std::string &a, const std::string &b)
{
    return a + b;
}

std::string Empty()
{
    return {};
}

std::string Shift(int shift, std::string current, std::size_t)
{
    for (auto &c : current)
    {
        c = 'a' + (c - 'a' + shift) % 26;
    }
    return current;
}

int Sum(int a, int b)
{
    return (a + b) % 26;
}

void Noncommutative()
{
    const std::array<std::string, 6> vertices{"a", "b", "c", "d", "e", "f"};
    sirkit::RollbackLazySegmentTree<std::string, int, Concat, Empty, Shift, Sum> tree(vertices);
    const auto mark = tree.snapshot();
    tree.apply(0, 6, 1);
    tree.apply(1, 4, 2);
    Expect(tree.prod(0, 4) == "befg");
    tree.set(2, "q");
    Expect(tree.prod(0, 6) == "beqgfg");
    Expect(tree.get(2) == "q");
    tree.rollback(mark);
    Expect(tree.prod(1, 5) == "bcde");
}

void Limits()
{
    const std::vector<long long> start{1, 2, 3, 4};
    Tree tree(start);
    const auto mark = tree.snapshot();
    ExpectThrow<std::runtime_error>([&]
                                    {
                                        tree.apply(0, 3, {1, 5, true});
                                    });
    Expect(tree.snapshot() == mark);
    Check(tree, start);
    tree.apply(0, 4, {1, 5, true});
    const auto pending = tree.snapshot();
    ExpectThrow<std::runtime_error>([&]
                                    {
                                        tree.set(0, 0);
                                    });
    Expect(tree.snapshot() == pending && tree.all_prod() == 30);
    tree.rollback(mark);
    Check(tree, start);
    tree.apply(0, 4, {1, 0});
    Expect(tree.snapshot() == mark);
    tree.apply(0, 4, {2, 1});
    tree.apply(0, 4, {-1, 2});
    auto independent = tree;
    tree.rollback(mark);
    Check(tree, start);
    Check(independent, {-1, -3, -5, -7});
    tree = independent;
    Tree moved(std::move(tree));
    Check(tree, {});
    Check(moved, {-1, -3, -5, -7});
    tree = std::move(moved);
    Check(moved, {});
    Check(tree, {-1, -3, -5, -7});

    const std::span<const long long> no_values;
    const Tree empty(no_values);
    Check(empty, {});
    Tree zeros(3);
    Check(zeros, {0, 0, 0});
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       tree.rollback(tree.snapshot() + 1);
                                   });
    ExpectThrow<std::length_error>([]
                                   {
                                       Tree large(std::numeric_limits<std::size_t>::max());
                                   });
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       tree.set(4, 0);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)tree.get(4);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       tree.apply(3, 2, {1, 5});
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)tree.prod(3, 5);
                                   });
#endif
}
}

int main()
{
    Random();
    Noncommutative();
    Limits();
}
