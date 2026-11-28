// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<sirkit::ModularShiftTerm> terms(n);
    for (auto &[offset, modulus] : terms)
    {
        verify_in >> offset >> modulus;
    }
    std::size_t queries{};
    verify_in >> queries;
    std::vector<std::uint32_t> shifts(queries);
    for (auto &x : shifts)
    {
        verify_in >> x;
    }
    const auto answers = sirkit::modular_shift_sums(*std::ranges::max_element(shifts), terms);
    for (const auto x : shifts)
    {
        verify_out << answers[x] << '\n';
    }
}
