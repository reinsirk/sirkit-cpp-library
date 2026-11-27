#ifndef SIRKIT_POLYNOMIAL_PREFIX_SUM_DEFINED
#define SIRKIT_POLYNOMIAL_PREFIX_SUM_DEFINED
inline FormalPowerSeries PolynomialPrefixSum(const FormalPowerSeries &f)
{
    const auto n = f.size();
    if (n > FormalPowerSeries::max_precision())
    {
        throw std::length_error("PolynomialPrefixSum: polynomial too long");
    }
    if (n == 0)
    {
        return FormalPowerSeries{0};
    }

    reinsirk::detail::fps_prepare_inverses(n);
    const auto &inverses = reinsirk::detail::fps_integer_inverses();
    std::vector<std::uint32_t> inverse_factorials(n + 1, 1), reversed(n);
    std::uint32_t factorial = 1;
    for (std::size_t i = 0; i < n; ++i)
    {
        reversed[n - 1 - i] = reinsirk::detail::fps_multiply(f.get(i), factorial);
        factorial = reinsirk::detail::fps_multiply(factorial, static_cast<std::uint32_t>(i + 1));
        inverse_factorials[i + 1] = reinsirk::detail::fps_multiply(inverse_factorials[i], inverses[i + 1]);
    }

    // Dで微分を表すと、前進差分はexp(D)-1。g' = D/(exp(D)-1) fを畳み込む。
    std::vector<std::uint32_t> exponential(inverse_factorials.begin() + 1, inverse_factorials.end());
    const auto kernel = FormalPowerSeries(std::move(exponential)).inverse(n);
    const auto product = FormalPowerSeries(std::move(reversed)) * kernel;
    for (std::size_t i = 1; i <= n; ++i)
    {
        inverse_factorials[i] = reinsirk::detail::fps_multiply(product.get(n - i), inverse_factorials[i]);
    }
    inverse_factorials[0] = 0;
    return FormalPowerSeries(std::move(inverse_factorials));
}
#endif
