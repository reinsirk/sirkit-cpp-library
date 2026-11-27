#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_MULTIVARIATE_CONVOLUTION_DEFINED
#define SIRKIT_MULTIVARIATE_CONVOLUTION_DEFINED
inline std::vector<std::uint32_t> multivariate_convolution(std::span<const std::size_t> dimensions,
                                                           std::span<const std::uint32_t> a,
                                                           std::span<const std::uint32_t> b)
{
    constexpr std::size_t kMaxDimensions = 18;
    constexpr std::size_t kMaxSize = std::size_t{1} << kMaxDimensions;
    constexpr std::uint64_t kMod = NTT<>::modulus();
    std::array<std::size_t, kMaxDimensions> strides;
    std::size_t n = 1, axes = 0;
    for (const auto dimension : dimensions)
    {
        if (dimension == 0)
        {
            throw std::invalid_argument("multivariate_convolution: zero dimension");
        }
        if (dimension > kMaxSize / n)
        {
            throw std::length_error("multivariate_convolution: product exceeds 2^18");
        }
        if (dimension > 1)
        {
            strides[axes++] = n;
            n *= dimension;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (a.size() != n || b.size() != n)
    {
        throw std::invalid_argument("multivariate_convolution: sizes do not match dimensions");
    }
#endif
    const auto groups = std::max(std::size_t{1}, axes);
    const auto length = std::bit_ceil(2 * n - 1);
    using Coefficients = std::vector<std::uint32_t>;
    std::vector<Coefficients> left(groups, Coefficients(length)), right(groups, Coefficients(length));
    std::vector<std::uint8_t> color(n);
    // 各桁境界に対するfloor(i/stride)の和を色とする。和の色のずれが繰り上がり回数になる。
    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t sum = 0;
        for (std::size_t dimension = 1; dimension < axes; ++dimension)
        {
            sum += i / strides[dimension];
        }
        const auto at = sum % groups;
        color[i] = static_cast<std::uint8_t>(at);
        left[at][i] = static_cast<std::uint32_t>(a[i] % kMod);
        right[at][i] = static_cast<std::uint32_t>(b[i] % kMod);
    }
    NTT<> ntt;
    for (std::size_t c = 0; c < groups; ++c)
    {
        ntt.transform(left[c]);
        ntt.transform(right[c]);
    }
    for (std::size_t i = 0; i < length; ++i)
    {
        std::array<std::uint64_t, kMaxDimensions> product{};
        for (std::size_t c = 0; c < groups; ++c)
        {
            const std::uint64_t x = left[c][i];
            for (std::size_t d = 0; d < groups; ++d)
            {
                const auto idx = c + d < groups ? c + d : c + d - groups;
                product[idx] += x * right[d][i];
            }
        }
        // 各色への寄与は18項以下。1項が(mod-1)^2なので合計はuint64_tに収まる。
        for (std::size_t c = 0; c < groups; ++c)
        {
            left[c][i] = static_cast<std::uint32_t>(product[c] % kMod);
        }
    }
    Coefficients result(n);
    for (std::size_t c = 0; c < groups; ++c)
    {
        ntt.transform(left[c], true);
        for (std::size_t i = 0; i < n; ++i)
        {
            if (color[i] == c)
            {
                result[i] = left[c][i];
            }
        }
    }
    return result;
}
#endif
