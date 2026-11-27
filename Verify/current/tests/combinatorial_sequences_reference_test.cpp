// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Formal_Power_Series.cpp"
#include "../../../Math/Combinatorial_Sequences.cpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>

namespace
{
constexpr std::uint64_t kMod = 998244353;

template <class Exception, class Operation>
void expect_error(Operation run)
{
    try
    {
        run();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}

std::uint64_t reference_power(std::uint64_t base, std::uint64_t exponent)
{
    std::uint64_t result = 1;
    for (; exponent != 0; exponent /= 2)
    {
        if (exponent % 2 != 0)
        {
            result = result * base % kMod;
        }
        base = base * base % kMod;
    }
    return result;
}
}

int main()
{
    using sirkit::bell_numbers;
    using sirkit::derangement_numbers;
    using sirkit::partition_numbers;
    using sirkit::stirling_first_row;
    using sirkit::stirling_second_row;
    constexpr std::size_t kSize = 270;
    std::vector<std::uint32_t> partitions(kSize + 1);
    partitions[0] = 1;
    for (std::size_t part = 1; part <= kSize; ++part)
    {
        for (std::size_t s = part; s <= kSize; ++s)
        {
            partitions[s] = static_cast<std::uint32_t>((partitions[s] + partitions[s - part]) % kMod);
        }
    }
    for (const auto n : {0U, 1U, 4U, 17U, 129U, 270U})
    {
        assert(partition_numbers(n) == std::vector<std::uint32_t>(partitions.begin(), partitions.begin() + n + 1));
    }
    std::vector<std::uint32_t> first{1}, second{1}, bell(kSize + 1, 1);
    assert((stirling_first_row(0) == first && stirling_second_row(0) == second));
    for (std::size_t n = 1; n <= kSize; ++n)
    {
        first.push_back(0);
        second.push_back(0);
        std::uint64_t total = 0;
        for (std::size_t k = n; k > 0; --k)
        {
            first[k] = static_cast<std::uint32_t>((first[k - 1] + kMod - std::uint64_t{n - 1} * first[k] % kMod) % kMod);
            second[k] = static_cast<std::uint32_t>((second[k - 1] + std::uint64_t{k} * second[k]) % kMod);
            total += second[k];
        }
        first[0] = 0;
        second[0] = 0;
        bell[n] = static_cast<std::uint32_t>(total % kMod);
        if (n < 70 || n == 128 || n == 129 || n == kSize)
        {
            assert(stirling_first_row(n) == first);
            assert(stirling_second_row(n) == second);
        }
    }
    assert(bell_numbers(kSize) == bell);
    assert((bell_numbers(0) == std::vector<std::uint32_t>{1}));
    constexpr std::uint32_t kBig = std::numeric_limits<std::uint32_t>::max();
    const auto derangements = derangement_numbers(9, kBig);
    for (std::size_t n = 0; n <= 9; ++n)
    {
        std::vector<std::size_t> p(n);
        std::iota(p.begin(), p.end(), std::size_t{0});
        std::uint32_t count = 0;
        do
        {
            bool has_fixed_point = false;
            for (std::size_t i = 0; i < n; ++i)
            {
                has_fixed_point = has_fixed_point || (p[i] == i);
            }
            count += !has_fixed_point;
        } while (std::next_permutation(p.begin(), p.end()));
        assert(derangements[n] == count);
    }
    assert((derangement_numbers(4, 1) == std::vector<std::uint32_t>(5)));
    assert((derangement_numbers(3) == std::vector<std::uint32_t>{1, 0, 1, 2}));
    const auto large = derangement_numbers(80, kBig);
    assert(std::any_of(large.begin(), large.end(), [](auto x)
                       {
                           return x > (std::uint64_t{1} << 31);
                       }));
    assert((sirkit::subset_sum_counts({}, 0) == std::vector<std::uint32_t>{1}));
    assert((sirkit::subset_sum_counts(std::array<std::size_t, 3>{0, 0, 1}, 2) ==
            std::vector<std::uint32_t>{4, 4, 0}));
    std::mt19937 random(9365);
    for (unsigned trial = 0; trial < 300; ++trial)
    {
        const std::size_t goal = random() % 45;
        std::vector<std::size_t> a(random() % 14);
        for (auto &value : a)
        {
            value = random() % 16;
        }
        std::vector<std::uint32_t> expected(goal + 1);
        for (std::uint32_t mask = 0; mask < (1U << a.size()); ++mask)
        {
            std::size_t sum = 0;
            for (std::size_t j = 0; j < a.size(); ++j)
            {
                if ((mask >> j) & 1)
                {
                    sum += a[j];
                }
            }
            if (sum <= goal)
            {
                ++expected[sum];
            }
        }
        assert(sirkit::subset_sum_counts(a, goal) == expected);
    }
    expect_error<std::length_error>([]
                                    {
                                        partition_numbers(sirkit::FormalPowerSeries::max_precision());
                                    });
    expect_error<std::length_error>([]
                                    {
                                        derangement_numbers(std::numeric_limits<std::size_t>::max());
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            derangement_numbers(0, 0);
                                        });

    constexpr std::size_t kBernoulliSize = 180;
    const auto actual_bernoulli = sirkit::bernoulli_numbers(kBernoulliSize);
    std::vector<std::uint64_t> row(kBernoulliSize + 2), reference_bernoulli{1};
    row[0] = row[1] = 1;
    assert((sirkit::bernoulli_numbers(0) == std::vector<std::uint32_t>{1}));
    for (std::size_t n = 1; n <= kBernoulliSize; ++n)
    {
        for (std::size_t i = n + 1; i != 0; --i)
        {
            row[i] = (row[i] + row[i - 1]) % kMod;
        }
        std::uint64_t sum = 0;
        for (std::size_t i = 0; i < n; ++i)
        {
            sum = (sum + row[i] * reference_bernoulli[i]) % kMod;
        }
        const auto denominator_inverse = reference_power(n + 1, kMod - 2);
        reference_bernoulli.push_back((kMod - sum) * denominator_inverse % kMod);
        assert(actual_bernoulli[n] == reference_bernoulli[n]);
    }
    for (unsigned exponent = 0; exponent < 35; ++exponent)
    {
        assert(sirkit::sum_of_powers(0, exponent) == 0);
        std::uint64_t sums = 0;
        for (unsigned value = 1; value < 70; ++value)
        {
            sums = (sums + reference_power(value, exponent)) % kMod;
            assert(sirkit::sum_of_powers(value, exponent) == sums);
        }
    }
    constexpr auto kFullWidth = std::numeric_limits<std::uint64_t>::max();
    constexpr auto kReduced = kFullWidth % kMod;
    assert(sirkit::sum_of_powers(kFullWidth, 0) == kReduced);
    assert(sirkit::sum_of_powers(kFullWidth, 1) == kReduced * (kReduced + 1) % kMod * ((kMod + 1) / 2) % kMod);
    assert(sirkit::sum_of_powers(kMod - 1, 3) == 0);
}
