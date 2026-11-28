// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_Recurrence_Fit.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::int64_t kMod = 998244353;

std::int64_t Reduce(std::int64_t value)
{
    return (value % kMod + kMod) % kMod;
}

std::size_t DenseRank(std::vector<std::vector<std::int64_t>> matrix)
{
    std::size_t rank = 0;
    for (std::size_t j = 0; j < matrix.front().size() && rank < matrix.size(); ++j)
    {
        auto row = rank;
        while (row < matrix.size() && matrix[row][j] == 0)
        {
            ++row;
        }
        if (row == matrix.size())
        {
            continue;
        }
        matrix[rank].swap(matrix[row]);
        const auto base = matrix[rank][j];
        for (std::size_t i = rank + 1; i < matrix.size(); ++i)
        {
            const auto factor = matrix[i][j];
            for (std::size_t k = j; k < matrix[i].size(); ++k)
            {
                matrix[i][k] = Reduce(base * matrix[i][k] - factor * matrix[rank][k]);
            }
        }
        ++rank;
    }
    return rank;
}

void CheckFit(const std::vector<std::int64_t> &terms, std::size_t order, std::size_t degree)
{
    const auto width = degree + 1;
    std::vector<std::vector<std::int64_t>> full(terms.size() - order);
    for (std::size_t t = 0; t < full.size(); ++t)
    {
        for (std::size_t j = 0; j <= order; ++j)
        {
            auto value = Reduce(terms[t + j]);
            for (std::size_t h = 0; h < width; ++h)
            {
                full[t].push_back(value);
                value = value * static_cast<std::int64_t>(t) % kMod;
            }
        }
    }
    auto without_leading = full;
    for (auto &row : without_leading)
    {
        row.resize(order * width);
    }
    const bool exists = DenseRank(full) - DenseRank(without_leading) < width;
    const auto fit = sirkit::fit_polynomial_recurrence(terms, order, degree);
    assert(fit.has_value() == exists);
    if (!fit)
    {
        return;
    }

    assert(fit->size() == order + 1);
    bool nonzero_leading = false;
    for (const auto factor : fit->back())
    {
        nonzero_leading |= factor != 0;
    }
    assert(nonzero_leading);
    for (std::size_t t = 0; t < full.size(); ++t)
    {
        std::int64_t sum = 0;
        for (std::size_t j = 0; j <= order; ++j)
        {
            assert((*fit)[j].size() == width);
            for (std::size_t h = 0; h < width; ++h)
            {
                const auto factor = (*fit)[j][h];
                assert(0 <= factor && factor < kMod);
                sum = (sum + factor * full[t][j * width + h]) % kMod;
            }
        }
        assert(sum == 0);
    }
}

template <class Error, class Call>
void ExpectError(Call call)
{
    bool rejected = false;
    try
    {
        call();
    }
    catch (const Error &)
    {
        rejected = true;
    }
    assert(rejected);
}
} // namespace

int main()
{
    const std::vector<std::int64_t> factorials{1, 1, 2, 6, 24, 120, 720, 5040};
    CheckFit(factorials, 1, 1);
    const auto fit = sirkit::fit_polynomial_recurrence(factorials, 1, 1).value();
    const std::array<std::int64_t, 1> seed{1};
    assert(sirkit::polynomial_recurrence_term(seed, fit, 10) == 3628800u);
    CheckFit({0, 0, 0, 0, 0}, 2, 2);
    CheckFit({0, 0, 0, 1}, 1, 0);
    CheckFit({0, 0, 0, 1}, 1, 1);
    constexpr auto kSmallest = std::numeric_limits<std::int64_t>::min();
    CheckFit({kSmallest, kSmallest, kSmallest}, 1, 0);

    std::mt19937 random(222);
    for (int trial = 0; trial < 250; ++trial)
    {
        const auto order = static_cast<std::size_t>(1 + random() % 3);
        const auto degree = static_cast<std::size_t>(random() % 3);
        std::vector<std::int64_t> terms(order + 1 + random() % 12);
        for (auto &value : terms)
        {
            value = static_cast<std::int64_t>(random()) - 2000000000;
        }
        CheckFit(terms, order, degree);
    }

    ExpectError<std::invalid_argument>([&]
                                       {
                                           (void)sirkit::fit_polynomial_recurrence(factorials, 0, 0);
                                       });
    ExpectError<std::invalid_argument>([&]
                                       {
                                           (void)sirkit::fit_polynomial_recurrence(factorials, factorials.size(), 0);
                                       });
    ExpectError<std::length_error>([&]
                                   {
                                       (void)sirkit::fit_polynomial_recurrence(factorials, 1, std::numeric_limits<std::size_t>::max());
                                   });
    ExpectError<std::length_error>([&]
                                   {
                                       (void)sirkit::fit_polynomial_recurrence(factorials, 1, std::vector<std::uint32_t>().max_size());
                                   });
}
