// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/NTT.cpp"
#include "../../../Math/Multiplicative_Convolution.cpp"
#include <array>
#include <cassert>
#include <random>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::vector<std::uint32_t> reference(const std::vector<std::uint32_t> &a,
                                     const std::vector<std::uint32_t> &b)
{
    const auto n = a.size();
    std::vector<std::uint32_t> c(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            const auto index = i * j % n;
            c[index] = static_cast<std::uint32_t>((c[index] + a[i] % kMod * (b[j] % kMod)) % kMod);
        }
    }
    return c;
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
    using sirkit::multiplicative_convolution_power_of_two;
    using sirkit::multiplicative_convolution_prime;
    std::mt19937 random(92833);
    for (const auto n : {2U, 3U, 5U, 13U, 29U, 101U, 257U, 769U})
    {
        for (int trial = 0; trial < 5; ++trial)
        {
            std::vector<std::uint32_t> a(n), b(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                a[i] = static_cast<std::uint32_t>(random());
                b[i] = static_cast<std::uint32_t>(random());
            }
            assert(multiplicative_convolution_prime(a, b) == reference(a, b));
        }
    }
    for (std::size_t n = 1; n <= 256; n *= 2)
    {
        for (int trial = 0; trial < 15; ++trial)
        {
            std::vector<std::uint32_t> a(n), b(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                a[i] = static_cast<std::uint32_t>(random());
                b[i] = static_cast<std::uint32_t>(random());
            }
            assert(multiplicative_convolution_power_of_two(a, b) == reference(a, b));
        }
    }
    const std::vector<std::uint32_t> dense(std::size_t{1} << 14, 1);
    std::vector<std::uint32_t> point(dense.size());
    point[12] = 7;
    const auto product = multiplicative_convolution_power_of_two(dense, point);
    for (std::size_t k = 0; k < product.size(); ++k)
    {
        assert(product[k] == (k % 4 == 0 ? 28U : 0U));
    }
    expect_error<std::length_error>([]
                                    {
                                        multiplicative_convolution_prime({}, {});
                                    });
    std::vector<std::uint32_t> too_large(std::size_t{1} << 21);
    expect_error<std::length_error>([&]
                                    {
                                        multiplicative_convolution_prime(too_large, too_large);
                                    });
    expect_error<std::length_error>([&]
                                    {
                                        multiplicative_convolution_power_of_two(too_large, too_large);
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array<std::uint32_t, 4> a{};
                                            multiplicative_convolution_prime(a, a);
                                        });
    expect_error<std::length_error>([]
                                    {
                                        const std::array<std::uint32_t, 3> a{};
                                        multiplicative_convolution_power_of_two(a, a);
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([]
                                        {
                                            multiplicative_convolution_power_of_two({}, std::array<std::uint32_t, 1>{0});
                                        });
#endif
}
