// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Dynamic_Wavelet_Matrix.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

void CheckBlocks()
{
    // The 48-bit first block is grown to 64, then the rightmost block shrinks to 15.
    for (const int trailing : {16, 32})
    {
        std::vector<std::uint32_t> values(48 + trailing);
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            values[i] = (i * 79U % 7U) < 3U;
        }
        sirkit::DynamicWaveletMatrix<std::uint32_t, 1> matrix(values);
        for (int i = 0; i < 16; ++i)
        {
            const auto value = static_cast<std::uint32_t>(i % 2);
            matrix.insert(0, value);
            values.insert(values.begin(), value);
        }
        while (values.size() > 79)
        {
            assert(matrix.erase(values.size() - 1) == values.back());
            values.pop_back();
        }
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            assert(matrix.access(i) == values[i]);
        }
    }
    // Make a shortened first block pair with a full successor.
    std::vector<std::uint32_t> left(96, 0);
    sirkit::DynamicWaveletMatrix<std::uint32_t, 1> matrix(left);
    for (int i = 0; i < 16; ++i)
    {
        matrix.insert(matrix.size(), 1);
        left.push_back(1);
    }
    for (int i = 0; i < 33; ++i)
    {
        assert(matrix.erase(0) == left.front());
        left.erase(left.begin());
    }
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        assert(matrix.access(i) == left[i]);
    }
    while (left.size())
    {
        assert(matrix.erase(0) == left.front());
        left.erase(left.begin());
    }
    assert(matrix.empty());
    matrix.insert(0, 1);
    assert(matrix.erase(0) == 1);
}

template <class T, int Bits>
void CheckRandom()
{
    using Matrix = sirkit::DynamicWaveletMatrix<T, Bits>;
    std::mt19937_64 random(Bits * 18091U);
    constexpr T kMaximum = std::numeric_limits<T>::max() >> (std::numeric_limits<T>::digits - Bits);
    auto value = [&]()
    {
        return static_cast<T>(random()) & kMaximum;
    };
    for (std::size_t initial : {0, 1, 48, 49, 97, 555})
    {
        std::vector<T> values(initial);
        std::generate(values.begin(), values.end(), value);
        Matrix matrix(values, random());

        for (int step = 0; step < 2400; ++step)
        {
            const auto operation = random() % 20;
            if (matrix.empty() || operation < 6)
            {
                const std::size_t position = random() % (values.size() + 1);
                const T x = value();
                values.insert(values.begin() + position, x);
                matrix.insert(position, x);
            }
            else if (operation < 10)
            {
                const std::size_t position = random() % values.size();
                assert(matrix.erase(position) == values[position]);
                values.erase(values.begin() + position);
            }
            else if (operation < 14)
            {
                const std::size_t position = random() % values.size();
                const T x = value();
                values[position] = x;
                matrix.set(position, x);
            }
            else
            {
                const std::size_t position = random() % values.size();
                assert(matrix.access(position) == values[position]);
            }

            auto first = random() % (values.size() + 1);
            auto last = random() % (values.size() + 1);
            if (first > last)
            {
                std::swap(first, last);
            }
            const T x = value();
            std::vector<T> sorted(values.begin() + first, values.begin() + last);
            std::sort(sorted.begin(), sorted.end());
            assert(matrix.size() == values.size());
            assert(matrix.count(first, last, x) == static_cast<std::size_t>(std::count(sorted.begin(), sorted.end(), x)));
            assert(matrix.count_less(first, last, x) == static_cast<std::size_t>(std::lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin()));
            if (!sorted.empty())
            {
                const auto k = random() % sorted.size();
                assert(matrix.kth_smallest(first, last, k) == sorted[k]);
            }

            if (step % 239 == 0)
            {
                Matrix copy = matrix;
                Matrix moved = std::move(copy);
                assert(copy.empty() && moved.size() == matrix.size());
                copy = moved;
                matrix = std::move(moved);
                assert(moved.empty() && copy.size() == matrix.size());
                for (std::size_t i = 0; i < values.size(); ++i)
                {
                    assert(copy.access(i) == matrix.access(i));
                    assert(matrix.access(i) == values[i]);
                }
            }
        }
        while (!values.empty())
        {
            assert(matrix.erase(0) == values.front());
            values.erase(values.begin());
        }
        assert(matrix.empty() && matrix.count(0, 0, T{0}) == 0);
    }
}

void CheckFixedWidths()
{
    const std::array<std::uint64_t, 5> extremes{0, UINT64_MAX, UINT64_MAX - 1, UINT64_C(1) << 63, 0};
    sirkit::DynamicWaveletMatrix<std::uint64_t> wide(extremes);
    assert(wide.kth_smallest(0, 5, 3) == UINT64_MAX - 1 && wide.kth_smallest(0, 5, 4) == UINT64_MAX);
    assert(wide.count_less(0, 5, UINT64_MAX) == 4 && wide.erase(1) == UINT64_MAX);
    const std::array<std::uint8_t, 4> values{5, 1, 6, 5};
    sirkit::DynamicWaveletMatrix<std::uint8_t, 3> small(values);
    assert(small.count(1, 4, 8) == 0 && small.count_less(1, 4, 8) == 3);
    assert(small.kth_smallest(0, 4, 0) == 1 && small.count_less(0, 4, 0) == 0);

#ifndef SKIP_RUNTIME_CHECKS
    auto fails = [](auto action)
    {
        try
        {
            action();
        }
        catch (const std::out_of_range &)
        {
            return true;
        }
        return false;
    };
    assert(fails([&]
                 {
                     small.insert(5, 0);
                 }));
    assert(fails([&]
                 {
                     small.insert(0, 8);
                 }));
    assert(fails([&]
                 {
                     small.set(0, 8);
                 }));
    assert(fails([&]
                 {
                     small.set(4, 0);
                 }));
    assert(fails([&]
                 {
                     (void)small.access(4);
                 }));
    assert(fails([&]
                 {
                     (void)small.erase(4);
                 }));
    assert(fails([&]
                 {
                     (void)small.kth_smallest(0, 4, 4);
                 }));
    assert(fails([&]
                 {
                     (void)small.count(4, 3, 0);
                 }));
    assert(fails([&]
                 {
                     (void)small.count_less(0, 5, 0);
                 }));
    assert(fails([&]
                 {
                     const std::array<std::uint8_t, 1> bad{8};
                     decltype(small) invalid(bad);
                 }));
    assert(small.size() == 4 && small.access(0) == 5);
#endif
}

int main()
{
    CheckBlocks();
    CheckRandom<std::uint8_t, 1>();
    CheckRandom<std::uint32_t, 9>();
    CheckRandom<std::uint64_t, 64>();
    CheckFixedWidths();
}
