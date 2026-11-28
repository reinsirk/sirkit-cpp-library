// SPDX-License-Identifier: CC0-1.0
#include "../Math/Big_Integer.cpp"
#include <iostream>

int main()
{
    const sirkit::BigInteger<> a("1000000000000000000000000");
    std::cout << (a * -2 + 1).to_string() << '\n'; // -1999999999999999999999999
    const auto [quotient, remainder] = sirkit::BigInteger<16>("-FF").divmod(16);
    std::cout << quotient.to_string() << ' ' << remainder << '\n'; // -F -15
    const sirkit::BigInteger<> b("999999999999999999999999");
    const auto [large_quotient, large_remainder] = (-a).divmod(b);
    std::cout << large_quotient.to_string() << ' ' << large_remainder.to_string() << '\n'; // -1 -1
}
