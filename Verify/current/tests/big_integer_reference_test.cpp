// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Big_Integer.cpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
using boost::multiprecision::cpp_int;

template <unsigned Radix>
std::string Convert(cpp_int integer)
{
    constexpr std::string_view digits = "0123456789ABCDEF";
    const bool negative = integer < 0;
    if (negative)
    {
        integer = -integer;
    }
    std::string out;
    do
    {
        const cpp_int digit = integer % Radix;
        out.push_back(digits[digit.convert_to<unsigned>()]);
        integer /= Radix;
    } while (integer > 0);
    if (negative)
    {
        out.push_back('-');
    }
    return {out.rbegin(), out.rend()};
}

template <unsigned Radix>
void Compare(const cpp_int &left, const cpp_int &right)
{
    const sirkit::BigInteger<Radix> a(Convert<Radix>(left)), b(Convert<Radix>(right));
    assert(a.to_string() == Convert<Radix>(left));
    assert(b.to_string() == Convert<Radix>(right));
    assert((a < b) == (left < right));
    assert((a == b) == (left == right));
    assert((-a).to_string() == Convert<Radix>(-left));
    assert((a + b).to_string() == Convert<Radix>(left + right));
    assert((a - b).to_string() == Convert<Radix>(left - right));
    assert((a * b).to_string() == Convert<Radix>(left * right));
    auto result = a;
    result += b;
    assert(result.to_string() == Convert<Radix>(left + right));
    result -= a;
    assert(result == b);
    result *= a;
    assert(result.to_string() == Convert<Radix>(left * right));
    const std::uint32_t divisor = std::numeric_limits<std::uint32_t>::max() - 10;
    const auto [quotient, remainder] = a.divmod(divisor);
    assert(quotient.to_string() == Convert<Radix>(left / divisor));
    assert(remainder == (left % divisor).convert_to<std::int64_t>());
}
}

int main()
{
    using Decimal = sirkit::BigInteger<>;
    using Hex = sirkit::BigInteger<16>;
    assert(Decimal().to_string() == "0");
    assert(Decimal("+000000000").to_string() == "0");
    assert(Decimal("-000000000000").sign() == 0);
    assert(Hex("+abcdef").to_string() == "ABCDEF");
    assert(Hex("-0002").sign() < 0);
    assert(Decimal(std::numeric_limits<std::int64_t>::min()).to_string() == "-9223372036854775808");
    assert(Decimal(std::numeric_limits<std::int64_t>::max()).to_string() == "9223372036854775807");
    const auto [small, signed_remainder] = Decimal(-5).divmod(4);
    assert(small == -1 && signed_remainder == -1);
    for (std::int64_t a : {0, 1, -1, 9999, -9999, 1000000000, -1000000000})
    {
        for (std::int64_t b : {0, -1, 1, 9999, -9999, 1000000000, -1000000000})
        {
            Compare<10>(a, b);
            Compare<16>(a, b);
        }
    }
    std::mt19937_64 random(41700);
    for (unsigned length : {5u, 8u, 9u, 17u, 100u, 160u})
    {
        cpp_int a{}, b{};
        for (unsigned i = 0; i < length; ++i)
        {
            a <<= 64;
            b <<= 64;
            a += random();
            b += random();
        }
        Compare<10>(a, -b);
        Compare<16>(b, a);
    }
    const Decimal carry(std::string(8193, '9'));
    const Decimal plus_one(std::string("1") + std::string(8193, '0'));
    assert(carry + 1 == plus_one);
    assert(plus_one - 1 == carry);
    auto square = carry;
    square *= square;
    assert(square.to_string() == std::string(8192, '9') + "8" + std::string(8192, '0') + "1");
    try
    {
        static_cast<void>(carry.divmod(0));
        assert(false);
    }
    catch (const std::domain_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (std::string_view bad : {"", "+", "-", "A", "-0X1", "12_3", "a", "Z", "/", ":", "."})
    {
        try
        {
            static_cast<void>(Decimal(bad));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    for (std::string_view bad : {"/", "G", "g"})
    {
        try
        {
            static_cast<void>(Hex(bad));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#endif
}
