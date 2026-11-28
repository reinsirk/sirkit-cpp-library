// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Set_Series_Composition.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Vector = std::vector<std::uint32_t>;

void AddProduct(std::uint32_t &sum, const std::uint32_t a, const std::uint32_t b)
{
    sum = static_cast<std::uint32_t>((sum + std::uint64_t{a} * b) % kMod);
}

Vector NaiveSubsetProduct(const Vector &a, const Vector &b)
{
    Vector result(a.size());
    for (std::size_t set = 0; set < a.size(); ++set)
    {
        auto sub = set;
        while (true)
        {
            AddProduct(result[set], a[sub], b[set ^ sub]);
            if (sub == 0)
            {
                break;
            }
            sub = (sub - 1) & set;
        }
    }
    return result;
}

void Compare(const Vector &polynomial, const Vector &series, const Vector &weights)
{
    Vector power(series.size()), sum(series.size()), projection(polynomial.size());
    power[0] = 1;
    for (std::size_t e = 0; e < polynomial.size(); ++e)
    {
        for (std::size_t set = 0; set < series.size(); ++set)
        {
            AddProduct(sum[set], polynomial[e], power[set]);
            AddProduct(projection[e], weights[set], power[set]);
        }
        power = NaiveSubsetProduct(power, series);
    }
    assert(sirkit::set_series_compose(polynomial, series) == sum);
    assert(sirkit::set_series_power_projection(series, weights, polynomial.size()) == projection);
    std::uint32_t dual_sum{}, weighted_sum{};
    for (std::size_t e = 0; e < polynomial.size(); ++e)
    {
        AddProduct(dual_sum, polynomial[e], projection[e]);
    }
    for (std::size_t set = 0; set < series.size(); ++set)
    {
        AddProduct(weighted_sum, weights[set], sum[set]);
    }
    assert(dual_sum == weighted_sum);
}
}

int main()
{
    using sirkit::set_series_compose;
    using sirkit::set_series_power_projection;
    assert((set_series_compose(Vector{1, 2, 3}, Vector{2, 3, 5, 7}) == Vector{17, 42, 70, 188}));
    assert((set_series_power_projection(Vector{2, 3, 5, 7}, Vector{7, 11, 13, 17}, 3) == Vector{7, 231, 1406}));
    try
    {
        static_cast<void>(set_series_compose(Vector{1}, Vector{1, 2, 3}));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    try
    {
        static_cast<void>(set_series_power_projection(Vector{1, 2}, Vector{1}, 1));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }

    std::mt19937 random(92641);
    for (const std::size_t n : {0, 1, 2, 3, 4, 5, 8, 9})
    {
        Vector series(std::size_t{1} << n), weights(series.size());
        for (const auto degree : {0, 1, 2, 3, 5, 12, 20})
        {
            for (auto &value : series)
            {
                value = static_cast<std::uint32_t>(random() % kMod);
            }
            for (auto &weight : weights)
            {
                weight = static_cast<std::uint32_t>(random());
            }
            if (degree % 2 == 0)
            {
                series[0] = 0;
            }
            Vector polynomial(degree);
            for (auto &coefficient : polynomial)
            {
                coefficient = static_cast<std::uint32_t>(random());
            }
            Compare(polynomial, series, weights);
        }
    }
}
