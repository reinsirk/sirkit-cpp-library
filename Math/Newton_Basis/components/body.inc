#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_NEWTON_BASIS_DEFINED
#define SIRKIT_NEWTON_BASIS_DEFINED
namespace reinsirk::detail
{
class NewtonBasisProducts
{
    static constexpr std::size_t kBlockSize = 24;
    static constexpr std::uint32_t kMod = 998244353;

    std::vector<std::uint32_t> points_;
    std::vector<FormalPowerSeries> products_;

    // 小区間は、(x-point) の掛け算を1係数ずつ行う。
    static void multiply_linear(std::vector<std::uint32_t> &f, std::uint32_t point)
    {
        f.push_back(0);
        for (auto j = f.size() - 1; j != 0; --j)
        {
            f[j] = subtract_product(f[j - 1], f[j], point);
        }
        f[0] = subtract_product(0, f[0], point);
    }

    static std::uint32_t subtract_product(std::uint32_t a, std::uint32_t b, std::uint32_t c)
    {
        const auto product = static_cast<std::uint32_t>(std::uint64_t{b} * c % kMod);
        return a >= product ? a - product : a + kMod - product;
    }

    void build(std::size_t node, std::size_t begin, std::size_t end, bool save_product)
    {
        if (end - begin <= kBlockSize)
        {
            if (save_product)
            {
                std::vector<std::uint32_t> product{1};
                product.reserve(end - begin + 1);
                for (auto i = begin; i < end; ++i)
                {
                    multiply_linear(product, points_[i]);
                }
                products_[node] = FormalPowerSeries(std::move(product));
            }
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        build(node * 2, begin, mid, true);
        build(node * 2 + 1, mid, end, save_product);
        if (save_product)
        {
            products_[node] = products_[node * 2] * products_[node * 2 + 1];
        }
    }

    void decompose(const FormalPowerSeries &f, std::size_t node, std::size_t begin, std::span<std::uint32_t> out) const
    {
        if (out.size() <= kBlockSize)
        {
            std::copy(f.coefficients().begin(), f.coefficients().end(), out.begin());
            // 定数項から、(x-point) による組立除法を繰り返す。
            for (std::size_t i = 0; i + 1 < out.size(); ++i)
            {
                const auto point = points_[begin + i];
                for (auto j = out.size() - 1; j > i; --j)
                {
                    out[j - 1] = static_cast<std::uint32_t>((out[j - 1] + std::uint64_t{point} * out[j]) % kMod);
                }
            }
            return;
        }
        const auto split = out.size() / 2;
        const auto [right, left] = f.divmod(products_[node * 2]);
        decompose(left, node * 2, begin, out.first(split));
        decompose(right, node * 2 + 1, begin + split, out.subspan(split));
    }

    FormalPowerSeries compose(std::span<const std::uint32_t> coefficients, std::size_t node, std::size_t begin) const
    {
        if (coefficients.size() <= kBlockSize)
        {
            std::vector<std::uint32_t> f{coefficients.back() % kMod};
            f.reserve(coefficients.size());
            for (auto i = coefficients.size() - 1; i != 0; --i)
            {
                multiply_linear(f, points_[begin + i - 1]);
                f[0] += coefficients[i - 1] % kMod;
                if (f[0] >= kMod)
                {
                    f[0] -= kMod;
                }
            }
            return FormalPowerSeries(std::move(f));
        }
        const auto split = coefficients.size() / 2;
        return compose(coefficients.first(split), node * 2, begin) +
               products_[node * 2] * compose(coefficients.subspan(split), node * 2 + 1, begin + split);
    }

public:
    NewtonBasisProducts(std::size_t n, std::span<const std::uint32_t> points)
        : points_(points.begin(), points.end()), products_(n <= kBlockSize ? 0 : 4 * (n / kBlockSize + 1))
    {
        for (auto &point : points_)
        {
            point %= kMod;
        }
        build(1, 0, n, false);
    }

    [[nodiscard]] std::vector<std::uint32_t> from_monomials(std::span<const std::uint32_t> coefficients) const
    {
        std::vector<std::uint32_t> result(coefficients.size());
        decompose(FormalPowerSeries(std::vector<std::uint32_t>(coefficients.begin(), coefficients.end())), 1, 0, result);
        return result;
    }

    [[nodiscard]] std::vector<std::uint32_t> to_monomials(std::span<const std::uint32_t> coefficients) const
    {
        auto result = compose(coefficients, 1, 0);
        result.resize(coefficients.size());
        return result.coefficients();
    }
};

inline void validate_newton_basis_sizes(std::size_t n, std::size_t points)
{
    if (n > (std::size_t{1} << 20))
    {
        throw std::length_error("Newton basis: too many coefficients");
    }
    if (points != (n == 0 ? 0 : n - 1))
    {
        throw std::invalid_argument("Newton basis: expected one fewer point than coefficients");
    }
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<std::uint32_t> monomial_to_newton_basis(
    std::span<const std::uint32_t> coefficients, std::span<const std::uint32_t> points)
{
    ::reinsirk::detail::validate_newton_basis_sizes(coefficients.size(), points.size());
    if (coefficients.empty())
    {
        return {};
    }
    return ::reinsirk::detail::NewtonBasisProducts(coefficients.size(), points).from_monomials(coefficients);
}

[[nodiscard]] inline std::vector<std::uint32_t> newton_to_monomial_basis(
    std::span<const std::uint32_t> coefficients, std::span<const std::uint32_t> points)
{
    ::reinsirk::detail::validate_newton_basis_sizes(coefficients.size(), points.size());
    if (coefficients.empty())
    {
        return {};
    }
    return ::reinsirk::detail::NewtonBasisProducts(coefficients.size(), points).to_monomials(coefficients);
}
#endif
