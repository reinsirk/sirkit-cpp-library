// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/NTT.cpp"
// 多項式評価と同じプログラムに貼り付けても内部の名前が衝突しないことを検査する。
#include "../../../Math/Consecutive_Polynomial_Evaluation.cpp"
#ifndef SIRKIT_CONSECUTIVE_LINEAR_RECURRENCE_DEFINED
#include "../../../Math/Consecutive_Linear_Recurrence.cpp"
#endif
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Coefficients = std::vector<std::uint32_t>;
using Matrix = std::vector<Coefficients>;
constexpr std::uint64_t kMod = 998244353;

void append(Coefficients &values, const Coefficients &coefficients)
{
    std::uint64_t term = 0;
    for (std::size_t i = 0; i < coefficients.size(); ++i)
    {
        term = (term + std::uint64_t{coefficients[i]} * values[values.size() - i - 1]) % kMod;
    }
    values.push_back(static_cast<std::uint32_t>(term));
}

Coefficients slow(Coefficients values, const Coefficients &coefficients, std::size_t first, std::size_t count)
{
    for (auto &value : values)
    {
        value %= kMod;
    }
    while (values.size() < first + count)
    {
        append(values, coefficients);
    }
    return {values.begin() + first, values.begin() + first + count};
}

Matrix multiply(const Matrix &left, const Matrix &right)
{
    const auto size = left.size();
    Matrix product(size, Coefficients(size));
    for (std::size_t i = 0; i < size; ++i)
    {
        for (std::size_t j = 0; j < size; ++j)
        {
            for (std::size_t k = 0; k < size; ++k)
            {
                product[i][j] = static_cast<std::uint32_t>(
                    (product[i][j] + std::uint64_t{left[i][k]} * right[k][j]) % kMod);
            }
        }
    }
    return product;
}

Coefficients matrix_reference(const Coefficients &initial, const Coefficients &coefficients,
                              std::uint64_t first, std::size_t count)
{
    const auto size = initial.size();
    Matrix state(size, Coefficients(size)), power(size, Coefficients(size));
    for (std::size_t i = 0; i + 1 < size; ++i)
    {
        power[i][i + 1] = 1;
    }
    for (std::size_t i = 0; i < size; ++i)
    {
        state[i][0] = initial[i] % kMod;
        power[size - 1][i] = coefficients[size - 1 - i] % kMod;
    }
    while (first != 0)
    {
        if (first % 2 == 1)
        {
            state = multiply(power, state);
        }
        power = multiply(power, power);
        first /= 2;
    }
    Coefficients result;
    for (const auto &row : state)
    {
        result.push_back(row[0]);
    }
    while (result.size() < count)
    {
        append(result, coefficients);
    }
    result.resize(count);
    return result;
}

template <class Error, class Func>
void check_error(Func call)
{
    try
    {
        call();
    }
    catch (const Error &)
    {
        return;
    }
    assert(false);
}
} // namespace

int main(int argc, char **)
{
    using sirkit::consecutive_linear_recurrence;
    constexpr auto kLast = std::numeric_limits<std::uint64_t>::max();
    assert(consecutive_linear_recurrence({}, {}, kLast, 0).empty());
    assert(consecutive_linear_recurrence({}, {}, kLast - 4, 5) == Coefficients(5));

#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::invalid_argument>([]
                                       {
                                           consecutive_linear_recurrence(Coefficients{1}, {}, 0, 1);
                                       });
#endif
    check_error<std::overflow_error>([]
                                     {
                                         consecutive_linear_recurrence({}, {}, kLast, 2);
                                     });
    check_error<std::length_error>([]
                                   {
                                       consecutive_linear_recurrence({}, {}, 0, (std::size_t{1} << 22) + 1);
                                   });
    check_error<std::length_error>([]
                                   {
                                       sirkit::detail::CheckConsecutiveRecurrenceSize((std::size_t{1} << 22) + 1, 1);
                                   });

    const Coefficients initial{7, 11, 99}, zero(3), short_order{2, 0, 0};
    assert(consecutive_linear_recurrence(initial, zero, 0, 6) == (Coefficients{7, 11, 99, 0, 0, 0}));
    assert(consecutive_linear_recurrence(initial, zero, 6, 2) == (Coefficients{0, 0}));
    assert(consecutive_linear_recurrence(initial, short_order, 1, 5) == (Coefficients{11, 99, 198, 396, 792}));

    std::mt19937_64 rng(98367126);
    for (std::size_t iteration = 0; iteration < 1500; ++iteration)
    {
        const auto degree = static_cast<std::size_t>(1 + rng() % 25);
        const auto sparse_degree = static_cast<std::size_t>(rng() % (degree + 1));
        Coefficients a(degree), c(degree);
        for (auto &value : a)
        {
            value = static_cast<std::uint32_t>(rng());
        }
        for (std::size_t j = 0; j < sparse_degree; ++j)
        {
            c[j] = static_cast<std::uint32_t>(rng());
        }
        const auto begin = static_cast<std::size_t>(rng() % (iteration % 3 == 0 ? 30 : 1500));
        const auto count = static_cast<std::size_t>(rng() % 60);
        assert(consecutive_linear_recurrence(a, c, begin, count) == slow(a, c, begin, count));
    }

    for (std::size_t iteration = 0; iteration < 90; ++iteration)
    {
        const auto degree = static_cast<std::size_t>(1 + rng() % 7);
        Coefficients a(degree), c(degree);
        for (std::size_t j = 0; j < degree; ++j)
        {
            a[j] = static_cast<std::uint32_t>(rng());
            c[j] = static_cast<std::uint32_t>(rng());
        }
        const auto count = 1 + iteration % 19;
        const auto first = iteration % 3 == 0 ? kLast - (count - 1) : rng() % (kLast - 18);
        assert(consecutive_linear_recurrence(a, c, first, count) == matrix_reference(a, c, first, count));
    }

    if (argc > 1)
    {
        constexpr std::size_t kDegree = 100000, kCount = 500000;
        constexpr std::uint64_t kFirst = 1000000000000000000ULL - 1;
        Coefficients period(kDegree), copy_period(kDegree);
        for (auto &value : period)
        {
            value = static_cast<std::uint32_t>(rng() % kMod);
        }
        copy_period.back() = 1;
        const auto values = consecutive_linear_recurrence(period, copy_period, kFirst, kCount);
        for (std::size_t j = 0; j < kCount; ++j)
        {
            assert(values[j] == period[(kFirst % kDegree + j) % kDegree]);
        }
    }

    if (argc > 2)
    {
        // この区間のNewton更新では、下位と高位をまとめて畳み込むとNTT上限を超える。
        constexpr std::size_t kDegree = (std::size_t{1} << 21) + 2;
        Coefficients period(kDegree), copy_period(kDegree);
        period.back() = 923;
        copy_period.back() = 1;
        assert(consecutive_linear_recurrence(period, copy_period, 2 * kDegree - 1, 1) == Coefficients{923});
    }
}
