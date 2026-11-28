// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_CONVOLUTION_WRAP64_DEFINED
#define SIRKIT_CONVOLUTION_WRAP64_DEFINED
namespace reinsirk::detail
{
inline constexpr std::size_t kWrap64MaxLength = std::size_t{1} << 21;
inline constexpr std::array<std::uint32_t, 5> kWrap64Primes{935329793, 943718401, 985661441, 998244353, 1004535809};
inline constexpr std::array<std::uint32_t, 5> kWrap64Roots{3, 7, 3, 3, 3};
static_assert(std::ranges::is_sorted(kWrap64Primes));             // 各CRT桁を次の大きい法でそのまま使う。
static_assert(kWrap64Primes.front() > (std::uint32_t{13} << 26)); // 五法の積は2^148より大きい。

consteval auto Wrap64Inverses()
{
    std::array<std::array<std::uint32_t, kWrap64Primes.size()>, kWrap64Primes.size()> result{};
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        for (std::size_t j = 0; j < i; ++j)
        {
            result[i][j] = ntt_power(kWrap64Primes[j], kWrap64Primes[i] - 2, kWrap64Primes[i]);
        }
    }
    return result;
}

inline constexpr auto kWrap64Inverses = Wrap64Inverses();
using Wrap64Residues = std::array<std::vector<std::uint32_t>, kWrap64Primes.size()>;

template <std::size_t Index>
std::vector<std::uint32_t> Wrap64Convolve(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
{
    constexpr auto kMod = kWrap64Primes[Index];
    NTT<kMod, kWrap64Roots[Index]> engine;
    std::vector<std::uint32_t> x(a.size()), y(b.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        x[i] = static_cast<std::uint32_t>(a[i] % kMod);
    }
    for (std::size_t i = 0; i < b.size(); ++i)
    {
        y[i] = static_cast<std::uint32_t>(b[i] % kMod);
    }
    return engine.convolution_normalized(std::move(x), std::move(y));
}

template <std::size_t... Indices>
Wrap64Residues Wrap64AllResidues(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b,
                                 std::index_sequence<Indices...>)
{
    return {Wrap64Convolve<Indices>(a, b)...};
}

template <std::size_t Index>
std::uint32_t Wrap64Digit(const Wrap64Residues &residues, std::size_t at,
                          const std::array<std::uint32_t, kWrap64Primes.size()> &digits)
{
    constexpr auto kMod = kWrap64Primes[Index];
    auto value = residues[Index][at];
    for (std::size_t j = 0; j < Index; ++j)
    {
        value = static_cast<std::uint32_t>(std::uint64_t{value + kMod - digits[j]} * kWrap64Inverses[Index][j] % kMod);
    }
    return value;
}

template <std::size_t... Indices>
std::uint64_t Wrap64Reconstruct(const Wrap64Residues &residues, std::size_t at,
                                std::index_sequence<Indices...>)
{
    std::array<std::uint32_t, kWrap64Primes.size()> digits{};
    ((digits[Indices] = Wrap64Digit<Indices>(residues, at, digits)), ...);
    std::uint64_t value{};
    for (std::size_t i = digits.size(); i > 0; --i)
    {
        value = value * kWrap64Primes[i - 1] + digits[i - 1]; // 下位64ビットだけを残す。
    }
    return value;
}

inline std::vector<std::uint64_t> ConvolveWrap64(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    if (a.size() > kWrap64MaxLength || b.size() > kWrap64MaxLength + 1 - a.size())
    {
        throw std::length_error("convolution_mod_2_64: length exceeds NTT budget");
    }
    std::vector<std::uint64_t> result(a.size() + b.size() - 1);
    if (std::min(a.size(), b.size()) <= 32)
    {
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            for (std::size_t j = 0; j < b.size(); ++j)
            {
                result[i + j] += a[i] * b[j];
            }
        }
        return result;
    }
    constexpr auto indices = std::make_index_sequence<kWrap64Primes.size()>{};
    const auto residues = Wrap64AllResidues(a, b, indices); // 係数はmin(|a|,|b|)*2^128以下。
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = Wrap64Reconstruct(residues, i, indices);
    }
    return result;
}
}

inline std::vector<std::uint64_t> convolution_mod_2_64(std::span<const std::uint64_t> a,
                                                       std::span<const std::uint64_t> b)
{
    return ::reinsirk::detail::ConvolveWrap64(a, b);
}
#endif
