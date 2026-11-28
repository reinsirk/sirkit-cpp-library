// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{
struct Residue
{
    static inline std::uint64_t modulus;
    std::uint64_t value{};

    Residue() = default;

    explicit Residue(std::uint64_t x) : value(x % modulus)
    {
    }

    Residue &operator+=(Residue other)
    {
        value = (value + other.value) % modulus;
        return *this;
    }
    Residue operator*(Residue other) const
    {
        return Residue(value * other.value);
    }
};
} // namespace

int main()
{
    std::uint64_t count{}, first{}, step{};
    std::uint32_t modulus{};
    verify_in >> count >> first >> step >> modulus;
    Residue::modulus = modulus;
    std::uint64_t answer = 0;
    for (std::uint64_t limit = 10; count != 0; limit *= 10)
    {
        if (first >= limit)
        {
            continue;
        }
        const auto length = std::min(count, 1 + (limit - 1 - first) / step);
        const auto multiplier = limit % modulus;
        const auto increment = step % modulus;
        std::array<std::uint32_t, 6> known{};
        known[0] = static_cast<std::uint32_t>(answer);
        auto element = first % modulus;
        // 固定桁で連結する列は、等差数列の2階差と1階の更新式から位数3以下。
        for (std::size_t i = 1; i < known.size(); ++i)
        {
            known[i] = static_cast<std::uint32_t>((multiplier * known[i - 1] + element) % modulus);
            element = (element + increment) % modulus;
        }
        const auto reconstructed = sirkit::reeds_sloane(known, modulus);
        std::vector<Residue> recurrence, initial;
        for (std::size_t i = 0; i < reconstructed.size(); ++i)
        {
            recurrence.emplace_back(reconstructed[i]);
            initial.emplace_back(known[i]);
        }
        answer = sirkit::LinearRecurrenceTerm<Residue>(initial, recurrence, length).value;
        count -= length;
        if (count != 0)
        {
            first += step * length;
        }
    }
    verify_out << answer << '\n';
}
