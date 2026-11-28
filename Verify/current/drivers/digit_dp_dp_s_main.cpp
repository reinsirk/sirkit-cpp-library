// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

namespace
{
struct Count
{
    static constexpr std::uint32_t kModulus = 1'000'000'007;
    std::uint32_t value = 0;

    Count &operator+=(Count other) noexcept
    {
        value += other.value;
        if (value >= kModulus)
        {
            value -= kModulus;
        }
        return *this;
    }
};
}

int main()
{
    std::string upper;
    std::size_t divisor{};
    verify_in >> upper >> divisor;
    const auto distribution = sirkit::digit_dp_counts<Count>(upper, divisor, 0, [divisor](std::size_t state, unsigned digit, bool)
                                                             {
                                                                 return state + digit < divisor ? state + digit : (state + digit) % divisor;
                                                             });
    const auto occurrences = distribution.front().value;
    verify_out << (occurrences == 0 ? Count::kModulus - 1 : occurrences - 1) << '\n';
}
