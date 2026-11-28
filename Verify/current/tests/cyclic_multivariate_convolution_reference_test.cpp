// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Cyclic_Multivariate_Convolution.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

static std::size_t add_indices(std::size_t left, std::size_t right, const std::vector<std::size_t> &dimensions)
{
    std::size_t result = 0, stride = 1;
    for (const auto dimension : dimensions)
    {
        const auto coordinate = (left % dimension + right % dimension) % dimension;
        result += stride * coordinate;
        stride *= dimension;
        left /= dimension;
        right /= dimension;
    }
    return result;
}

static void check(const std::vector<std::size_t> &dimensions, std::uint32_t prime)
{
    const auto size = std::accumulate(dimensions.begin(), dimensions.end(), std::size_t{1}, std::multiplies{});
    std::mt19937 rng(prime);
    std::vector<std::uint32_t> a(size), b(size), expected(size);
    for (auto &entry : a)
    {
        entry = rng();
    }
    for (auto &entry : b)
    {
        entry = rng();
    }
    for (std::size_t i = 0; i < size; ++i)
    {
        for (std::size_t j = 0; j < size; ++j)
        {
            const auto coefficient = std::uint64_t{a[i]} * b[j] % prime;
            auto &entry = expected[add_indices(i, j, dimensions)];
            entry = static_cast<std::uint32_t>((entry + coefficient) % prime);
        }
    }
    assert(sirkit::cyclic_multivariate_convolution(dimensions, a, b, prime) == expected);
    assert(sirkit::cyclic_multivariate_convolution(dimensions, b, a, prime) == expected);
}

template <class Error, class Test>
static void check_invalid(const Test &test)
{
    bool thrown = false;
    try
    {
        test();
    }
    catch (const Error &)
    {
        thrown = true;
    }
    assert(thrown);
}

int main()
{
    check({}, 2);
    check({1, 1, 1}, 17);
    check({1, 2, 4}, 17);
    check({2, 8, 2, 2}, 17);
    check({32, 3}, 97);
    check({17, 2, 3}, 103);
    check({34, 3}, 103);
    check({97, 2}, 389);
    check({7, 64}, 449);
    check({119}, 998244353);
    check({2, 5}, 4294967291U);
    const auto max = std::numeric_limits<std::uint32_t>::max();
    const std::vector<std::uint32_t> a{max, max};
    const std::vector<std::size_t> dimension{2};
    assert((sirkit::cyclic_multivariate_convolution(dimension, a, a, 3) == std::vector<std::uint32_t>{0, 0}));

    check_invalid<std::domain_error>([]
                                     {
                                         (void)sirkit::cyclic_multivariate_convolution({}, {}, {}, 4);
                                     });
    check_invalid<std::invalid_argument>([&]
                                         {
                                             (void)sirkit::cyclic_multivariate_convolution(dimension, {}, a, 3);
                                         });
    check_invalid<std::invalid_argument>([&]
                                         {
                                             (void)sirkit::cyclic_multivariate_convolution(dimension, a, {}, 3);
                                         });
    check_invalid<std::invalid_argument>([&]
                                         {
                                             (void)sirkit::cyclic_multivariate_convolution(dimension, a, a, 2);
                                         });
    check_invalid<std::invalid_argument>([]
                                         {
                                             const std::vector<std::size_t> dimensions{0};
                                             (void)sirkit::cyclic_multivariate_convolution(dimensions, {}, {}, 17);
                                         });
    check_invalid<std::length_error>([]
                                     {
                                         const std::vector<std::size_t> dimensions{(std::size_t{1} << 21) + 1};
                                         (void)sirkit::cyclic_multivariate_convolution(dimensions, {}, {}, 17);
                                     });
}
