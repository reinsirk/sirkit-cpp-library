// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Fraction.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
using sirkit::Fraction;

template<class Error, class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const Error&) {
        return;
    }
    assert(false);
}
} // namespace

int main() {
    volatile std::uint64_t zero = 0;
    assert(sirkit::detail::fraction_gcd(3, zero) == 3);
    const Fraction positive(7, 4), negative(-7, 4), integer(-2);
    assert(positive.trunc() == 1 && positive.floor() == 1 && positive.ceil() == 2);
    assert(negative.trunc() == -1 && negative.floor() == -2 && negative.ceil() == -1);
    assert(integer.trunc() == -2 && integer.floor() == -2 && integer.ceil() == -2);
    assert(positive.sign() == 1 && negative.sign() == -1 && Fraction{}.sign() == 0);
    assert(integer.is_integer() && !positive.is_integer());
    assert(static_cast<double>(positive) == 1.75);

    assert(+negative == negative && -positive == negative && -Fraction{} == Fraction{});
    assert(negative.abs() == positive && positive.inverse() == Fraction(4, 7));
    assert(negative.inverse() == Fraction(-4, 7) && Fraction{}.abs() == Fraction{});
    const Fraction smallest(std::numeric_limits<std::int64_t>::min());
    rejects<std::domain_error>([] { (void)Fraction{}.inverse(); });
    rejects<std::overflow_error>([&] { (void)smallest.abs(); });
    rejects<std::overflow_error>([&] { (void)smallest.inverse(); });
    rejects<std::overflow_error>([&] { (void)-smallest; });

    Fraction value(1, 2);
    assert(&(value += Fraction(1, 3)) == &value && value == Fraction(5, 6));
    assert(&(value -= Fraction(1, 6)) == &value && value == Fraction(2, 3));
    assert(&(value *= Fraction(-3, 2)) == &value && value == Fraction(-1));
    assert(&(value /= Fraction(2)) == &value && value == Fraction(-1, 2));
    rejects<std::domain_error>([&] { value /= Fraction{}; });
    assert(value == Fraction(-1, 2));

    std::istringstream at_end("-42");
    assert((at_end >> value) && value == Fraction(-42) && at_end.eof());
}
