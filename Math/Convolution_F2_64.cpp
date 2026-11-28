// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/convolution_f2_64.md
// 参考（Cantor基底の加法FFT）: https://arxiv.org/abs/1802.03932
//
// API一覧
// multiply_f2_64(a,b): F_(2^64)の積。O(1)。和はXOR。
// convolution_f2_64(a,b): 同じ体の係数列の畳み込み。空列なら空列。結果長は2^21まで。
// L=bit_ceil(|a|+|b|-1) としてO(L log^2 L)時間、O(L)領域。短い方が16以下ならO(|a||b|)。
// uint64_tの第iビットはx^iの係数。既約多項式はx^64+x^4+x^3+x+1。
// 対応するGCC/Clang x86-64ではPCLMULQDQを使用。SIRKIT_DISABLE_SIMDで標準C++経路を指定できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <wmmintrin.h>
#endif


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CONVOLUTION_F2_64_DEFINED
#define SIRKIT_CONVOLUTION_F2_64_DEFINED
namespace detail
{
inline constexpr std::size_t kF2ConvolutionMax = std::size_t{1} << 21;

constexpr std::uint64_t F2Carryless32(std::uint32_t a, std::uint32_t b) noexcept
{
    constexpr auto kMask0 = std::uint64_t{0x1111111111111111ULL};
    constexpr auto kMask1 = kMask0 << 1, kMask2 = kMask0 << 2, kMask3 = kMask0 << 3;
    const auto a0 = a & kMask0, a1 = a & kMask1, a2 = a & kMask2, a3 = a & kMask3;
    const auto b0 = b & kMask0, b1 = b & kMask1, b2 = b & kMask2, b3 = b & kMask3;
    // 各積の同じ桁に重なる項は8個以下。4ビット間隔なら次の有効ビットに桁上がりしない。
    return ((a0 * b0 ^ a1 * b3 ^ a2 * b2 ^ a3 * b1) & kMask0) ^
           ((a0 * b1 ^ a1 * b0 ^ a2 * b3 ^ a3 * b2) & kMask1) ^
           ((a0 * b2 ^ a1 * b1 ^ a2 * b0 ^ a3 * b3) & kMask2) ^
           ((a0 * b3 ^ a1 * b2 ^ a2 * b1 ^ a3 * b0) & kMask3);
}

constexpr std::uint64_t F2ReduceProduct(std::uint64_t low, std::uint64_t high) noexcept
{
    const auto overflow = (high >> 60) ^ (high >> 61) ^ (high >> 63);
    const auto fold = high ^ overflow;
    return low ^ fold ^ (fold << 1) ^ (fold << 3) ^ (fold << 4);
}

constexpr std::uint64_t F2MultiplyPortable(std::uint64_t a, std::uint64_t b) noexcept
{
    const auto a0 = static_cast<std::uint32_t>(a), a1 = static_cast<std::uint32_t>(a >> 32);
    const auto b0 = static_cast<std::uint32_t>(b), b1 = static_cast<std::uint32_t>(b >> 32);
    const auto low = F2Carryless32(a0, b0), high = F2Carryless32(a1, b1);
    const auto middle = F2Carryless32(a0 ^ a1, b0 ^ b1) ^ low ^ high;
    return F2ReduceProduct(low ^ (middle << 32), high ^ (middle >> 32));
}

#if !defined(SIRKIT_DISABLE_SIMD) && defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
inline bool F2PclmulAvailable() noexcept
{
    return __builtin_cpu_supports("pclmul");
}

__attribute__((target("pclmul,sse2"))) inline std::uint64_t F2MultiplyPclmul(std::uint64_t a, std::uint64_t b) noexcept
{
    const auto product = _mm_clmulepi64_si128(_mm_set_epi64x(0, std::bit_cast<std::int64_t>(a)),
                                              _mm_set_epi64x(0, std::bit_cast<std::int64_t>(b)), 0);
    return F2ReduceProduct(static_cast<std::uint64_t>(_mm_cvtsi128_si64(product)),
                           static_cast<std::uint64_t>(_mm_cvtsi128_si64(_mm_srli_si128(product, 8))));
}
#endif

// x→x²+x のGF(2)線形系を下から順に解いた基底。β_0=1、β_i²+β_i=β_(i-1)。
inline constexpr std::array<std::uint64_t, 21> kF2CantorBasis{
    0x0000000000000001ULL,
    0x19c9369f278adc02ULL,
    0xa181e7d66f5ff794ULL,
    0x5db84357ce785d08ULL,
    0xb973d466f5c9d0caULL,
    0x521ac889831a075eULL,
    0x033ce8beddc8a656ULL,
    0xb5846c4e07b91010ULL,
    0x4087b8cbb37a32ecULL,
    0x00d0d3888c0ae17cULL,
    0xafd5ac70237f2222ULL,
    0xe3f5af99cc3aaaf8ULL,
    0x5a1db3b16a0b58b8ULL,
    0x09947c54fe7ee248ULL,
    0x0e8eaf0e0068f544ULL,
    0xa2a113500b4b4f5aULL,
    0xe96f9805d6ce0bb0ULL,
    0x53496f8b5c9edd4cULL,
    0xad325cb6f4ac2a9eULL,
    0x4a8dcf8bd7ede826ULL,
    0xa3e9c552b6434210ULL,
};

static_assert([]
              {
                  if (kF2CantorBasis.front() != 1)
                  {
                      return false;
                  }
                  for (std::size_t i = 1; i < kF2CantorBasis.size(); ++i)
                  {
                      const auto x = kF2CantorBasis[i];
                      if ((F2MultiplyPortable(x, x) ^ x) != kF2CantorBasis[i - 1])
                      {
                          return false;
                      }
                  }
                  return true;
              }());

inline void F2ApplyBasisTerm(std::span<std::uint64_t> values, std::size_t index, std::size_t half, unsigned depth) noexcept
{
    // S_depth(x)=(Frob+1)^depth(x) の下位項は、depthの真部分マスクに対応する。
    for (unsigned mask = depth; mask != 0;)
    {
        mask = (mask - 1) & depth;
        values[index - half + (std::size_t{1} << mask)] ^= values[index];
    }
}

template <bool Inverse>
void F2MonomialBasis(std::span<std::uint64_t> values) noexcept
{
    const auto size = values.size();
    for (auto half = Inverse ? 2U : size / 2; Inverse ? half < size : half > 1;)
    {
        const auto depth = static_cast<unsigned>(std::countr_zero(half));
        for (std::size_t base = 0; base < size; base += 2 * half)
        {
            if constexpr (Inverse)
            {
                for (auto i = base + half; i < base + 2 * half; ++i)
                {
                    F2ApplyBasisTerm(values, i, half, depth);
                }
            }
            else
            {
                for (auto i = base + 2 * half; i-- > base + half;)
                {
                    F2ApplyBasisTerm(values, i, half, depth);
                }
            }
        }
        half = Inverse ? 2 * half : half / 2;
    }
}

template <bool Inverse, auto Multiply>
void F2AdditiveTransform(std::span<std::uint64_t> values, std::span<const std::uint64_t> offsets) noexcept
{
    const auto size = values.size();
    for (auto half = Inverse ? 1U : size / 2; Inverse ? half < size : half != 0;)
    {
        for (std::size_t base = 0; base < size; base += 2 * half)
        {
            const auto scale = offsets[base / half];
            if (scale == 0)
            {
                for (std::size_t i = base; i < base + half; ++i)
                {
                    values[i + half] ^= values[i];
                }
                continue;
            }
            // 左の剰余はa+scale*b、右の剰余はa+(scale+1)*b。
            for (std::size_t i = base; i < base + half; ++i)
            {
                if constexpr (Inverse)
                {
                    values[i + half] ^= values[i];
                    values[i] ^= Multiply(scale, values[i + half]);
                }
                else
                {
                    values[i] ^= Multiply(scale, values[i + half]);
                    values[i + half] ^= values[i];
                }
            }
        }
        half = Inverse ? 2 * half : half / 2;
    }
}

template <auto Multiply>
std::vector<std::uint64_t> F2Convolution(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b,
                                         std::size_t result_size)
{
    if (std::min(a.size(), b.size()) <= 16)
    {
        std::vector<std::uint64_t> result(result_size);
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            for (std::size_t j = 0; j < b.size(); ++j)
            {
                result[i + j] ^= Multiply(a[i], b[j]);
            }
        }
        return result;
    }
    const auto length = std::bit_ceil(result_size);
    std::vector<std::uint64_t> left(length), right(length), offsets(length);
    std::copy(a.begin(), a.end(), left.begin());
    std::copy(b.begin(), b.end(), right.begin());
    for (std::size_t i = 1; i < length; ++i)
    {
        offsets[i] = offsets[i & (i - 1)] ^ kF2CantorBasis[std::countr_zero(i)];
    }
    F2MonomialBasis<false>(left);
    F2MonomialBasis<false>(right);
    F2AdditiveTransform<false, Multiply>(left, offsets);
    F2AdditiveTransform<false, Multiply>(right, offsets);
    for (std::size_t i = 0; i < length; ++i)
    {
        left[i] = Multiply(left[i], right[i]);
    }
    F2AdditiveTransform<true, Multiply>(left, offsets);
    F2MonomialBasis<true>(left);
    left.resize(result_size);
    return left;
}

inline std::uint64_t F2Multiply(std::uint64_t a, std::uint64_t b) noexcept
{
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
    if (F2PclmulAvailable())
    {
        return F2MultiplyPclmul(a, b);
    }
#endif
    return F2MultiplyPortable(a, b);
}

inline std::vector<std::uint64_t> F2ConvolutionChecked(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    if (a.size() > kF2ConvolutionMax || b.size() - 1 > kF2ConvolutionMax - a.size())
    {
        throw std::length_error("convolution_f2_64: result length exceeds 2^21");
    }
    const auto size = a.size() + b.size() - 1;
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
    if (F2PclmulAvailable())
    {
        return F2Convolution<F2MultiplyPclmul>(a, b, size);
    }
#endif
    return F2Convolution<F2MultiplyPortable>(a, b, size);
}
}

inline std::uint64_t multiply_f2_64(std::uint64_t a, std::uint64_t b) noexcept
{
    return detail::F2Multiply(a, b);
}

inline std::vector<std::uint64_t> convolution_f2_64(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
{
    return detail::F2ConvolutionChecked(a, b);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
