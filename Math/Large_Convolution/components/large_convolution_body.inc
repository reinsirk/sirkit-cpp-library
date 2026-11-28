#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_LARGE_CONVOLUTION_DEFINED
#define SIRKIT_LARGE_CONVOLUTION_DEFINED
namespace detail
{
template <std::uint32_t Mod, std::uint32_t Root>
auto LargeConvolutionSpectra(std::span<const std::uint32_t> input, std::size_t block, std::size_t length,
                             NTT<Mod, Root> &transform)
{
    using Block = std::vector<std::uint32_t>;
    std::vector<Block> spectra;
    spectra.reserve((input.size() - 1) / block + 1);
    while (!input.empty())
    {
        const auto width = std::min(input.size(), block);
        auto part = input.first(width);
        input = input.subspan(width);
        Block spectrum;
        if (std::ranges::any_of(part, [](std::uint32_t coefficient)
                                {
                                    return coefficient != 0;
                                }))
        {
            spectrum.resize(length);
            for (std::size_t i = 0; i < width; ++i)
            {
                spectrum[i] = part[i] % Mod;
            }
            transform.transform(spectrum);
        }
        spectra.push_back(std::move(spectrum));
    }
    return spectra;
}

template <std::uint32_t Mod>
void LargeConvolutionDiagonal(std::span<std::uint32_t> frequencies,
                              const std::vector<std::pair<const std::uint32_t *, const std::uint32_t *>> &terms) noexcept
{
    constexpr auto kMaxProduct = std::uint64_t{Mod - 1} * (Mod - 1);
    constexpr auto kMaxTerms = (std::numeric_limits<std::uint64_t>::max() - (Mod - 1)) / kMaxProduct;
    auto pairs = std::span(terms);
    std::ranges::fill(frequencies, 0);
    while (!pairs.empty())
    {
        const auto count = static_cast<std::size_t>(std::min(kMaxTerms, std::uint64_t{pairs.size()}));
        const auto chunk = pairs.first(count);
        pairs = pairs.subspan(count);
        for (std::size_t k = 0; k < frequencies.size(); ++k)
        {
            std::uint64_t sum = frequencies[k];
            for (const auto &[left, right] : chunk)
            {
                sum += std::uint64_t{left[k]} * right[k];
            }
            frequencies[k] = static_cast<std::uint32_t>(sum % Mod);
        }
    }
}

template <std::uint32_t Mod>
std::vector<std::uint32_t> LargeConvolutionNaive(std::span<const std::uint32_t> left,
                                                 std::span<const std::uint32_t> right, std::size_t size)
{
    if (right.size() < left.size())
    {
        std::swap(left, right);
    }
    std::vector<std::uint32_t> result(size);
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        const std::uint64_t coefficient = left[i] % Mod;
        if (coefficient == 0)
        {
            continue;
        }
        for (std::size_t j = 0; j < right.size(); ++j)
        {
            result[i + j] = static_cast<std::uint32_t>((result[i + j] + coefficient * right[j]) % Mod);
        }
    }
    return result;
}

template <std::uint32_t Mod, std::uint32_t Root>
std::vector<std::uint32_t> LargeConvolutionMod(std::span<const std::uint32_t> left,
                                               std::span<const std::uint32_t> right, std::size_t transform_length)
{
    NTT<Mod, Root> transform;
    const auto size = convolution_result_size(left.size(), right.size(), "convolution_mod_large: result size overflow");
    if (size == 0)
    {
        return {};
    }
    const auto length = transform_length;
    if (!std::has_single_bit(length) || length < 2 || length > transform.max_length())
    {
        throw std::invalid_argument("convolution_mod_large: invalid transform length");
    }
    const auto shorter = std::min(left.size(), right.size());
    if (shorter <= 16)
    {
        return LargeConvolutionNaive<Mod>(left, right, size);
    }
    if (size <= length)
    {
        return transform.convolution(std::vector<std::uint32_t>(left.begin(), left.end()),
                                     std::vector<std::uint32_t>(right.begin(), right.end()));
    }
    const auto block = shorter <= length / 2 ? length - shorter + 1 : length / 2;
    const auto left_spectra = LargeConvolutionSpectra(left, block, length, transform);
    const auto right_spectra = LargeConvolutionSpectra(right, block, length, transform);
    std::vector<std::uint32_t> result(size), frequencies(length);
    std::vector<std::pair<const std::uint32_t *, const std::uint32_t *>> terms;
    terms.reserve(std::min(left_spectra.size(), right_spectra.size()));
    for (std::size_t stride = 0; stride < left_spectra.size() + right_spectra.size() - 1; ++stride)
    {
        const auto begin = stride < right_spectra.size() ? 0 : stride - right_spectra.size() + 1;
        const auto end = std::min(stride + 1, left_spectra.size());
        terms.clear();
        for (std::size_t i = begin; i < end; ++i)
        {
            const auto &a = left_spectra[i];
            const auto &b = right_spectra[stride - i];
            if (!a.empty() && !b.empty())
            {
                terms.emplace_back(a.data(), b.data());
            }
        }
        if (terms.empty())
        {
            continue;
        }
        LargeConvolutionDiagonal<Mod>(frequencies, terms);
        transform.transform(frequencies, true);
        const auto offset = stride * block;
        const auto count = std::min(length, size - offset);
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto sum = result[offset + i] + frequencies[i];
            result[offset + i] = sum >= Mod ? sum - Mod : sum;
        }
    }
    return result;
}
}

template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3>
std::vector<std::uint32_t> convolution_mod_large(std::span<const std::uint32_t> a,
                                                 std::span<const std::uint32_t> b,
                                                 std::size_t transform_length = NTT<Mod, Root>::max_length())
{
    return detail::LargeConvolutionMod<Mod, Root>(a, b, transform_length);
}
#endif
