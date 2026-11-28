// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_MIXED_RADIX_DEFINED
#define SIRKIT_MIXED_RADIX_DEFINED
template <unsigned Radix = 10>
class MixedRadix
{
    using Int = BigInteger<Radix>;
    static constexpr std::size_t kLeafSize = 24;

    std::vector<std::uint32_t> radices_;
    std::vector<Int> products_;

    void build(std::size_t node, std::size_t begin, std::size_t end)
    {
        if (end - begin <= kLeafSize)
        {
            Int product{1};
            for (auto i = begin; i < end; ++i)
            {
                product *= Int{radices_[i]};
            }
            products_[node] = std::move(product);
            return;
        }
        const auto middle = begin + (end - begin) / 2;
        build(2 * node, begin, middle);
        build(2 * node + 1, middle, end);
        products_[node] = products_[2 * node] * products_[2 * node + 1];
    }

    [[nodiscard]] Int encode_tree(std::size_t node, std::size_t begin, std::size_t end,
                                  std::span<const std::uint32_t> digits) const
    {
        if (end - begin <= kLeafSize)
        {
            Int value;
            for (auto i = begin; i < end; ++i)
            {
                value = value * Int{radices_[i]} + Int{digits[i]};
            }
            return value;
        }
        const auto middle = begin + (end - begin) / 2;
        auto high = encode_tree(2 * node, begin, middle, digits);
        auto low = encode_tree(2 * node + 1, middle, end, digits);
        return high * products_[2 * node + 1] + low;
    }

    void decode_tree(std::size_t node, std::size_t begin, std::size_t end, Int value,
                     std::span<std::uint32_t> digits) const
    {
        if (value.sign() == 0)
        {
            return;
        }
        if (end - begin <= kLeafSize)
        {
            for (auto i = end; i > begin; --i)
            {
                auto [quotient, remainder] = value.divmod(radices_[i - 1]);
                digits[i - 1] = static_cast<std::uint32_t>(remainder);
                value = std::move(quotient);
            }
            return;
        }
        const auto middle = begin + (end - begin) / 2;
        auto [high, low] = value.divmod(products_[2 * node + 1]);
        decode_tree(2 * node, begin, middle, std::move(high), digits);
        decode_tree(2 * node + 1, middle, end, std::move(low), digits);
    }

public:
    explicit MixedRadix(std::span<const std::uint32_t> radices)
        : radices_(radices.begin(), radices.end()),
          products_(2 * std::bit_ceil(radices.size() / kLeafSize + static_cast<std::size_t>(radices.size() % kLeafSize != 0)))
    {
        if (std::ranges::find(radices_, 0u) != radices_.end())
        {
            throw std::invalid_argument("MixedRadix: positive digit radices required");
        }
        build(1, 0, radices_.size());
    }

    [[nodiscard]] const Int &capacity() const noexcept
    {
        return products_[1];
    }

    [[nodiscard]] Int encode(std::span<const std::uint32_t> digits) const
    {
        if (digits.size() != radices_.size())
        {
            throw std::invalid_argument("MixedRadix: digit count differs");
        }
        for (std::size_t i = 0; i < digits.size(); ++i)
        {
            if (digits[i] >= radices_[i])
            {
                throw std::invalid_argument("MixedRadix: digit exceeds radix");
            }
        }
        return encode_tree(1, 0, radices_.size(), digits);
    }

    [[nodiscard]] std::vector<std::uint32_t> decode(Int value) const
    {
        if (value.sign() < 0 || value >= capacity())
        {
            throw std::out_of_range("MixedRadix: integer exceeds representable range");
        }
        std::vector<std::uint32_t> digits(radices_.size());
        decode_tree(1, 0, digits.size(), std::move(value), digits);
        return digits;
    }
};
#endif
