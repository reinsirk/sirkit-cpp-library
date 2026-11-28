// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Point_Set_Mex.cpp"

#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <set>

template <std::integral T>
static std::size_t brute(const std::vector<T> &values)
{
    const std::set<T> present(values.begin(), values.end());
    std::size_t missing = 0;
    while (missing <= static_cast<std::size_t>(std::numeric_limits<T>::max()) && present.contains(static_cast<T>(missing)))
    {
        ++missing;
    }
    return missing;
}

int main()
{
    const sirkit::PointSetMex<int> empty;
    assert(empty.size() == 0 && empty.mex() == 0);
    const sirkit::PointSetMex<int> empty_vector(std::vector<int>{});
    assert(empty_vector.size() == 0 && empty_vector.mex() == 0);
    sirkit::PointSetMex<int> index({0, 0, -1, 4});
    assert(index.size() == 4 && index.mex() == 1);
    index.set(0, 0);
    index.set(2, 3);
    index.set(3, 2);
    assert(index.mex() == 1 && index.get(2) == 3);
    index.set(1, 1);
    assert(index.mex() == 4);
    index.set(2, -2147483647);
    assert(index.mex() == 3 && index.get(2) == -2147483647);
    index.set(0, 0);
    auto copy = index;
    copy.set(0, 2);
    copy.set(1, 2);
    assert(copy.mex() == 0 && index.mex() == 3);
    index = copy;
    assert(index.mex() == 0 && index.get(1) == 2);
    auto moved = std::move(index);
    assert(index.size() == 0 && index.mex() == 0);
    assert(moved.size() == 4 && moved.mex() == 0);
    index = std::move(moved);
    assert(moved.size() == 0 && moved.mex() == 0);
    auto &alias = index;
    index = std::move(alias);
    assert(index.size() == 4 && index.mex() == 0);
    index.set(0, 0);
    assert(index.mex() == 1);
    moved = index;
    assert(moved.mex() == 1);

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    sirkit::PointSetMex big_unsigned(std::vector<std::uint64_t>{0, kMax});
    assert(big_unsigned.mex() == 1 && big_unsigned.get(1) == kMax);
    big_unsigned.set(1, 1);
    assert(big_unsigned.mex() == 2);
    sirkit::PointSetMex two_values(std::vector<bool>{true, false});
    assert(two_values.mex() == 2 && two_values.get(0) && !two_values.get(1));
    two_values.set(0, false);
    assert(two_values.mex() == 1 && !two_values.get(0));
    const auto kWideNegative = std::numeric_limits<std::int64_t>::min();
    sirkit::PointSetMex big_negative(std::vector<std::int64_t>{kWideNegative, 0});
    assert(big_negative.mex() == 1);
    big_negative.set(1, kWideNegative);
    assert(big_negative.mex() == 0 && big_negative.get(1) == kWideNegative);

#ifndef SKIP_RUNTIME_CHECKS
    bool bad_get = false;
    try
    {
        (void)index.get(index.size());
    }
    catch (const std::out_of_range &)
    {
        bad_get = true;
    }
    assert(bad_get);
    bool bad_set = false;
    try
    {
        index.set(index.size(), 0);
    }
    catch (const std::out_of_range &)
    {
        bad_set = true;
    }
    assert(bad_set && index.mex() == 1);
#endif

    std::mt19937 random(12045);
    for (int trial = 0; trial < 360; ++trial)
    {
        const int n = 1 + random() % 112;
        std::vector<int> slow(n);
        for (int &number : slow)
        {
            number = static_cast<int>(random() % (n + 7)) - 3;
        }
        sirkit::PointSetMex fast(slow);
        for (int query = 0; query < 100; ++query)
        {
            assert(fast.mex() == brute(slow));
            const auto i = random() % n;
            assert(fast.get(i) == slow[i]);
            const int next = static_cast<int>(random() % (n + 7)) - 3;
            slow[i] = next;
            fast.set(i, next);
        }
        assert(fast.mex() == brute(slow));
    }
    for (const int n : {63, 64, 65, 4095, 4096, 4097, 65536})
    {
        std::vector<int> initial(n);
        std::iota(initial.begin(), initial.end(), 0);
        sirkit::PointSetMex dense(std::move(initial));
        assert(dense.mex() == static_cast<std::size_t>(n));
        for (int i = 0; i < n; i += 31)
        {
            dense.set(i, n);
            assert(dense.mex() == static_cast<std::size_t>(i));
            dense.set(i, i);
            assert(dense.mex() == static_cast<std::size_t>(n));
        }
    }
}
