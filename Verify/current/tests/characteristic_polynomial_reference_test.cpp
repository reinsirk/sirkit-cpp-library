// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Characteristic_Polynomial.cpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>

// Leibniz展開した項を多項式として掛け合わせる。
template <std::uint32_t Mod>
void check(const std::vector<std::vector<std::int64_t>> &a)
{
    const auto n = a.size();
    std::vector<std::int64_t> expected(n + 1);
    std::vector<std::size_t> order(n);
    std::iota(order.begin(), order.end(), 0);
    do
    {
        int sign = 1;
        std::vector<std::int64_t> term{1};
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < i; ++j)
            {
                if (order[j] > order[i])
                {
                    sign = -sign;
                }
            }
            const auto remainder = a[i][order[i]] % static_cast<std::int64_t>(Mod);
            const auto x = static_cast<std::uint64_t>((remainder + Mod) % Mod);
            term.push_back(0);
            for (std::size_t j = i + 1; j > 0; --j)
            {
                const auto product = static_cast<std::int64_t>(term[j] * x % Mod);
                term[j] = ((i == order[i] ? term[j - 1] : 0) - product + Mod) % Mod;
            }
            term[0] = (Mod - term[0] * x % Mod) % Mod;
        }
        for (std::size_t k = 0; k <= n; ++k)
        {
            expected[k] = (expected[k] + sign * term[k]) % Mod;
        }
    } while (std::next_permutation(order.begin(), order.end()));
    const auto got = sirkit::characteristic_polynomial<Mod>(a);
    assert(got.size() == n + 1);
    for (std::size_t k = 0; k <= n; ++k)
    {
        assert(got[k] == static_cast<std::uint32_t>((expected[k] + Mod) % Mod));
    }
}

int main()
{
    static_assert(!sirkit::detail::characteristic_prime(0));
    static_assert(!sirkit::detail::characteristic_prime(1));
    static_assert(!sirkit::detail::characteristic_prime(9));
    static_assert(sirkit::detail::characteristic_prime(3));
    check<998244353>({});
    check<2>({{1, 0, 1}, {0, 1, 1}, {1, 1, 0}});
    check<998244353>({{0, 0, 0}, {0, 0, 0}, {0, 0, 0}});
    check<998244353>({{0, 0, 1}, {0, 1, 0}, {1, 0, 0}});
    check<998244353>({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
    check<4294967291U>({{4'294'967'290LL, 4'294'967'289LL}, {-2, -7}});
    std::mt19937 random(28468);
    for (std::size_t n = 1; n <= 6; ++n)
    {
        for (int trial = 0; trial < 80; ++trial)
        {
            std::vector<std::vector<std::int64_t>> a(n, std::vector<std::int64_t>(n));
            for (auto &row : a)
            {
                for (auto &x : row)
                {
                    x = static_cast<int>(random() % 21) - 10;
                }
            }
            check<998244353>(a);
            check<2>(a);
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<std::vector<std::int64_t>> invalid{{1, 2}};
    try
    {
        sirkit::characteristic_polynomial(invalid);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
