// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fraction.md
//
// API一覧
// [Fraction]
// Fraction(n=0,d=1): int64_tの分子・分母から既約分数を作る。分母は正、0は0/1。O(log M)。
// numerator()/denominator()/sign()/is_integer(): 正規化済みの分子・分母・符号・整数判定。O(1)。
// 四則演算・複合代入・単項+/-: 厳密な既約結果を返す。整数との演算も可。O(log M)。
// ==, <=>: 厳密な等値・大小比較。O(1)。最終結果が表現できても中間積が大きい場合に対応。
// inverse()/abs(): 逆数・絶対値を返す。O(1)。0の逆数はdomain_error。
// floor()/ceil()/trunc(): 床・天井・0方向への整数丸めをint64_tで返す。O(1)。
// explicit浮動小数点変換: 分子/分母を対象の浮動小数点型で計算する。O(1)。
// <<, >>: n/dを出力。入力は整数またはn/d（/は分子の直後）。失敗時は値を保持。
// 0分母・0除算はdomain_error、既約結果の分子/正分母がint64_t範囲外ならoverflow_error。
// これらの数学的・表現上の保護はSKIP_RUNTIME_CHECKS時も有効。Mは整数の絶対値の上限。
// SIRKIT_FRACTION_DISABLE_INT128: GCC/Clangの128ビット拡張を使わず標準C++経路を選ぶ。

#include <bit>
#include <compare>
#include <concepts>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>

#ifndef SIRKIT_FRACTION_DETAIL_DEFINED
#define SIRKIT_FRACTION_DETAIL_DEFINED
namespace reinsirk::detail {

// 引数は0以上2^63以下。奇数化した正の値の差は絶対値が2^63未満。
constexpr std::uint64_t fraction_gcd(std::uint64_t a, std::uint64_t b) {
    if (a == 1 || b == 1) return 1;
    if (a == 0) return b;
    if (b == 0) return a;
    const auto common = std::countr_zero(a | b);
    a >>= std::countr_zero(a);
    b >>= std::countr_zero(b);
    while (a != b) {
        const auto diff = a - b;
        const auto shift = std::countr_zero(diff);
        const auto low = a < b ? a : b;
        const auto mag = diff < (std::uint64_t{1} << 63) ? diff : 0 - diff;
        a = mag >> shift;
        b = low;
    }
    return a << common;
}

struct FractionWide {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FRACTION_DISABLE_INT128)
    __extension__ using Storage = unsigned __int128;
    Storage value = 0;
    constexpr FractionWide(std::uint64_t x = 0) : value(x) {}
    static constexpr FractionWide product(std::uint64_t a, std::uint64_t b) {
        FractionWide r;
        r.value = Storage(a) * b;
        return r;
    }
    friend constexpr FractionWide operator+(FractionWide a, FractionWide b) {
        a.value += b.value;
        return a;
    }
    friend constexpr FractionWide operator-(FractionWide a, FractionWide b) {
        a.value -= b.value;
        return a;
    }
    constexpr auto operator<=>(const FractionWide&) const = default;
    constexpr std::uint64_t low() const { return static_cast<std::uint64_t>(value); }
    constexpr bool fits(std::uint64_t bound) const { return value <= bound; }
    constexpr std::pair<FractionWide, std::uint64_t> divmod(std::uint64_t d) const {
        FractionWide q;
        q.value = value / d;
        return {q, static_cast<std::uint64_t>(value % d)};
    }
#else
    std::uint64_t hi = 0, lo = 0;
    constexpr FractionWide(std::uint64_t x = 0) : lo(x) {}
    static constexpr FractionWide product(std::uint64_t a, std::uint64_t b) {
        constexpr std::uint64_t mask = 0xffffffff;
        const auto a0 = a & mask, a1 = a >> 32;
        const auto b0 = b & mask, b1 = b >> 32;
        const auto t0 = a0 * b0;
        const auto t1 = a1 * b0 + (t0 >> 32);
        const auto t2 = a0 * b1 + (t1 & mask);
        FractionWide r;
        r.hi = a1 * b1 + (t1 >> 32) + (t2 >> 32);
        r.lo = (t2 << 32) | (t0 & mask);
        return r;
    }
    friend constexpr FractionWide operator+(FractionWide a, FractionWide b) {
        const auto old = a.lo;
        a.lo += b.lo;
        a.hi += b.hi + (a.lo < old);
        return a;
    }
    friend constexpr FractionWide operator-(FractionWide a, FractionWide b) {
        const auto old = a.lo;
        a.lo -= b.lo;
        a.hi -= b.hi + (old < b.lo);
        return a;
    }
    constexpr auto operator<=>(const FractionWide&) const = default;
    constexpr std::uint64_t low() const { return lo; }
    constexpr bool fits(std::uint64_t bound) const { return hi == 0 && lo <= bound; }
    constexpr std::pair<FractionWide, std::uint64_t> divmod(std::uint64_t d) const {
        if (hi == 0) return {FractionWide(lo / d), lo % d};
        FractionWide q;
        q.hi = hi / d;
        auto rem = hi % d;
        // 呼び出し側では1 <= d <= 2^63。余りを1bit伸ばしてもuint64_tに収まる。
        for (int bit = 63; bit >= 0; --bit) {
            rem = (rem << 1) | ((lo >> bit) & 1);
            if (rem >= d) {
                rem -= d;
                q.lo |= std::uint64_t{1} << bit;
            }
        }
        return {q, rem};
    }
#endif
};

constexpr std::uint64_t fraction_magnitude(std::int64_t x) {
    const auto u = static_cast<std::uint64_t>(x);
    return x < 0 ? std::uint64_t{0} - u : u;
}

} // namespace reinsirk::detail
#endif

#ifndef SIRKIT_FRACTION_DEFINED
#define SIRKIT_FRACTION_DEFINED
class Fraction {
    using Wide = reinsirk::detail::FractionWide;
    std::int64_t num_ = 0, den_ = 1;
    static constexpr auto max_ = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());

    static constexpr std::uint64_t magnitude(std::int64_t n) {
        return reinsirk::detail::fraction_magnitude(n);
    }
    static constexpr Fraction result(Wide n, Wide d, bool negative) {
        if (!n.fits(max_ + static_cast<unsigned>(negative)) || !d.fits(max_))
            throw std::overflow_error("Fraction: result out of int64_t range");
        Fraction r;
        const auto u = n.low();
        r.num_ = negative && u != 0 ? -static_cast<std::int64_t>(u - 1) - 1
                                    : static_cast<std::int64_t>(u);
        r.den_ = static_cast<std::int64_t>(d.low());
        return r;
    }
    static constexpr Fraction combine(const Fraction& a, const Fraction& b, bool subtract) {
        const auto da = static_cast<std::uint64_t>(a.den_);
        const auto db = static_cast<std::uint64_t>(b.den_);
        std::uint64_t g;
        Wide left, right;
        if (da == db) {
            g = da;
            left = Wide(magnitude(a.num_));
            right = Wide(magnitude(b.num_));
        } else {
            g = reinsirk::detail::fraction_gcd(da, db);
            left = Wide::product(magnitude(a.num_), db / g);
            right = Wide::product(magnitude(b.num_), da / g);
        }
        bool negative = a.num_ < 0;
        const bool right_negative = (b.num_ < 0) != subtract;
        Wide n;
        if (negative == right_negative) n = left + right;
        else if (left >= right) n = left - right;
        else { n = right - left; negative = right_negative; }
        if (n == Wide{}) return {};
        // 各分数は既約なので、和の分子との追加の共通因子はgの約数だけ。
        const auto h = g == 1 ? 1 : reinsirk::detail::fraction_gcd(n.divmod(g).second, g);
        if (h != 1) n = n.divmod(h).first;
        return result(n, Wide::product(da / g, db / h), negative);
    }

public:
    constexpr Fraction() = default;
    constexpr Fraction(std::int64_t n, std::int64_t d = 1) {
        if (d == 0) throw std::domain_error("Fraction: zero denominator");
        if (d == 1) { num_ = n; return; }
        if (n == 0) return;
        auto u = magnitude(n), v = magnitude(d);
        const auto g = reinsirk::detail::fraction_gcd(u, v);
        if (g == 1 && d > 0) { num_ = n; den_ = d; return; }
        if (g != 1) { u /= g; v /= g; }
        *this = result(Wide(u), Wide(v), (n < 0) != (d < 0));
    }
    constexpr std::int64_t numerator() const noexcept { return num_; }
    constexpr std::int64_t denominator() const noexcept { return den_; }
    constexpr int sign() const noexcept { return (num_ > 0) - (num_ < 0); }
    constexpr bool is_integer() const noexcept { return den_ == 1; }
    constexpr std::int64_t trunc() const noexcept { return num_ / den_; }
    constexpr std::int64_t floor() const noexcept { return trunc() - (num_ % den_ < 0); }
    constexpr std::int64_t ceil() const noexcept { return trunc() + (num_ % den_ > 0); }
    constexpr Fraction inverse() const {
        if (num_ == 0) throw std::domain_error("Fraction: inverse of zero");
        return result(Wide(static_cast<std::uint64_t>(den_)), Wide(magnitude(num_)), num_ < 0);
    }
    constexpr Fraction abs() const {
        return result(Wide(magnitude(num_)), Wide(static_cast<std::uint64_t>(den_)), false);
    }
    template<std::floating_point T>
    explicit constexpr operator T() const noexcept { return static_cast<T>(num_) / static_cast<T>(den_); }
    constexpr Fraction operator+() const noexcept { return *this; }
    constexpr Fraction operator-() const {
        return result(Wide(magnitude(num_)), Wide(static_cast<std::uint64_t>(den_)), num_ > 0);
    }
    friend constexpr Fraction operator+(const Fraction& a, const Fraction& b) { return combine(a, b, false); }
    friend constexpr Fraction operator-(const Fraction& a, const Fraction& b) { return combine(a, b, true); }
    friend constexpr Fraction operator*(const Fraction& a, const Fraction& b) {
        if (a.num_ == 0 || b.num_ == 0) return {};
        const auto na = magnitude(a.num_), nb = magnitude(b.num_);
        const auto da = static_cast<std::uint64_t>(a.den_), db = static_cast<std::uint64_t>(b.den_);
        const auto g = reinsirk::detail::fraction_gcd(na, db), h = reinsirk::detail::fraction_gcd(nb, da);
        return result(Wide::product(na / g, nb / h), Wide::product(da / h, db / g), (a.num_ < 0) != (b.num_ < 0));
    }
    friend constexpr Fraction operator/(const Fraction& a, const Fraction& b) {
        if (b.num_ == 0) throw std::domain_error("Fraction: division by zero");
        if (a.num_ == 0) return {};
        const auto na = magnitude(a.num_), nb = magnitude(b.num_);
        const auto da = static_cast<std::uint64_t>(a.den_), db = static_cast<std::uint64_t>(b.den_);
        const auto g = reinsirk::detail::fraction_gcd(na, nb), h = reinsirk::detail::fraction_gcd(da, db);
        return result(Wide::product(na / g, db / h), Wide::product(da / h, nb / g), (a.num_ < 0) != (b.num_ < 0));
    }
    constexpr Fraction& operator+=(const Fraction& b) { return *this = *this + b; }
    constexpr Fraction& operator-=(const Fraction& b) { return *this = *this - b; }
    constexpr Fraction& operator*=(const Fraction& b) { return *this = *this * b; }
    constexpr Fraction& operator/=(const Fraction& b) { return *this = *this / b; }
    constexpr bool operator==(const Fraction&) const = default;
    constexpr std::strong_ordering operator<=>(const Fraction& b) const {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FRACTION_DISABLE_INT128)
        if (den_ == b.den_) return num_ <=> b.num_;
        __extension__ using SignedWide = __int128;
        const auto left = static_cast<SignedWide>(num_) * b.den_;
        const auto right = static_cast<SignedWide>(b.num_) * den_;
        return left <=> right;
#else
        if (den_ == b.den_) return num_ <=> b.num_;
        if ((num_ < 0) != (b.num_ < 0)) return num_ <=> b.num_;
        const auto left = Wide::product(magnitude(num_), static_cast<std::uint64_t>(b.den_));
        const auto right = Wide::product(magnitude(b.num_), static_cast<std::uint64_t>(den_));
        return num_ < 0 ? right <=> left : left <=> right;
#endif
    }
    friend std::ostream& operator<<(std::ostream& out, const Fraction& x) {
        return out << x.num_ << '/' << x.den_;
    }
    friend std::istream& operator>>(std::istream& in, Fraction& x) {
        std::int64_t n = 0, d = 1;
        if (!(in >> n)) return in;
        if (!in.eof() && in.peek() == '/') {
            in.get();
            if (!(in >> d)) return in;
        }
        try { x = Fraction(n, d); }
        catch (const std::domain_error&) { in.setstate(std::ios::failbit); }
        catch (const std::overflow_error&) { in.setstate(std::ios::failbit); }
        return in;
    }
};
#endif
