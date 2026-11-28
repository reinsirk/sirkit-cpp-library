// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

std::uint32_t Product(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kMod);
}

std::uint32_t Inverse(std::uint32_t base)
{
    std::uint32_t result = 1;
    for (auto power = kMod - 2; power != 0; power >>= 1)
    {
        if ((power & 1u) != 0)
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
    std::int64_t n{};
    verify_in >> n;
    const auto square = n * n;
    const std::vector<std::vector<std::vector<std::int64_t>>> transition{
        {{0, n, -1}, {square, -n}, {0, n}},
        {{square, -n}, {0, n, -1}, {0, 0, 1}},
        {{}, {}, {square, 0, -1}}};
    const auto product = sirkit::polynomial_matrix_interval_product(transition, 0, static_cast<std::uint64_t>(n));
    const auto scale = Inverse(product[2][2]);
    verify_out << Product(product[0][2], scale) << ' ' << Product(product[1][2], scale) << '\n';
}
