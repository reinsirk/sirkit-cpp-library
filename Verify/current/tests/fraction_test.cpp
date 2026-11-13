// SPDX-License-Identifier: CC0-1.0
// Python fractionsによる独立期待値を標準入力から読み込む。
// 生成: python3 Verify/current/tests/fraction_reference.py /tmp/fraction_cases.txt
// 実行: ./fraction_test < /tmp/fraction_cases.txt
#ifndef SIRKIT_FRACTION_TEST_SOURCE
#define SIRKIT_FRACTION_TEST_SOURCE "../../../Math/Fraction.cpp"
#endif
#include SIRKIT_FRACTION_TEST_SOURCE
#include SIRKIT_FRACTION_TEST_SOURCE

#include <array>
#include <compare>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

static_assert(Fraction(2,-4) == Fraction(-1,2));
static_assert(Fraction(std::numeric_limits<std::int64_t>::min(),
                       std::numeric_limits<std::int64_t>::min()) == 1);
static_assert(Fraction(1,2) + Fraction(1,6) == Fraction(2,3));
static_assert(Fraction(2,3) * Fraction(9,4) == Fraction(3,2));
static_assert(Fraction(-1,3).floor() == -1 && Fraction(-1,3).ceil() == 0);
static_assert(Fraction(-2,3) < Fraction(-1,2));

namespace {
std::string value(const Fraction& x) {
    return std::to_string(x.numerator()) + '/' + std::to_string(x.denominator());
}
void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
std::string run(const std::string& op, std::istream& in) {
    std::int64_t n, d;
    in >> n >> d;
    if (op == "CTOR") return value(Fraction(n, d));
    const Fraction a(n,d);
    if (op == "SCALAR") {
        return std::to_string(a.floor()) + ',' + std::to_string(a.ceil()) + ',' +
            std::to_string(a.trunc()) + ',' + std::to_string(a.sign()) + ',' +
            std::to_string(a.is_integer());
    }
    if (op == "NEG") return value(-a);
    if (op == "ABS") return value(a.abs());
    if (op == "INV") return value(a.inverse());
    std::int64_t m, e;
    in >> m >> e;
    const Fraction b(m,e);
    if (op == "CMP") {
        const int c = (a > b) - (a < b);
        require((a == b) == (c == 0), "== inconsistent");
        require((a != b) == (c != 0), "!= inconsistent");
        require((a <= b) == (c <= 0), "<= inconsistent");
        require((a >= b) == (c >= 0), ">= inconsistent");
        require(((a <=> b) < 0) == (c < 0), "<=> inconsistent");
        require(((a <=> b) > 0) == (c > 0), "<=> inconsistent");
        return std::to_string(c);
    }
    if (op == "ADD") return value(a+b);
    if (op == "SUB") return value(a-b);
    if (op == "MUL") return value(a*b);
    if (op == "DIV") return value(a/b);
    Fraction x = a;
    try {
        if (op == "IADD") x += b;
        else if (op == "ISUB") x -= b;
        else if (op == "IMUL") x *= b;
        else if (op == "IDIV") x /= b;
        else throw std::runtime_error("unknown operation");
    } catch (...) {
        require(x == a, "compound assignment changed value on exception");
        throw;
    }
    return value(x);
}
std::size_t gcd_reference_checks() {
    constexpr std::uint64_t limit = std::uint64_t{1} << 63;
    constexpr std::array values = {std::uint64_t{0}, std::uint64_t{1}, std::uint64_t{2},
        std::uint64_t{3}, std::uint64_t{4}, std::uint64_t{5}, std::uint64_t{7},
        std::uint64_t{8}, std::uint64_t{15}, std::uint64_t{16}, std::uint64_t{17},
        (std::uint64_t{1} << 31) - 1, std::uint64_t{1} << 31, (std::uint64_t{1} << 31) + 1,
        (std::uint64_t{1} << 32) - 1, std::uint64_t{1} << 32, (std::uint64_t{1} << 32) + 1,
        (std::uint64_t{1} << 62) - 1, std::uint64_t{1} << 62, (std::uint64_t{1} << 62) + 1,
        limit - 2, limit - 1, limit};
    std::size_t count = 0;
    const auto check = [&](std::uint64_t a, std::uint64_t b) {
        require(reinsirk::detail::fraction_gcd(a,b) == std::gcd(a,b), "internal gcd mismatch");
        ++count;
    };
    for (auto a : values) for (auto b : values) check(a,b);
    for (unsigned bit = 0; bit <= 63; ++bit) {
        const auto x = std::uint64_t{1} << bit;
        for (auto y : values) { check(x,y); check(x-1,y); if (x < limit) check(x+1,y); }
    }
    std::mt19937_64 rng(0xc836942f0ab55141ULL);
    for (std::size_t i = 0; i < 1000000; ++i) {
        auto a = rng() >> 1, b = rng() >> 1;
        if (i % 17 == 0) a = limit;
        if (i % 19 == 0) b = limit;
        check(a,b);
    }
    return count;
}
void supplemental() {
    require(Fraction() == 0, "default is not zero");
    require(Fraction(3) == Fraction(3,1), "integer construction");
    require(Fraction(1,2) + 1 == Fraction(3,2), "right integer addition");
    require(1 + Fraction(1,2) == Fraction(3,2), "left integer addition");
    require(Fraction(1,2) < 1 && 0 < Fraction(1,2), "mixed comparison");
    require(value(+Fraction(-2,-4)) == "1/2", "unary plus");
    Fraction x(7,11);
    x += x; require(x == Fraction(14,11), "self addition");
    x -= x; require(x == 0, "self subtraction");
    x = Fraction(-3,5); x *= x; require(x == Fraction(9,25), "self multiplication");
    x /= x; require(x == 1, "self division");
    require(static_cast<double>(Fraction(-7,5)) == -7.0 / 5.0, "double conversion");
    require(static_cast<long double>(Fraction(-7,5)) == -7.0L / 5.0L, "long double conversion");
    for (const char* input : {"3/4", "-3/4", "3/-4", "-3/-4", "0/123", "-17", "+17", "123/246"}) {
        Fraction a;
        std::istringstream in(input);
        require(bool(in >> a), "valid stream input failed");
        std::ostringstream out;
        out << a;
        Fraction b;
        std::istringstream again(out.str());
        require(bool(again >> b) && a == b, "stream round trip");
    }
    for (const char* input : {"", "abc", "1/", "1/0", "9223372036854775808", "1/9223372036854775808", "-9223372036854775808/-1", "1/-9223372036854775808"}) {
        Fraction a(3,7);
        std::istringstream in(input);
        require(!(in >> a), "invalid stream input accepted");
        require(a == Fraction(3,7), "failed stream input changed value");
    }
    // 末尾の非分数文字を消費しない。
    Fraction a;
    std::istringstream in("3/4 5 z");
    require(bool(in >> a) && a == Fraction(3,4), "multi stream first");
    require(bool(in >> a) && a == Fraction(5), "multi stream integer");
    char tail = 0; in >> tail;
    require(tail == 'z', "stream consumed next token");
}
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    try {
        const auto gcd_checks = gcd_reference_checks();
        supplemental();
        std::size_t count = 0;
        std::string line;
        while (std::getline(std::cin,line)) {
            if (line.empty()) continue;
            std::istringstream in(line);
            std::string op;
            in >> op;
            std::string actual;
            try {
                actual = run(op,in);
            } catch (const std::overflow_error&) {
                actual = "OVERFLOW";
            } catch (const std::domain_error&) {
                actual = "ZERO";
            } catch (const std::invalid_argument&) {
                actual = "ZERO";
            }
            std::string expected;
            in >> expected;
            if (actual != expected) {
                std::cerr << "Mismatch case " << count << ": " << line
                          << " actual=" << actual << '\n';
                return 1;
            }
            ++count;
        }
        require(count != 0, "no independent cases supplied");
        std::cout << count << " Python-fractions cases, " << gcd_checks
                  << " internal gcd reference cases and supplemental API checks passed\n";
    } catch (const std::exception& ex) {
        std::cerr << "Fraction test failed: " << ex.what() << '\n';
        return 1;
    }
}
