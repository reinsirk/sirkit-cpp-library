#include <cassert>
#include <cfloat>
#include <cmath>
#include <limits>

#if defined(__x86_64__) && LDBL_MANT_DIG == 113
// x86-64のシステムのlong double ABIは80ビット拡張精度のまま。
// このテストで使う3関数を、同じbinary128 ABIを持つglibcの入口へ渡す。
extern "C" long double binary128_frexp(long double, int*) noexcept asm("frexpf128");
extern "C" long double binary128_ldexp(long double, int) noexcept asm("ldexpf128");
extern "C" long double binary128_fma(long double, long double, long double) noexcept asm("fmaf128");

extern "C" long double frexpl(long double x, int* exponent) noexcept {
    return binary128_frexp(x, exponent);
}
extern "C" long double ldexpl(long double x, int exponent) noexcept {
    return binary128_ldexp(x, exponent);
}
extern "C" long double fmal(long double x, long double y, long double z) noexcept {
    return binary128_fma(x, y, z);
}
#endif

int main() {
    using sirkit::geo::RealPoint;
    constexpr int low_bit = std::numeric_limits<long double>::digits - 1;
    const long double increment = std::ldexp(1.0L, -low_bit);
    const long double coordinate = 1 + increment;
    int exponent = 0;
    assert(std::frexp(coordinate, &exponent) == coordinate / 2 && exponent == 1);

    // 行列式は (1 + 2^-low_bit) - 1。binary128では仮数の途中に0の桁が並ぶ。
    const RealPoint a{0, 0}, b{coordinate, 1}, c{1, 1};
    const auto positive = sirkit::detail::SegmentDeterminant<long double>{}.orientation(a, b, c);
    const auto negative = sirkit::detail::SegmentDeterminant<long double>{}.orientation(a, c, b);
    assert(positive.sign == 1 && std::ldexp(positive.fraction, positive.exponent) == increment);
    assert(negative.sign == -1 && std::ldexp(negative.fraction, negative.exponent) == increment);
}
