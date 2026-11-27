#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_CONSECUTIVE_LINEAR_RECURRENCE_DEFINED
#define SIRKIT_CONSECUTIVE_LINEAR_RECURRENCE_DEFINED

namespace reinsirk::detail
{
using ConsecutiveCoefficients = std::vector<std::uint32_t>;
inline constexpr auto kConsecutiveRecurrenceMod = NTT<>::modulus();

inline std::uint32_t ConsecutiveSubtract(std::uint32_t a, std::uint32_t b)
{
    return a >= b ? a - b : a + kConsecutiveRecurrenceMod - b;
}

inline void CheckConsecutiveRecurrenceSize(std::size_t order, std::size_t count)
{
    constexpr auto kLimit = NTT<>::max_length() / 2;
    if (order > kLimit || count > kLimit - order)
    {
        throw std::length_error("consecutive_linear_recurrence: NTT size limit");
    }
}

inline ConsecutiveCoefficients ConsecutiveReciprocalPrefix(std::span<const std::uint32_t> q,
                                                           std::size_t count, NTT<> &ntt)
{
    ConsecutiveCoefficients inverse{1};
    for (std::size_t known = 1; known < count;)
    {
        const auto next = std::min(count, 2 * known);
        const auto prefix = q.first(std::min(q.size(), next));
        auto correction = ntt.convolution_normalized({prefix.begin(), prefix.end()}, inverse);
        correction.resize(next);
        correction.erase(correction.begin(), correction.begin() + known);
        for (auto &value : correction)
        {
            value = ConsecutiveSubtract(0, value);
        }
        // G←G(2−QG)。下位known項は既知なので、補正の高位部分だけとGを畳み込む。
        const auto high = ntt.convolution_normalized(inverse, std::move(correction));
        inverse.resize(next);
        std::copy_n(high.begin(), next - known, inverse.begin() + known);
        known = next;
    }
    return inverse;
}

inline void ConsecutiveFromTransform(ConsecutiveCoefficients &filter, std::span<const std::uint32_t> transformed,
                                     NTT<> &ntt)
{
    filter.resize(transformed.size());
    ntt.transform(filter);
    for (std::size_t i = 0; i < filter.size(); ++i)
    {
        filter[i] = static_cast<std::uint32_t>(std::uint64_t{filter[i]} * transformed[i] % kConsecutiveRecurrenceMod);
    }
    ntt.transform(filter, true);
}

inline ConsecutiveCoefficients ConsecutiveReciprocalSlice(ConsecutiveCoefficients q, std::uint64_t first,
                                                          std::size_t count, NTT<> &ntt)
{
    const auto degree = q.size() - 1;
    if (degree == 0)
    {
        ConsecutiveCoefficients result(count);
        result[0] = static_cast<std::uint32_t>(first == 0);
        return result;
    }
    if (first <= degree)
    {
        const auto start = static_cast<std::size_t>(first);
        auto result = ConsecutiveReciprocalPrefix(q, start + count, ntt);
        result.erase(result.begin(), result.begin() + start);
        return result;
    }

    ConsecutiveCoefficients even((q.size() + 1) / 2), odd(q.size() / 2);
    for (std::size_t j = 0; j < q.size(); j += 2)
    {
        even[j / 2] = q[j];
    }
    for (std::size_t j = 1; j < q.size(); j += 2)
    {
        odd[j / 2] = ConsecutiveSubtract(0, q[j]);
    }
    // Q(x)Q(-x)=R(x²)。R(z)=even²−z*odd² を半分の長さの二乗で求める。
    auto norm = ntt.square(even);
    norm.resize(q.size());
    const auto odd_square = ntt.square(odd);
    for (std::size_t j = 0; j < odd_square.size(); ++j)
    {
        norm[j + 1] = ConsecutiveSubtract(norm[j + 1], odd_square[j]);
    }
    ConsecutiveCoefficients{}.swap(q);

    const auto distance = first - degree;
    const auto first_half = distance / 2 + distance % 2;
    const auto last_half = (first + (count - 1)) / 2;
    const auto count_half = static_cast<std::size_t>(last_half - first_half + 1);
    auto center = ConsecutiveReciprocalSlice(std::move(norm), first_half, count_half, ntt);

    // 1/Q=Q(-x)/R(x²) を偶数・奇数に分けて復元する。
    // 各抽出位置は該当filterの次数以上なので、低次へ回り込む巡回畳み込みでも値が変わらない。
    center.resize(std::bit_ceil(count_half));
    ntt.transform(center);
    ConsecutiveFromTransform(even, center, ntt);
    ConsecutiveFromTransform(odd, center, ntt);
    ConsecutiveCoefficients result(count);
    for (std::size_t j = 0; j < count; ++j)
    {
        const auto index = first + j;
        result[j] = ((index & 1) == 0 ? even : odd)[static_cast<std::size_t>(index / 2 - first_half)];
    }
    return result;
}
} // namespace reinsirk::detail

inline std::vector<std::uint32_t> consecutive_linear_recurrence(
    std::span<const std::uint32_t> initial, std::span<const std::uint32_t> coefficients,
    std::uint64_t first, std::size_t count)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (initial.size() != coefficients.size())
    {
        throw std::invalid_argument("consecutive_linear_recurrence: size mismatch");
    }
#endif
    if (count == 0)
    {
        return {};
    }
    if (count - 1 > std::numeric_limits<std::uint64_t>::max() - first)
    {
        throw std::overflow_error("consecutive_linear_recurrence: index overflow");
    }
    const auto order = initial.size();
    reinsirk::detail::CheckConsecutiveRecurrenceSize(order, count);
    if (order == 0)
    {
        return std::vector<std::uint32_t>(count);
    }
    const auto last = first + (count - 1);
    constexpr auto kMod = reinsirk::detail::kConsecutiveRecurrenceMod;
    if (last < order)
    {
        const auto requested = initial.subspan(static_cast<std::size_t>(first), count);
        std::vector<std::uint32_t> result;
        result.reserve(count);
        for (const auto value : requested)
        {
            result.push_back(value % kMod);
        }
        return result;
    }

    std::vector<std::uint32_t> denominator(order + 1);
    denominator[0] = 1;
    for (std::size_t i = 0; i < order; ++i)
    {
        denominator[i + 1] = reinsirk::detail::ConsecutiveSubtract(0, coefficients[i] % kMod);
    }
    while (denominator.size() > 1 && denominator.back() == 0)
    {
        denominator.pop_back();
    }
    NTT<> ntt;
    auto numerator = ntt.convolution(std::vector(initial.begin(), initial.end()), denominator);
    numerator.resize(order);
    const auto window_start = first - std::min(first, static_cast<std::uint64_t>(order - 1));
    const auto window_count = static_cast<std::size_t>(last - window_start + 1);
    auto window = reinsirk::detail::ConsecutiveReciprocalSlice(std::move(denominator), window_start, window_count, ntt);
    auto series = ntt.convolution_normalized(std::move(numerator), std::move(window));
    const auto start = static_cast<std::size_t>(first - window_start);
    return {series.begin() + start, series.begin() + start + count};
}

#endif
