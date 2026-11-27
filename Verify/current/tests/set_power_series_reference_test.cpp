// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Set_Power_Series.cpp"
#include <array>
#include <cassert>
#include <random>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::uint32_t direct_product_at(const std::vector<std::uint32_t> &a,
                                const std::vector<std::uint32_t> &b, std::size_t mask)
{
    std::uint64_t result = 0;
    for (auto sub = mask;; sub = (sub - 1) & mask)
    {
        result = (result + (a[sub] % kMod) * (b[mask ^ sub] % kMod)) % kMod;
        if (sub == 0)
        {
            return static_cast<std::uint32_t>(result);
        }
    }
}

// 添字の最下位要素を含むブロックを選び、残りを小さい部分集合の答えで数える。
void check_series(const std::vector<std::uint32_t> &f, const std::vector<std::uint32_t> &g)
{
    const auto exponent = sirkit::set_series_exp(f);
    const auto logarithm = sirkit::set_series_log(g);
    assert(exponent[0] == 1 && logarithm[0] == 0);
    for (std::size_t mask = 1; mask < f.size(); ++mask)
    {
        const auto first = mask & ~(mask - 1);
        std::uint64_t e = 0, l = g[mask] % kMod;
        for (auto rest = mask ^ first;; rest = (rest - 1) & (mask ^ first))
        {
            const auto block = mask ^ rest;
            e = (e + (f[block] % kMod) * exponent[rest]) % kMod;
            if (rest == 0)
            {
                break;
            }
            l = (l + kMod - logarithm[block] * (g[rest] % kMod) % kMod) % kMod;
        }
        assert(exponent[mask] == e && logarithm[mask] == l);
    }
    const auto original_f = sirkit::set_series_log(exponent);
    const auto original_g = sirkit::set_series_exp(logarithm);
    for (std::size_t i = 0; i < f.size(); ++i)
    {
        assert(original_f[i] == f[i] % kMod && original_g[i] == g[i] % kMod);
    }
}

template <class Exception, class Operation>
void expect_error(Operation run)
{
    try
    {
        run();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
}

int main()
{
    using sirkit::set_series_exp;
    using sirkit::set_series_log;
    using sirkit::subset_convolution;
    const std::vector<std::uint32_t> zero(8);
    const auto identity = set_series_exp(zero);
    assert(identity[0] == 1 && set_series_log(identity) == zero);
    std::mt19937 random(2648);
    for (unsigned n = 0; n <= 8; ++n)
    {
        for (int trial = 0; trial < 25; ++trial)
        {
            std::vector<std::uint32_t> a(std::size_t{1} << n), b(a.size());
            for (std::size_t i = 0; i < a.size(); ++i)
            {
                a[i] = static_cast<std::uint32_t>(random());
                b[i] = static_cast<std::uint32_t>(random());
            }
            const auto actual = subset_convolution(a, b);
            for (std::size_t mask = 0; mask < a.size(); ++mask)
            {
                assert(actual[mask] == direct_product_at(a, b, mask));
            }
            a[0] = static_cast<std::uint32_t>((trial % 4) * kMod);
            b[0] = a[0] + 1;
            check_series(a, b);
        }
    }

    std::vector<std::uint32_t> a(std::size_t{1} << 17, kMod - 1), b(a.size(), kMod - 2);
    const auto product = subset_convolution(a, b);
    for (std::size_t mask = 0; mask < a.size(); ++mask)
    {
        assert(product[mask] == (2U << std::popcount(mask)) % kMod);
    }
    expect_error<std::length_error>([]
                                    {
                                        set_series_exp({});
                                    });
    expect_error<std::length_error>([]
                                    {
                                        set_series_log(std::array<std::uint32_t, 3>{});
                                    });
    expect_error<std::length_error>([]
                                    {
                                        sirkit::detail::CheckSetSeriesSize(std::size_t{1} << 21);
                                    });
    expect_error<std::domain_error>([]
                                    {
                                        set_series_exp(std::array<std::uint32_t, 1>{1});
                                    });
    expect_error<std::domain_error>([]
                                    {
                                        set_series_log(std::array<std::uint32_t, 1>{0});
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([]
                                        {
                                            subset_convolution(std::array<std::uint32_t, 1>{0}, {});
                                        });
#endif
}
