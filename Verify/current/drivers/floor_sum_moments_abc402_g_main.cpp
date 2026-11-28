// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <utility>

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::uint32_t n{}, m{};
        std::uint64_t a{}, b1{}, b2{};
        verify_in >> n >> m >> a >> b1 >> b2;
        if (b1 > b2)
        {
            std::swap(b1, b2);
        }
        const auto lo = sirkit::floor_sum_moments(n, m, a, b1);
        const auto hi = sirkit::floor_sum_moments(n, m, a, b2);
        const auto sum_i = static_cast<std::uint64_t>(n) * (n - 1) / 2;
        const auto sum_i2 = sum_i * (2 * static_cast<std::uint64_t>(n) - 1) / 3;
        const auto sum_r = a * sum_i + b1 * n - m * lo.sum;
        const auto sum_r2 = a * a * sum_i2 + 2 * a * b1 * sum_i + b1 * b1 * n -
                            (2 * std::uint64_t{m}) * (a * lo.index_weighted + b1 * lo.sum) +
                            std::uint64_t{m} * m * lo.sum_of_squares();
        // q2-q1が1の場合だけr2=r1+b2-b1-mとなる。
        const auto wrapped = a * (hi.index_weighted - lo.index_weighted) +
                             b1 * (hi.sum - lo.sum) -
                             m * (hi.pairs - lo.pairs);
        verify_out << sum_r2 + (b2 - b1) * sum_r - m * wrapped << '\n';
    }
}
