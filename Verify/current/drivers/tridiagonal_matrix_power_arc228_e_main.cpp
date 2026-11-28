// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

std::uint32_t Product(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kMod);
}

std::uint32_t Power(std::uint32_t base, std::uint32_t exponent)
{
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = Product(result, base);
        }
        base = Product(base, base);
    }
    return result;
}
} // namespace

int main()
{
    std::uint64_t n{}, width{};
    verify_in >> n >> width;
    const auto m = static_cast<std::size_t>(std::min(n, width));
    std::vector<std::int64_t> diagonal(m + 1), lower(m), upper(m);
    for (std::size_t i = 0; i < m; ++i)
    {
        const auto remaining = static_cast<std::int64_t>(m - i);
        const auto chosen = static_cast<std::int64_t>(i);
        lower[i] = remaining * (remaining + 1);
        upper[i] = i == 0 ? 1 : (chosen + 1) * (chosen + 1);
        diagonal[i + 1] = 2 * (chosen + 1) * remaining;
    }
    const auto current = sirkit::tridiagonal_power_first_column(n - m, lower, diagonal, upper);

    std::vector<std::uint32_t> factorial(m + 1, 1), inverse_factorial(m + 1);
    for (std::size_t i = 2; i <= m; ++i)
    {
        factorial[i] = Product(factorial[i - 1], static_cast<std::uint32_t>(i));
    }
    inverse_factorial[m] = Power(factorial[m], kMod - 2);
    for (auto i = m; i != 0; --i)
    {
        inverse_factorial[i - 1] = Product(inverse_factorial[i], static_cast<std::uint32_t>(i));
    }
    std::vector<std::uint32_t> first(m + 1);
    for (std::size_t i = 0; i <= m; ++i)
    {
        const auto value = Product(factorial[m - i], inverse_factorial[i]);
        first[i] = (i & 1u) == 0 ? value : kMod - value;
    }
    sirkit::NTT<> convolution;
    const auto tail = convolution.convolution_normalized(std::move(first), std::move(inverse_factorial));
    std::uint32_t total = 0;
    for (std::size_t j = 0; j <= m; ++j)
    {
        total += Product(Product(current[j], factorial[m - j]), tail[m - j]);
        if (total >= kMod)
        {
            total -= kMod;
        }
    }
    verify_out << Product(total, factorial[m]) << '\n';
}
