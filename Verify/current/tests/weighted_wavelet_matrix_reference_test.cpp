// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Weighted_Wavelet_Matrix.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct PairWeight
{
    int mass = 0, cost = 0;

    friend PairWeight operator+(PairWeight a, PairWeight b)
    {
        return {a.mass + b.mass, a.cost + b.cost};
    }

    friend PairWeight operator-(PairWeight a, PairWeight b)
    {
        return {a.mass - b.mass, a.cost - b.cost};
    }

    bool operator==(const PairWeight &) const = default;
};
}

int main()
{
    using Matrix = sirkit::WeightedWaveletMatrix<>;
    using I = std::int64_t;
    constexpr I kLow = std::numeric_limits<I>::min();
    constexpr I kHigh = std::numeric_limits<I>::max();

    const Matrix empty;
    assert(empty.empty() && empty.size() == 0 && empty.sum(0, 0) == 0);
    assert(empty.count_sum_less(0, 0, kHigh) == Matrix::CountSum{});
    assert(empty.count_sum_less_equal(0, 0, kLow) == Matrix::CountSum{});

    const std::array<I, 7> keys{kLow, -2, 1, 1, kHigh, kHigh, 5};
    const std::array<I, 7> weights{9, -2, 5, -10, 8, -2, 4};
    const Matrix matrix(keys, weights);
    assert(!matrix.empty() && matrix.size() == 7 && matrix.sum(0, 7) == 12);
    assert(matrix.count_sum_less(0, 7, kLow) == Matrix::CountSum{});
    assert((matrix.count_sum_less_equal(0, 7, kLow) == Matrix::CountSum{1, 9}));
    assert((matrix.count_sum_less(0, 7, kHigh) == Matrix::CountSum{5, 6}));
    assert((matrix.count_sum_less_equal(0, 7, kHigh) == Matrix::CountSum{7, 12}));
    assert((matrix.count_sum(1, 6, 1, kHigh) == Matrix::CountSum{2, -5}));
    assert(matrix.count_sum(1, 6, 5, 5) == Matrix::CountSum{});
    assert(matrix.count_sum(1, 6, 6, 5) == Matrix::CountSum{});
    assert(matrix.count_sum(3, 3, kLow, kHigh) == Matrix::CountSum{});

    const std::array<I, 4> equal{-7, -7, -7, -7};
    const Matrix singleton(equal, std::array<I, 4>{1, 2, 3, 4});
    assert((singleton.count_sum_less_equal(1, 4, -7) == Matrix::CountSum{3, 9}));
    assert(singleton.count_sum_less(1, 4, -7) == Matrix::CountSum{});

    auto copy = matrix;
    auto moved = std::move(copy);
    assert(copy.empty() && moved.sum(1, 4) == -7);
    auto &move_alias = moved;
    moved = std::move(move_alias);
    copy = moved;
    const auto &copy_alias = copy;
    copy = copy_alias;
    assert(copy.sum(0, 7) == 12);
    moved = std::move(copy);
    assert(copy.empty() && moved.sum(0, 7) == 12);

    const std::array<I, 4> wrap_keys{2, 0, 1, 3};
    const std::array<I, 4> wrap_weights{kHigh, 1, kHigh, kLow};
    const Matrix wrap(wrap_keys, wrap_weights);
    assert(wrap.sum(0, 2) == kLow);
    assert((wrap.count_sum_less_equal(0, 4, 2) == Matrix::CountSum{3, -1}));
    assert((wrap.count_sum(0, 4, 1, 3) == Matrix::CountSum{2, -2}));
    const std::array<std::int8_t, 4> narrow_weights{127, 1, -1, 127};
    const sirkit::WeightedWaveletMatrix<I, std::int8_t> narrow(wrap_keys, narrow_weights);
    assert(narrow.sum(0, 2) == -128);
    assert(narrow.count_sum_less(0, 4, 3).second == 127);

    const std::array<std::string, 4> labels{"a", "c", "b", "a"};
    const std::array<PairWeight, 4> pairs{{{1, 50}, {2, -2}, {3, 10}, {4, 20}}};
    const sirkit::WeightedWaveletMatrix<std::string, PairWeight> custom(labels, pairs);
    assert((custom.sum(0, 4) == PairWeight{10, 78}));
    const auto [count, weight] = custom.count_sum_less_equal(0, 4, std::string{"b"});
    assert((count == 3 && weight == PairWeight{8, 80}));

    std::mt19937_64 rng(6284357);
    for (int trial = 0; trial < 580; ++trial)
    {
        const auto n = static_cast<std::size_t>(trial % 193);
        const auto coord = [&]
        {
            const auto kind = rng() % 17;
            if (kind == 0)
            {
                return kLow;
            }
            if (kind == 1)
            {
                return kHigh;
            }
            return static_cast<I>(rng() % (1 + trial % 61)) - 30;
        };
        std::vector<I> ordered_keys(n), nonpaired_weights(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            ordered_keys[i] = coord();
            nonpaired_weights[i] = static_cast<I>(rng() % 40'021) - 20'010;
        }
        const Matrix candidate(ordered_keys, nonpaired_weights);
        for (int step = 0; step < 190; ++step)
        {
            const auto [left, right] = std::minmax({rng() % (n + 1), rng() % (n + 1)});
            const auto [lo, hi] = std::minmax({coord(), coord()});
            Matrix::CountSum less{}, at_most{}, inside{};
            I total = 0;
            for (auto i = left; i < right; ++i)
            {
                const auto k = ordered_keys[i];
                const auto w = nonpaired_weights[i];
                total += w;
                if (k < hi)
                {
                    ++less.first;
                    less.second += w;
                    if (lo <= k)
                    {
                        ++inside.first;
                        inside.second += w;
                    }
                }
                if (k <= hi)
                {
                    ++at_most.first;
                    at_most.second += w;
                }
            }
            assert(candidate.sum(left, right) == total);
            assert(candidate.count_sum_less(left, right, hi) == less);
            assert(candidate.count_sum_less_equal(left, right, hi) == at_most);
            assert(candidate.count_sum(left, right, lo, hi) == inside);
        }
    }

    try
    {
        const Matrix bad(keys, std::span<const I>{});
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        (void)sirkit::detail::CheckedWeightedWaveletSize(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[left, right] : {std::pair{3, 1}, std::pair{0, 8}})
    {
        try
        {
            (void)matrix.sum(left, right);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
