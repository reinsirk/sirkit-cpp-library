// SPDX-License-Identifier: CC0-1.0
#include "../Math/Fraction.cpp"
#include <cstdint>
#include <iostream>
#include <sstream>

int main() {
    constexpr Fraction a(2, 6), b(-3, 4);
    static_assert(a == Fraction(1, 3));
    std::cout << a.numerator() << ' ' << a.denominator() << '\n'; // 1 3
    std::cout << a + b << ' ' << a - b << '\n'; // -5/12 13/12
    std::cout << a * b << ' ' << a / b << '\n'; // -1/4 -4/9
    Fraction x = a;
    x += 2;
    x *= Fraction(3, 7);
    std::cout << x << ' ' << (x == 1) << ' ' << (b < a) << '\n'; // 1/1 1 1
    Fraction y = x;
    y -= Fraction(1, 2);
    y /= 2;
    std::cout << y << '\n'; // 1/4
    std::cout << +a << ' ' << -a << ' ' << b.abs() << ' ' << b.inverse() << '\n';
    // 1/3 -1/3 3/4 -4/3
    std::cout << b.floor() << ' ' << b.ceil() << ' ' << b.trunc() << '\n'; // -1 0 0
    std::cout << b.sign() << ' ' << x.is_integer() << '\n'; // -1 1
    std::cout << static_cast<double>(b) << '\n'; // -0.75
    std::istringstream in("-6/8 5");
    Fraction c, d;
    in >> c >> d;
    std::cout << c << ' ' << d << '\n'; // -3/4 5/1
    // 中間積が64bitを超えても、既約結果が表現できれば計算できる。
    constexpr std::int64_t large = 9'000'000'000'000'000'000;
    std::cout << Fraction(large, 7) * Fraction(7, large) << '\n'; // 1/1
}
