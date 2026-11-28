#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#ifndef SIRKIT_LABELED_TREE_MATCHING_COUNTS_DEFINED
#define SIRKIT_LABELED_TREE_MATCHING_COUNTS_DEFINED

[[nodiscard]] inline std::vector<std::uint32_t> labeled_tree_exposable_root_counts(std::size_t limit)
{
    detail::fps_require_precision(limit);
    std::vector<std::uint32_t> result(limit + 1);
    if (limit == 0)
    {
        return result;
    }

    detail::fps_prepare_inverses(limit - 1);
    const auto &inverses = detail::fps_integer_inverses();
    FormalPowerSeries rooted(limit);
    std::uint32_t inverse_factorial = 1;
    for (std::size_t n = 1; n < limit; ++n)
    {
        inverse_factorial = detail::fps_multiply(inverse_factorial, inverses[n]);
        rooted.set(n, detail::fps_multiply(detail::fps_power(static_cast<std::uint32_t>(n), n - 1), inverse_factorial));
    }

    // L は開始手番が負ける根付き木のEGF。L=x exp(T-L) と T=x exp(T) を使う。
    // U=L/x に対する log(U)+xU=T のNewton法。分母 (1+xU)/U の定数項は1。
    FormalPowerSeries scaled{1};
    for (std::size_t known = 1; known < limit;)
    {
        const auto target = std::min(known * 2, limit);
        std::vector<std::uint32_t> shifted_coefficients(target);
        std::copy(scaled.coefficients().begin(), scaled.coefficients().end(), shifted_coefficients.begin() + 1);
        const FormalPowerSeries shifted(std::move(shifted_coefficients));
        auto residual = rooted.prefix(target) - shifted - scaled.log(target);
        auto denominator = shifted;
        denominator.set(0, 1);
        residual = (residual * scaled).prefix(target);
        scaled += (residual * denominator.inverse(target)).prefix(target);
        known = target;
    }

    std::uint32_t factorial = 1;
    for (std::size_t n = 1; n <= limit; ++n)
    {
        result[n] = detail::fps_multiply(scaled[n - 1], factorial);
        factorial = detail::fps_multiply(factorial, static_cast<std::uint32_t>(n));
    }
    return result;
}

[[nodiscard]] inline std::vector<std::uint32_t> labeled_tree_perfect_matching_counts(std::size_t limit)
{
    detail::fps_require_precision(limit);
    std::vector<std::uint32_t> result(limit + 1);
    std::uint32_t pairings = 1, endpoint_choices = 1;
    for (std::size_t pairs = 1; pairs <= limit / 2; ++pairs)
    {
        pairings = detail::fps_multiply(pairings, static_cast<std::uint32_t>(pairs * 2 - 1));
        const auto contracted_trees = detail::fps_power(static_cast<std::uint32_t>(pairs), pairs > 2 ? pairs - 2 : 0);
        result[pairs * 2] = detail::fps_multiply(detail::fps_multiply(pairings, endpoint_choices), contracted_trees);
        endpoint_choices = detail::fps_multiply(endpoint_choices, 4);
    }
    return result;
}

#endif
