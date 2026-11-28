// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Prefix = std::array<std::int64_t, 8>;

Prefix Product(const Prefix &left, const Prefix &right)
{
    Prefix product{};
    for (std::size_t i = 0; i < product.size(); ++i)
    {
        for (std::size_t j = 0; j <= i; ++j)
        {
            product[i] += left[j] * right[i - j] % kMod;
        }
        product[i] %= kMod;
    }
    return product;
}

Prefix Training(std::uint32_t exponent)
{
    Prefix base{1, 3, 1}, one{1};
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            one = Product(one, base);
        }
        base = Product(base, base);
    }
    return one;
}
} // namespace

int main()
{
    std::uint32_t n{};
    verify_in >> n;
    const auto training = Training(2 * n);
    // 初めの6式の5x5小行列式2つの共通零点はm=0,1,-1のみ（m=2n）。
    // 制約内では核が1次元なので、微分式から得られる漸化式が定数倍まで決まる。
    const auto coefficients = sirkit::fit_polynomial_recurrence(training, 2, 1).value();
    const auto coefficient = sirkit::polynomial_recurrence_term(std::span(training).first(2), coefficients, n - 1);
    verify_out << sirkit::detail::ConsecutiveMultiply(coefficient, sirkit::detail::ConsecutiveInverse(n)) << '\n';
}
