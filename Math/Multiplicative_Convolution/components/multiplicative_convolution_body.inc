#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_MULTIPLICATIVE_CONVOLUTION_DEFINED
#define SIRKIT_MULTIPLICATIVE_CONVOLUTION_DEFINED
namespace reinsirk::detail
{
using ProductCoefficients = std::vector<std::uint32_t>;
inline constexpr std::uint32_t kProductMod = NTT<>::modulus();
inline constexpr unsigned kProductMaxPower = 20;

inline std::uint32_t ProductAdd(std::uint32_t a, std::uint32_t b)
{
    const auto sum = a + b;
    return sum < kProductMod ? sum : sum - kProductMod;
}

inline std::uint32_t ProductDifference(std::uint32_t a, std::uint32_t b)
{
    return a >= b ? a - b : a + kProductMod - b;
}

inline std::uint32_t ProductScale(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kProductMod);
}

inline std::uint32_t ProductPow(std::uint32_t base, std::uint32_t exponent, std::uint32_t prime)
{
    std::uint32_t result = 1;
    while (exponent > 0)
    {
        if (exponent % 2 != 0)
        {
            result = static_cast<std::uint32_t>(std::uint64_t{result} * base % prime);
        }
        base = static_cast<std::uint32_t>(std::uint64_t{base} * base % prime);
        exponent /= 2;
    }
    return result;
}

inline std::uint32_t ProductPrimitiveRoot(std::uint32_t prime)
{
    std::vector<std::uint32_t> divisors;
    auto unfactored = prime - 1;
    for (std::uint32_t d = 2; d <= unfactored / d; ++d)
    {
        if (unfactored % d == 0)
        {
            divisors.push_back(d);
            do
            {
                unfactored /= d;
            } while (unfactored % d == 0);
        }
    }
    if (unfactored > 1)
    {
        divisors.push_back(unfactored);
    }
    for (std::uint32_t root = 1;; ++root)
    {
        if (std::none_of(divisors.begin(), divisors.end(), [&](std::uint32_t d)
                         {
                             return ProductPow(root, (prime - 1) / d, prime) == 1;
                         }))
        {
            return root;
        }
    }
}

inline void ProductCheckSizes(std::size_t a, std::size_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (a != b)
    {
        throw std::invalid_argument("multiplicative convolution: size mismatch");
    }
#else
    static_cast<void>(a);
    static_cast<void>(b);
#endif
}

struct ProductUnitLayer
{
    ProductCoefficients sum;
    ProductCoefficients difference;
};

struct ProductUnitProjection
{
    std::array<ProductUnitLayer, kProductMaxPower> layers;
    std::array<std::uint32_t, kProductMaxPower + 1> mass{};
};

inline ProductUnitProjection ProductSplitPowers(std::span<const std::uint32_t> values, unsigned exponent)
{
    ProductUnitProjection result;
    result.mass[exponent] = values[0] % kProductMod;
    if (exponent == 0)
    {
        return result;
    }
    result.mass[exponent - 1] = values[values.size() / 2] % kProductMod;
    for (unsigned v = 0; v + 1 < exponent; ++v)
    {
        const auto modulus = values.size() >> v;
        const auto count = modulus / 4;
        auto &layer = result.layers[v];
        layer.sum.resize(count);
        layer.difference.resize(count);
        std::size_t unit = 1;
        for (std::size_t k = 0; k < count; ++k)
        {
            const auto plus = values[unit << v] % kProductMod;
            const auto minus = values[(modulus - unit) << v] % kProductMod;
            layer.sum[k] = ProductAdd(plus, minus);
            layer.difference[k] = ProductDifference(plus, minus);
            result.mass[v] = ProductAdd(result.mass[v], layer.sum[k]);
            unit = unit * 5 % modulus;
        }
    }
    return result;
}

inline void ProductFoldUnits(ProductCoefficients &values, std::size_t length)
{
    for (std::size_t i = 0; i < length; ++i)
    {
        values[i] = ProductAdd(values[i], values[i + length]);
    }
    values.resize(length);
}

inline void ProductAccumulateCyclic(NTT<> &ntt, ProductCoefficients a, ProductCoefficients b,
                                    ProductCoefficients &result)
{
    ntt.transform(a);
    ntt.transform(b);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = ProductAdd(result[i], ProductScale(a[i], b[i]));
    }
}
}

inline std::vector<std::uint32_t> multiplicative_convolution_prime(std::span<const std::uint32_t> a,
                                                                   std::span<const std::uint32_t> b)
{
    ::reinsirk::detail::ProductCheckSizes(a.size(), b.size());
    const auto n = a.size();
    if (n < 2 || n > (std::size_t{1} << 19))
    {
        throw std::length_error("multiplicative_convolution_prime: size outside [2,2^19]");
    }
    const auto prime = static_cast<std::uint32_t>(n);
    for (std::uint32_t d = 2; d <= prime / d; ++d)
    {
        if (prime % d == 0)
        {
            throw std::invalid_argument("multiplicative_convolution_prime: size must be a prime");
        }
    }
    const auto order = n - 1;
    const auto root = ::reinsirk::detail::ProductPrimitiveRoot(prime);
    std::vector<std::uint32_t> left(order), right(order), group(order), answer(n);
    std::uint32_t value = 1;
    constexpr auto kMod = ::reinsirk::detail::kProductMod;
    // 非零の添字に原始根の指数を割り当てると、添字の積が指数の巡回和になる。
    for (std::size_t i = 0; i < order; ++i)
    {
        group[i] = value;
        left[i] = a[value] % kMod;
        right[i] = b[value] % kMod;
        value = static_cast<std::uint32_t>(std::uint64_t{value} * root % prime);
    }
    const auto a0 = a[0] % kMod;
    const auto b0 = b[0] % kMod;
    const std::uint64_t total_left_nonzero = std::accumulate(left.begin(), left.end(), std::uint64_t{0}) % kMod;
    const std::uint64_t total_right = (std::accumulate(right.begin(), right.end(), std::uint64_t{0}) + b0) % kMod;
    answer[0] = static_cast<std::uint32_t>((a0 * total_right + b0 * total_left_nonzero) % kMod);
    NTT<> ntt;
    const auto linear = ntt.convolution_normalized(std::move(left), std::move(right));
    for (std::size_t i = 0; i + 1 < order; ++i)
    {
        answer[group[i]] = ::reinsirk::detail::ProductAdd(linear[i], linear[i + order]);
    }
    answer[group.back()] = linear[order - 1];
    return answer;
}

inline std::vector<std::uint32_t> multiplicative_convolution_power_of_two(std::span<const std::uint32_t> a,
                                                                          std::span<const std::uint32_t> b)
{
    ::reinsirk::detail::ProductCheckSizes(a.size(), b.size());
    const auto n = a.size();
    if (!std::has_single_bit(n) || n > (std::size_t{1} << ::reinsirk::detail::kProductMaxPower))
    {
        throw std::length_error("multiplicative_convolution_power_of_two: expected a power of two up to 2^20");
    }
    const auto exponent = static_cast<unsigned>(std::countr_zero(n));
    auto left = ::reinsirk::detail::ProductSplitPowers(a, exponent);
    auto right = ::reinsirk::detail::ProductSplitPowers(b, exponent);
    std::vector<std::uint32_t> answer(n);
    for (unsigned i = 0; i <= exponent; ++i)
    {
        for (unsigned j = exponent - i; j <= exponent; ++j)
        {
            answer[0] = ::reinsirk::detail::ProductAdd(answer[0], ::reinsirk::detail::ProductScale(left.mass[i], right.mass[j]));
        }
    }
    if (exponent == 0)
    {
        return answer;
    }
    for (unsigned i = 0; i < exponent; ++i)
    {
        const auto remaining = exponent - 1 - i;
        answer[n / 2] = ::reinsirk::detail::ProductAdd(answer[n / 2], ::reinsirk::detail::ProductScale(left.mass[i], right.mass[remaining]));
    }

    NTT<> ntt;
    constexpr auto kHalf = (::reinsirk::detail::kProductMod + 1) / 2;
    // 奇数単元は ±5^k。符号を和と差に分け、5の指数の巡回畳み込みを行う。
    for (unsigned v = 0; v + 1 < exponent; ++v)
    {
        const auto modulus = n >> v;
        const auto width = modulus / 4;
        std::vector<std::uint32_t> plus(width), minus(width);
        for (unsigned i = 0; i < v; ++i)
        {
            ::reinsirk::detail::ProductFoldUnits(left.layers[i].sum, width);
            ::reinsirk::detail::ProductFoldUnits(left.layers[i].difference, width);
            ::reinsirk::detail::ProductFoldUnits(right.layers[i].sum, width);
            ::reinsirk::detail::ProductFoldUnits(right.layers[i].difference, width);
        }
        // v_a+v_b=vの組は、同じ変換長で周波数ごとに和を取れる。
        for (unsigned i = 0; i <= v; ++i)
        {
            const auto &a_layer = left.layers[i];
            const auto &b_layer = right.layers[v - i];
            ::reinsirk::detail::ProductAccumulateCyclic(ntt, a_layer.sum, b_layer.sum, plus);
            ::reinsirk::detail::ProductAccumulateCyclic(ntt, a_layer.difference, b_layer.difference, minus);
        }
        ntt.transform(plus, true);
        ntt.transform(minus, true);
        std::size_t unit = 1;
        for (std::size_t j = 0; j < width; ++j)
        {
            answer[unit << v] = ::reinsirk::detail::ProductScale(::reinsirk::detail::ProductAdd(plus[j], minus[j]), kHalf);
            answer[(modulus - unit) << v] = ::reinsirk::detail::ProductScale(::reinsirk::detail::ProductDifference(plus[j], minus[j]), kHalf);
            unit = unit * 5 % modulus;
        }
    }
    return answer;
}
#endif
