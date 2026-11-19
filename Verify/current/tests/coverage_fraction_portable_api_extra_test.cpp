// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_FRACTION_DISABLE_INT128
#define SIRKIT_FRACTION_DISABLE_INT128
#endif
#include "../../../Math/Fraction.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <boost/multiprecision/cpp_int.hpp>

namespace {
using Big = boost::multiprecision::cpp_int;
using Integer = std::int64_t;
using sirkit::Fraction;
constexpr Integer kMin = std::numeric_limits<Integer>::min();
constexpr Integer kMax = std::numeric_limits<Integer>::max();

void equivalent(Big n, Big d, auto calculate) {
    if (d < 0) {
        n = -n;
        d = -d;
    }
    if (d != 0) {
        Big a = n < 0 ? -n : n;
        Big b = d;
        while (b != 0) {
            const Big remainder = a % b;
            a = b;
            b = remainder;
        }
        n /= a;
        d /= a;
    }
    const bool overflow = d != 0 && (n < kMin || n > kMax || d > kMax);
    try {
        const Fraction result = calculate();
        assert(d != 0 && !overflow);
        assert(result.numerator() == n && result.denominator() == d);
    } catch (const std::domain_error&) {
        assert(d == 0);
    } catch (const std::overflow_error&) {
        assert(overflow);
    }
}

void check_operations() {
    constexpr std::array<Integer, 8> numerators{kMin, kMin + 1, -9223372036854775783, -1, 0, 1, kMax - 1, kMax};
    constexpr std::array<Integer, 7> denominators{-2, 1, 2, 3, 2'000'000'014, 3'000'000'021, kMax};
    for (Integer n : numerators) for (Integer d : denominators) {
        equivalent(n, d, [=] { return Fraction(n, d); });
    }
    for (Integer n : numerators) for (Integer m : numerators) {
        for (Integer d : denominators) for (Integer e : denominators) {
            const Fraction a(n, d), b(m, e);
            const Big left = Big(a.numerator()) * b.denominator();
            const Big right = Big(b.numerator()) * a.denominator();
            const Big denom = Big(a.denominator()) * b.denominator();
            assert((a < b) == (left < right));
            assert((a == b) == (left == right));
            equivalent(left + right, denom, [&] { return a + b; });
            equivalent(left - right, denom, [&] { return a - b; });
            equivalent(Big(a.numerator()) * b.numerator(), denom, [&] { return a * b; });
            equivalent(Big(a.numerator()) * b.denominator(), Big(a.denominator()) * b.numerator(), [&] { return a / b; });
        }
    }
    equivalent(1, kMin, [] { return Fraction(1, kMin); });
    equivalent(1, 0, [] { return Fraction(1, 0); });
    equivalent(0, kMin, [] { return Fraction(0, kMin); });
}

void check_streams() {
    Fraction result;
    std::istringstream input("-18/-24 42 0/13");
    assert((input >> result) && result.numerator() == 3 && result.denominator() == 4);
    std::ostringstream output;
    output << result;
    assert(output.str() == "3/4");
    assert((input >> result) && result.numerator() == 42 && result.denominator() == 1);
    assert((input >> result) && result.numerator() == 0 && result.denominator() == 1);
    for (const char* invalid : {"", "word", "12/", "1/0", "-9223372036854775808/-1", "1/-9223372036854775808"}) {
        std::istringstream bad(invalid);
        const Fraction saved(3, 7);
        result = saved;
        assert(!(bad >> result));
        assert(result == saved);
    }
}
}

int main() {
    check_operations();
    check_streams();
}
