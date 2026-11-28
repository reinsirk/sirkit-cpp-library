// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Multiset_Permutation.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
using Int = sirkit::BigInteger<>;

void CheckEnumerated(std::vector<int> values)
{
    const sirkit::MultisetPermutation order(values);
    assert(order.size() == static_cast<int>(values.size()));
    std::sort(values.begin(), values.end());
    std::int64_t expected = 0;
    do
    {
        assert(order.rank(values).value() == expected);
        assert(order.unrank(expected).value() == values);
        ++expected;
    } while (std::next_permutation(values.begin(), values.end()));
    assert(order.count() == expected);
    assert(!order.unrank(expected));
}

void CheckLargeExact()
{
    std::vector<int> word(200, -13);
    word.insert(word.end(), 200, 7);
    const sirkit::MultisetPermutation order(word);
    const Int total("102952500135414432972975880320401986757210925381077648234849059575923332372651958598336595518976492951564048597506774120");
    assert(order.count() == total);
    assert(order.rank(word).value() == 0);
    assert(order.unrank(0).value() == word);
    std::reverse(word.begin(), word.end());
    assert(order.rank(word).value() == total - 1);
    assert(order.unrank(total - 1).value() == word);
    auto middle = total.divmod(2u).first;
    word.assign(1, 7);
    word.insert(word.end(), 200, -13);
    word.insert(word.end(), 199, 7);
    assert(order.rank(word).value() == middle);
    assert(order.unrank(middle).value() == word);

    std::vector<int> many_distinct(80);
    std::iota(many_distinct.begin(), many_distinct.end(), -40);
    const sirkit::MultisetPermutation distinct(many_distinct);
    Int factorial = 1;
    for (std::int64_t i = 2; i <= 80; ++i)
    {
        factorial *= i;
    }
    assert(distinct.count() == factorial);
    std::reverse(many_distinct.begin(), many_distinct.end());
    assert(distinct.rank(many_distinct).value() == factorial - 1);
    assert(distinct.unrank(factorial - 1).value() == many_distinct);
}

void CheckMalformed()
{
    const sirkit::MultisetPermutation order(std::vector{1, 1, 3});
    assert(!order.rank(std::vector{1, 1}));
    assert(!order.rank(std::vector{0, 1, 1}));
    assert(!order.rank(std::vector{1, 1, 4}));
    assert(!order.rank(std::vector{1, 1, 1}));
    assert(!order.unrank(-1));
    assert(!order.unrank(Int("100000000000000000000000000000000000")));
    bool capacity_rejected = false;
    try
    {
        (void)sirkit::detail::MultisetPermutationSize(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
    }
    catch (const std::length_error &)
    {
        capacity_rejected = true;
    }
    assert(capacity_rejected);
}
} // namespace

int main()
{
    CheckEnumerated({});
    CheckEnumerated({1, 1, 1});
    CheckEnumerated({3, 0, -7, 2});
    CheckEnumerated({1, 3, 2, 3, 1});
    std::mt19937 rng(215);
    for (int trial = 0; trial < 73; ++trial)
    {
        std::vector<int> values(rng() % 9);
        for (auto &value : values)
        {
            value = static_cast<int>(rng() % 5) - 3;
        }
        CheckEnumerated(values);
    }
    const sirkit::MultisetPermutation text(std::vector<std::string>{"z", "a", "z"});
    const std::vector<std::string> middle{"z", "a", "z"};
    assert(text.rank(middle).value() == 1);
    assert(text.unrank(1).value() == middle);
    CheckLargeExact();
    CheckMalformed();
}
