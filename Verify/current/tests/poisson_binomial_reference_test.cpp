#include "../../../Math/Poisson_Binomial.cpp"

#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::poisson_binomial_at_least;
using sirkit::poisson_binomial_distribution;

std::vector<double> enumerate(const std::vector<double> &probabilities)
{
    std::vector<double> expected(probabilities.size() + 1);
    for (unsigned mask = 0; mask < (1U << probabilities.size()); ++mask)
    {
        double mass = 1;
        for (std::size_t i = 0; i < probabilities.size(); ++i)
        {
            mass *= (mask & (1U << i)) ? probabilities[i] : 1.0 - probabilities[i];
        }
        expected[static_cast<std::size_t>(std::popcount(mask))] += mass;
    }
    return expected;
}

void verify(const std::vector<double> &probabilities)
{
    const auto expected = enumerate(probabilities);
    const auto actual = poisson_binomial_distribution(probabilities);
    assert(actual.size() == expected.size());
    for (std::size_t i = 0; i < actual.size(); ++i)
    {
        assert(std::abs(actual[i] - expected[i]) < 1e-13);
        const auto tail = std::accumulate(expected.begin() + static_cast<std::ptrdiff_t>(i), expected.end(), 0.0);
        assert(std::abs(poisson_binomial_at_least(probabilities, i) - tail) < 1e-13);
    }
    assert(poisson_binomial_at_least(probabilities, actual.size()) == 0);
}

template <typename Exception, typename Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    verify({});
    verify({0.0, 1.0, 0.0, 1.0, 1.0});
    verify({0.125, 0.25, 0.5, 0.75, 0.875, 0.25});
    const std::array probabilities{1.0F, 0.5F, 0.5F};
    assert(poisson_binomial_at_least(probabilities, 2) == 0.75F);
    assert((poisson_binomial_distribution(probabilities) == std::vector{0.0F, 0.25F, 0.5F, 0.25F}));

    const double tiny = std::ldexp(1.0, -65);
    const std::vector<double> rare(64, tiny);
    assert(std::abs(poisson_binomial_at_least(rare, 1) / (64 * tiny) - 1) < 1e-14);
    const std::vector<double> all_rare(71, 0.25);
    assert(std::abs(poisson_binomial_at_least(all_rare, all_rare.size()) / std::ldexp(1.0, -142) - 1) < 1e-14);

    std::mt19937 random(2999);
    for (int test = 0; test < 900; ++test)
    {
        std::vector<double> probabilities(random() % 12);
        for (auto &probability : probabilities)
        {
            probability = static_cast<double>(random() % 17) / 16.0;
        }
        verify(probabilities);
    }

    expect_exception<std::length_error>([] { (void)sirkit::detail::PoissonBinomialTableSize(4, 4); });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::invalid_argument>([] { (void)poisson_binomial_distribution(std::array{-0.1}); });
    expect_exception<std::invalid_argument>([] { (void)poisson_binomial_at_least(std::array{1.1}, 1); });
    expect_exception<std::invalid_argument>([] {
        (void)poisson_binomial_at_least(std::array{std::numeric_limits<double>::quiet_NaN()}, 0);
    });
#endif
}
