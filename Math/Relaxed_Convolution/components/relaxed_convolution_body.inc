// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_RELAXED_CONVOLUTION_DEFINED
#define SIRKIT_RELAXED_CONVOLUTION_DEFINED
template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3>
class RelaxedConvolution
{
    using Residue = std::uint32_t;
    using Vector = std::vector<Residue>;

    NTT<Mod, Root> engine_;
    Vector left_, right_, sums_;

    static Residue Add(Residue a, Residue b) noexcept
    {
        const auto sum = std::uint64_t{a} + b;
        return static_cast<Residue>(sum >= Mod ? sum - Mod : sum);
    }

    static Residue Multiply(Residue a, Residue b) noexcept
    {
        return static_cast<Residue>(std::uint64_t{a} * b % Mod);
    }

    static Vector WithNext(const Vector &sequence, std::size_t length, Residue next)
    {
        Vector result(length);
        std::copy_n(sequence.end() - (length - 1), length - 1, result.begin());
        result.back() = next;
        return result;
    }

    static void Accumulate(std::span<Residue> sums, const Vector &addition)
    {
        for (std::size_t i = 0; i < addition.size(); ++i)
        {
            sums[i] = Add(sums[i], addition[i]);
        }
    }

    Vector NextContributions(std::size_t end, Residue a, Residue b)
    {
        const auto largest = std::min(std::bit_floor(end / 2), std::size_t{1} << std::countr_zero(end));
        Vector contributions(largest == 0 ? 0 : 2 * largest - 1);
        for (std::size_t width = 1; width <= largest; width *= 2)
        {
            auto last_left = WithNext(left_, width, a); // 小さい添字が[width,2*width)の組を足す。
            auto last_right = WithNext(right_, width, b);
            if (end == 2 * width)
            {
                Accumulate(contributions, engine_.convolution_normalized(std::move(last_left), std::move(last_right)));
            }
            else
            {
                Vector first_left(left_.begin() + width, left_.begin() + 2 * width);
                Vector first_right(right_.begin() + width, right_.begin() + 2 * width);
                Accumulate(contributions, engine_.convolution_normalized(std::move(last_left), std::move(first_right)));
                Accumulate(contributions, engine_.convolution_normalized(std::move(first_left), std::move(last_right)));
            }
        }
        return contributions;
    }

public:
    [[nodiscard]] static constexpr std::size_t max_size() noexcept
    {
        return NTT<Mod, Root>::max_length();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return left_.size();
    }

    void clear() noexcept
    {
        left_.clear();
        right_.clear();
        sums_.clear();
    }

    [[nodiscard]] Residue push(Residue a, Residue b)
    {
        const auto i = size();
        if (i == max_size())
        {
            throw std::length_error("RelaxedConvolution: stream length exceeds NTT budget");
        }
        if (i == std::min(left_.capacity(), right_.capacity()))
        {
            const auto capacity = std::max(std::size_t{1}, 2 * i);
            left_.reserve(capacity);
            right_.reserve(capacity);
        }
        a %= Mod;
        b %= Mod;
        const auto updates = NextContributions(i + 1, a, b);
        sums_.resize(std::max(sums_.size(), i + 1 + updates.size()));
        Accumulate(std::span<Residue>(sums_).subspan(i + 1), updates);
        const auto answer = i == 0 ? Multiply(a, b)
                                   : Add(sums_[i], Add(Multiply(a, right_.front()), Multiply(b, left_.front())));
        left_.push_back(a);
        right_.push_back(b);
        return answer;
    }
};
#endif
