// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/NTT.cpp"
#include "../../../Math/Multivariate_Convolution.cpp"
#include <cassert>
#include <random>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::vector<std::uint32_t> reference(const std::vector<std::size_t> &dims,
                                     const std::vector<std::uint32_t> &a,
                                     const std::vector<std::uint32_t> &b)
{
    std::vector<std::uint32_t> result(a.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; j < a.size(); ++j)
        {
            bool retained = true;
            auto x = i, y = j;
            for (const auto dimension : dims)
            {
                if (x % dimension + y % dimension >= dimension)
                {
                    retained = false;
                    break;
                }
                x /= dimension;
                y /= dimension;
            }
            if (retained)
            {
                result[i + j] = static_cast<std::uint32_t>((result[i + j] + a[i] % kMod * (b[j] % kMod)) % kMod);
            }
        }
    }
    return result;
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
    using sirkit::multivariate_convolution;
    std::mt19937 random(12972);
    for (const std::vector<std::size_t> &dims : {std::vector<std::size_t>{}, {1}, {6}, {1, 3, 4}, {2, 2, 2, 2}, {2, 3, 2, 1, 4}})
    {
        std::size_t n = 1;
        for (const auto dimension : dims)
        {
            n *= dimension;
        }
        for (int trial = 0; trial < 25; ++trial)
        {
            std::vector<std::uint32_t> a(n), b(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                a[i] = static_cast<std::uint32_t>(random());
                b[i] = static_cast<std::uint32_t>(random());
            }
            assert(multivariate_convolution(dims, a, b) == reference(dims, a, b));
        }
    }
    const std::vector<std::size_t> cube(10, 2);
    const std::vector<std::uint32_t> all(std::size_t{1} << cube.size(), kMod - 1);
    const auto answer = multivariate_convolution(cube, all, all);
    for (std::size_t i = 0; i < answer.size(); ++i)
    {
        assert(answer[i] == (std::uint32_t{1} << std::popcount(i)));
    }
    expect_error<std::invalid_argument>([]
                                        {
                                            multivariate_convolution(std::array<std::size_t, 1>{0}, {}, {});
                                        });
    expect_error<std::length_error>([]
                                    {
                                        multivariate_convolution(std::array<std::size_t, 2>{std::size_t{1} << 18, 2}, {}, {});
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([]
                                        {
                                            multivariate_convolution({}, {}, {});
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            multivariate_convolution({}, std::array<std::uint32_t, 1>{1}, {});
                                        });
#endif
}
