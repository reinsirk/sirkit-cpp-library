// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Sqrt_Tree.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Affine = std::pair<std::uint64_t, std::uint64_t>;

struct Compose
{
    Affine operator()(const Affine &left, const Affine &right) const
    {
        return {left.first * right.first % 97, (left.second * right.first + right.second) % 97};
    }
};

void CheckAffine(std::size_t n, std::mt19937_64 &random)
{
    std::vector<Affine> functions(n);
    for (auto &value : functions)
    {
        value = {random() % 97, random() % 97};
    }
    constexpr Affine identity{1, 0};
    const sirkit::SqrtTree tree(functions, identity, Compose{});
    assert(tree.size() == n);
    assert(tree.empty() == (n == 0));

    for (std::size_t left = 0; left <= n; ++left)
    {
        assert(tree.prod(left, left) == identity);
        if (left < n)
        {
            assert(tree.get(left) == functions[left]);
        }
        std::uint64_t at_zero = 0;
        std::uint64_t at_one = 1;
        for (std::size_t right = left; right < n; ++right)
        {
            const auto [scale, shift] = functions[right];
            at_zero = (scale * at_zero + shift) % 97;
            at_one = (scale * at_one + shift) % 97;
            const Affine expected{(at_one + 97 - at_zero) % 97, at_zero};
            assert(tree.prod(left, right + 1) == expected);
            if (left == 0 && right + 1 == n)
            {
                assert(tree.all_prod() == expected);
            }
        }
    }
}

template <class Call>
void ExpectOutOfRange(Call call)
{
    bool rejected = false;
    try
    {
        call();
    }
    catch (const std::out_of_range &)
    {
        rejected = true;
    }
    assert(rejected);
}
} // namespace

int main()
{
    const sirkit::SqrtTree<Affine, Compose> empty({}, {1, 0});
    const Affine identity{1, 0};
    assert(empty.all_prod() == identity);

    std::mt19937_64 random(378);
    for (std::size_t n = 0; n < 100; ++n)
    {
        CheckAffine(n, random);
    }
    CheckAffine(513, random);
    const auto all_true = [](bool a, bool b)
    {
        return a && b;
    };
    const sirkit::SqrtTree boolean(std::vector<bool>{true, true, false, true, true, true, true, true, false}, true, all_true);
    assert(boolean.get(0) && !boolean.get(2));
    assert(boolean.prod(0, 2) && !boolean.prod(0, 5));
    assert(boolean.prod(3, 8) && !boolean.all_prod());

    ExpectOutOfRange([&]
                     {
                         (void)boolean.get(boolean.size());
                     });
    ExpectOutOfRange([&]
                     {
                         (void)boolean.prod(3, 1);
                     });
    ExpectOutOfRange([&]
                     {
                         (void)boolean.prod(0, boolean.size() + 1);
                     });
}
