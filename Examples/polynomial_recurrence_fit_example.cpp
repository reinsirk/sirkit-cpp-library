// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Recurrence_Fit.cpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <span>

int main()
{
    constexpr std::array<std::int64_t, 6> training{1, 1, 2, 6, 24, 120};
    const auto coefficients = sirkit::fit_polynomial_recurrence(training, 1, 1).value();
    std::cout << sirkit::polynomial_recurrence_term(std::span(training).first(1), coefficients, 7) << '\n'; // 5040
}
