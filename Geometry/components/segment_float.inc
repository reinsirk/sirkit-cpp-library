#ifndef SIRKIT_GEO_SEGMENT_FLOAT_DEFINED
#define SIRKIT_GEO_SEGMENT_FLOAT_DEFINED
namespace detail {

template<class C>
inline constexpr bool kExactFloatingSegment =
    std::is_floating_point_v<C> && std::numeric_limits<C>::radix == 2;

struct SegmentScaledMagnitude {
    long double fraction = 0;
    int exponent = 0;
    int sign = 0;
};

// 二進浮動小数点の積和を、共通の二進桁に揃えて加減算する。
template<class C>
class SegmentDeterminant {
    static constexpr int kFirstBit = 2 * (std::numeric_limits<C>::min_exponent - std::numeric_limits<C>::digits)
                                   - std::numeric_limits<long double>::digits - 64;
    static constexpr int kWords = (2 * std::numeric_limits<C>::max_exponent + 8 - kFirstBit + 31) / 32;
    using Words = std::array<std::uint32_t, kWords>;
    Words positive_{}, negative_{};
    int first_ = kWords;
    int last_ = -1;

    void accumulate(Words& words, long double value, int scale) {
        int exponent;
        long double fraction = std::frexp(value, &exponent);
        for (int bit = exponent + scale - kFirstBit - 32; fraction != 0; bit -= 32) {
            fraction = std::ldexp(fraction, 32);
            const auto chunk = static_cast<std::uint32_t>(fraction);
            fraction -= chunk;
            if (chunk == 0) continue;
            int word = bit / 32;
            first_ = std::min(first_, word);
            std::uint64_t carry = std::uint64_t{chunk} << (bit % 32);
            do {
                const auto sum = std::uint64_t{words[word]} + static_cast<std::uint32_t>(carry);
                words[word] = static_cast<std::uint32_t>(sum);
                carry = (carry >> 32) + (sum >> 32);
                last_ = std::max(last_, word++);
            } while (carry != 0);
        }
    }

    void add_product(C x, C y, bool subtract) {
        if (x == 0 || y == 0) return;
        int x_scale, y_scale;
        const long double a = std::frexp(static_cast<long double>(x), &x_scale);
        const long double b = std::frexp(static_cast<long double>(y), &y_scale);
        const long double high = a * b;
        // 仮数同士の積は通常範囲にあり、FMAで丸め前の残差を取り出せる。
        const long double low = std::fma(a, b, -high);
        const auto add_term = [&](long double term) {
            auto& words = (term < 0) != subtract ? negative_ : positive_;
            accumulate(words, std::abs(term), x_scale + y_scale);
        };
        add_term(high);
        add_term(low);
    }

    SegmentScaledMagnitude value(long double tolerance) {
        int top = last_;
        while (top >= first_ && positive_[top] == negative_[top]) --top;
        if (top < first_) return {};
        const bool negative = positive_[top] < negative_[top];
        auto& magnitude = negative ? negative_ : positive_;
        auto& other = negative ? positive_ : negative_;
        std::uint64_t borrow = 0;
        for (int i = first_; i <= top; ++i) {
            const std::uint64_t minuend = magnitude[i];
            const std::uint64_t subtrahend = std::uint64_t{other[i]} + borrow;
            magnitude[i] = static_cast<std::uint32_t>(minuend - subtrahend);
            borrow = minuend < subtrahend;
        }
        while (magnitude[top] == 0) --top;
        if (tolerance > 0) {
            if (!std::isfinite(tolerance)) return {};
            int exponent;
            std::frexp(tolerance, &exponent);
            int top_exponent;
            std::frexp(static_cast<long double>(magnitude[top]), &top_exponent);
            const int magnitude_exponent = kFirstBit + top * 32 + top_exponent;
            if (magnitude_exponent < exponent) return {};
            if (magnitude_exponent == exponent) {
                other.fill(0);
                accumulate(other, tolerance, 0);
                int i = top;
                while (i >= first_ && magnitude[i] == other[i]) --i;
                if (i < first_ || magnitude[i] < other[i]) return {};
            }
        }
        // 座標を近似する際は先頭の仮数だけを使用し、指数は別に保持する。
        constexpr int kLeadingWords = (std::numeric_limits<long double>::digits + 31) / 32;
        const int bottom = std::max(first_, top - kLeadingWords);
        long double leading = magnitude[top];
        for (int i = top - 1; i >= bottom; --i) leading = std::ldexp(leading, 32) + magnitude[i];
        int exponent;
        const long double fraction = std::frexp(leading, &exponent);
        return {fraction, exponent + kFirstBit + bottom * 32, negative ? -1 : 1};
    }

public:
    template<class T>
    SegmentScaledMagnitude orientation(const geo::Point<T, C>& a, const geo::Point<T, C>& b,
                                       const geo::Point<T, C>& c, long double tolerance = 0) {
        const C ax = C(a.x), ay = C(a.y), bx = C(b.x), by = C(b.y), cx = C(c.x), cy = C(c.y);
        add_product(bx, cy, false);
        add_product(bx, ay, true);
        add_product(ax, cy, true);
        add_product(by, cx, true);
        add_product(by, ax, false);
        add_product(ay, cx, false);
        return value(tolerance);
    }
};

template<class T, class C>
int floating_segment_side(const geo::Point<T, C>& a, const geo::Point<T, C>& b,
                          const geo::Point<T, C>& c, geo::Epsilon eps) {
    const C p = (C(b.x) - C(a.x)) * (C(c.y) - C(a.y));
    const C q = (C(b.y) - C(a.y)) * (C(c.x) - C(a.x));
    const long double determinant = static_cast<long double>(p) - static_cast<long double>(q);
    const long double scale = std::abs(static_cast<long double>(p)) + std::abs(static_cast<long double>(q));
    const long double tolerance = std::is_integral_v<T> ? 0 : eps.rel == 0 ? eps.abs : eps.tolerance(scale);
    // 差と積の丸め、非正規数の絶対誤差を含む保守的な幅。
    const long double error = 16 * std::numeric_limits<C>::epsilon() * scale
                            + 8 * static_cast<long double>(std::numeric_limits<C>::min());
    if (std::abs(std::abs(determinant) - tolerance) > error)
        return (determinant > tolerance) - (determinant < -tolerance);
    return SegmentDeterminant<C>{}.orientation(a, b, c, tolerance).sign;
}

template<class T, class C>
std::array<int, 4> floating_segment_sides(const geo::Segment<T, C>& s, const geo::Segment<T, C>& t,
                                         geo::Epsilon eps) {
    return {floating_segment_side(t.a, t.b, s.a, eps), floating_segment_side(t.a, t.b, s.b, eps),
            floating_segment_side(s.a, s.b, t.a, eps), floating_segment_side(s.a, s.b, t.b, eps)};
}

inline bool segment_proper_crossing(const std::array<int, 4>& sides) {
    return sides[0] * sides[1] < 0 && sides[2] * sides[3] < 0;
}

template<class T, class C>
bool floating_segment_endpoint(const geo::Point<T, C>& p, const geo::Segment<T, C>& s, int side,
                               geo::Epsilon eps) {
    if (side != 0) return false;
    if (std::is_integral_v<T> || (eps.abs == 0 && eps.rel == 0)) {
        return std::min(s.a.x, s.b.x) <= p.x && p.x <= std::max(s.a.x, s.b.x) &&
               std::min(s.a.y, s.b.y) <= p.y && p.y <= std::max(s.a.y, s.b.y);
    }
    const C x = (C(p.x) - C(s.a.x)) * (C(p.x) - C(s.b.x));
    const C y = (C(p.y) - C(s.a.y)) * (C(p.y) - C(s.b.y));
    return geo::predicate_sign<T, C>(x + y, std::abs(x) + std::abs(y), eps) <= 0;
}

template<class T, class C>
std::array<bool, 4> floating_segment_contacts(const geo::Segment<T, C>& s, const geo::Segment<T, C>& t,
                                             const std::array<int, 4>& sides, geo::Epsilon eps) {
    return {floating_segment_endpoint(s.a, t, sides[0], eps), floating_segment_endpoint(s.b, t, sides[1], eps),
            floating_segment_endpoint(t.a, s, sides[2], eps), floating_segment_endpoint(t.b, s, sides[3], eps)};
}

template<class T, class C>
SegmentScaledMagnitude floating_segment_magnitude(const geo::Point<T, C>& a, const geo::Point<T, C>& b,
                                                   const geo::Point<T, C>& c) {
    const auto convert = [](T value) { return static_cast<long double>(C(value)); };
    const long double ux = convert(b.x) - convert(a.x), uy = convert(b.y) - convert(a.y);
    const long double vx = convert(c.x) - convert(a.x), vy = convert(c.y) - convert(a.y);
    const long double p = ux * vy, q = uy * vx;
    const long double magnitude = std::abs(p - q);
    const long double scale = std::abs(p) + std::abs(q);
    // 交点の比に使う仮数は、桁落ちが小さい場合だけ通常演算から得る。
    if (magnitude > scale * 0x1p-12L && scale >= std::numeric_limits<long double>::min()) {
        int exponent;
        const long double fraction = std::frexp(magnitude, &exponent);
        return {fraction, exponent, 1};
    }
    return SegmentDeterminant<C>{}.orientation(a, b, c);
}

template<class T, class C>
geo::RealPoint floating_segment_crossing(const geo::Segment<T, C>& s, const geo::Segment<T, C>& t) {
    const auto a = floating_segment_magnitude(t.a, t.b, s.a);
    const auto b = floating_segment_magnitude(t.a, t.b, s.b);
    // 端点の向きが異符号なので、媒介変数の分母は絶対値の和になる。
    const bool reverse = a.exponent > b.exponent || (a.exponent == b.exponent && a.fraction > b.fraction);
    const auto& near = reverse ? b : a;
    const auto& far = reverse ? a : b;
    const long double scaled_near = std::ldexp(near.fraction, near.exponent - far.exponent);
    const long double ratio = scaled_near / (far.fraction + scaled_near);
    const auto& start = reverse ? s.b : s.a;
    const auto& end = reverse ? s.a : s.b;
    return {std::lerp(static_cast<long double>(C(start.x)), static_cast<long double>(C(end.x)), ratio),
            std::lerp(static_cast<long double>(C(start.y)), static_cast<long double>(C(end.y)), ratio)};
}

}
#endif
