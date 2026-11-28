#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_MODULAR_SHIFT_MINIMUM_DEFINED
#define SIRKIT_MODULAR_SHIFT_MINIMUM_DEFINED

namespace detail
{
inline std::uint32_t ShiftMinimumResidue(std::int64_t value, std::uint32_t modulus)
{
    const auto remainder = value % static_cast<std::int64_t>(modulus);
    return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus : remainder);
}
} // namespace detail

[[nodiscard]] inline std::uint64_t sum_minimum_modular_shifts(
    std::uint32_t count, std::uint32_t modulus, std::int64_t step,
    std::span<const std::int64_t> offsets, std::int64_t start = 0)
{
    if (modulus == 0 || offsets.empty())
    {
        throw std::invalid_argument("sum_minimum_modular_shifts: zero modulus or no offsets");
    }
    if (count == 0)
    {
        return 0;
    }
    const auto phase = detail::ShiftMinimumResidue(start, modulus);
    std::vector<std::uint32_t> residues;
    residues.reserve(offsets.size());
    for (const auto offset : offsets)
    {
        const auto base = detail::ShiftMinimumResidue(offset, modulus);
        residues.push_back(static_cast<std::uint32_t>((std::uint64_t{base} + phase) % modulus));
    }
    const auto slope = detail::ShiftMinimumResidue(step, modulus);
    const std::uint64_t length = count;
    if (slope == 0 || count == 1)
    {
        return length * *std::ranges::min_element(residues);
    }
    std::ranges::sort(residues);
    const auto duplicates = std::ranges::unique(residues);
    residues.erase(duplicates.begin(), duplicates.end());

    // 一周の各区切りで、その次の剰余までの距離だけ最小値が減る。
    // 中間結果は2^64で折り返すが、最終結果はcount*(modulus-1)以下。
    auto total = (length * (length - 1) / 2) * slope + length * residues.front();
    for (std::size_t i = 0; i < residues.size(); ++i)
    {
        const auto prior = residues[i];
        const auto gap = i + 1 == residues.size() ? modulus - prior + residues.front() : residues[i + 1] - prior;
        const auto drops = static_cast<std::uint64_t>(floor_sum(count, modulus, slope, prior));
        total -= std::uint64_t{gap} * drops;
    }
    return total;
}

#endif
