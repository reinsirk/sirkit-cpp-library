// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Geometry/Delaunay_Triangulation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/delaunay_triangulation.md
//
// API一覧
// delaunay_triangulation(points): 平面の整数点からDelaunay三角形分割の辺を返す。
// DelaunayPoint = pair<int64_t, int64_t>。DelaunayEdge = pair<size_t, size_t> は元の0始まり添字。
// 座標が同じ点は最小添字で代表。共線だけなら座標順の隣接辺、異なる座標が2点未満なら空。
// 各軸の座標幅は2×10^9以下。超えるとoverflow_error。O(N log(N+1))時間・O(N)領域。
//
// [同梱のユークリッド最小全域木]
// euclidean_mst(points): 整数座標の完全グラフで、辺の重みをユークリッド距離とした最小全域木を返す。
// EuclideanMstEdge{from, to, distance_squared}: 元の点の0始まり添字と距離の二乗。
// 同じ点の間の距離は0。同距離の辺が複数あれば任意の最小全域木を返す。0点・1点では空。
// 各軸の座標幅は25,000以下。超える場合はoverflow_error。O(N log(N+1))時間・O(N)領域。
//
// [同梱の固定幅整数]
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


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EUCLIDEAN_MST_DEFINED
#define SIRKIT_EUCLIDEAN_MST_DEFINED

struct EuclideanMstEdge
{
    std::size_t from;
    std::size_t to;
    std::uint64_t distance_squared;
};

namespace detail
{
using EuclideanMstPoint = std::pair<std::int64_t, std::int64_t>;

inline EuclideanMstPoint EuclideanMstDifference(const EuclideanMstPoint &a, const EuclideanMstPoint &b)
{
    return {a.first - b.first, a.second - b.second};
}

inline std::int64_t EuclideanMstCross(const EuclideanMstPoint &a, const EuclideanMstPoint &b)
{
    return a.first * b.second - a.second * b.first;
}

inline std::int64_t EuclideanMstNormSquared(const EuclideanMstPoint &p)
{
    return p.first * p.first + p.second * p.second;
}

struct EuclideanMstCircleTest
{
    bool operator()(const EuclideanMstPoint &x, const EuclideanMstPoint &y, const EuclideanMstPoint &z) const
    {
        // 軸幅をWとすると絶対値は12W^4以下で、int64_tに収まる。
        return EuclideanMstNormSquared(x) * EuclideanMstCross(y, z) +
                   EuclideanMstNormSquared(y) * EuclideanMstCross(z, x) +
                   EuclideanMstNormSquared(z) * EuclideanMstCross(x, y) >
               0;
    }
};

template <class CircleTest = EuclideanMstCircleTest>
class EuclideanMstTriangulation
{
    using Index = std::size_t;
    static constexpr Index kNone = std::numeric_limits<Index>::max();

    struct HalfEdge
    {
        Index next;
        Index origin;
    };

public:
    explicit EuclideanMstTriangulation(std::span<const EuclideanMstPoint> points) : points_(points)
    {
        edges_.reserve(12 * points.size());
        Build(0, points.size());
    }

    std::vector<EuclideanMstEdge> WeightedEdges() const
    {
        std::vector<EuclideanMstEdge> result;
        result.reserve(edges_.size() / 4);
        // 構築途中も辺は交差しない。完成した三角形分割が最大辺数に達するため全ブロックが使用中。
        for (Index i = 0; i < edges_.size(); i += 4)
        {
            const auto from = Origin(i), to = Destination(i);
            const auto delta = EuclideanMstDifference(points_[from], points_[to]);
            result.push_back({from, to, static_cast<std::uint64_t>(EuclideanMstNormSquared(delta))});
        }
        return result;
    }

private:
    std::span<const EuclideanMstPoint> points_;
    std::vector<HalfEdge> edges_;
    Index free_ = kNone;

    static Index Reverse(Index edge)
    {
        return edge ^ Index{2};
    }

    static Index Rotate(Index edge)
    {
        return (edge & ~Index{3}) | ((edge + 1) & Index{3});
    }

    static Index RotateBack(Index edge)
    {
        return Reverse(Rotate(edge));
    }

    Index Origin(Index edge) const
    {
        return edges_[edge].origin;
    }

    Index Destination(Index edge) const
    {
        return Origin(Reverse(edge));
    }

    Index AroundOrigin(Index edge) const
    {
        return edges_[edge].next;
    }

    Index AroundOriginBack(Index edge) const
    {
        return Rotate(AroundOrigin(Rotate(edge)));
    }

    Index AroundLeftFace(Index edge) const
    {
        return Rotate(AroundOrigin(RotateBack(edge)));
    }

    Index Add(Index from, Index to)
    {
        const auto base = free_ == kNone ? edges_.size() : free_;
        const std::array<HalfEdge, 4> block{{{base, from}, {base + 3, kNone}, {base + 2, to}, {base + 1, kNone}}};
        if (free_ == kNone)
        {
            edges_.insert(edges_.end(), block.begin(), block.end());
        }
        else
        {
            free_ = edges_[base].next;
            std::ranges::copy(block, edges_.begin() + base);
        }
        return base;
    }

    void Splice(Index a, Index b)
    {
        const auto dual_a = Rotate(AroundOrigin(a));
        const auto dual_b = Rotate(AroundOrigin(b));
        std::swap(edges_[a].next, edges_[b].next);
        std::swap(edges_[dual_a].next, edges_[dual_b].next);
    }

    Index Connect(Index a, Index b)
    {
        const auto edge = Add(Destination(a), Origin(b));
        Splice(edge, AroundLeftFace(a));
        Splice(Reverse(edge), b);
        return edge;
    }

    void Erase(Index edge)
    {
        Splice(edge, AroundOriginBack(edge));
        const auto opposite = Reverse(edge);
        Splice(opposite, AroundOriginBack(opposite));
        const auto base = edge & ~Index{3};
        edges_[base] = {free_, kNone};
        free_ = base;
    }

    std::int64_t Side(Index edge, Index point) const
    {
        const auto &from = points_[Origin(edge)];
        const auto to = EuclideanMstDifference(points_[Destination(edge)], from);
        return EuclideanMstCross(to, EuclideanMstDifference(points_[point], from));
    }

    bool InCircle(Index a, Index b, Index c, Index test) const
    {
        const auto x = EuclideanMstDifference(points_[a], points_[test]);
        const auto y = EuclideanMstDifference(points_[b], points_[test]);
        const auto z = EuclideanMstDifference(points_[c], points_[test]);
        // abの左にcがあるとき、行列式が正ならtestは外接円の内側。
        return CircleTest{}(x, y, z);
    }

    Index FindBase(Index &left, Index &right)
    {
        for (;;)
        {
            if (Side(left, Origin(right)) > 0)
            {
                left = AroundLeftFace(left);
            }
            else if (Side(right, Origin(left)) < 0)
            {
                right = AroundOrigin(Reverse(right));
            }
            else
            {
                return Connect(Reverse(right), left);
            }
        }
    }

    Index BorderCandidate(Index base, bool right)
    {
        const auto advance = [&](Index edge)
        {
            return right ? AroundOriginBack(edge) : AroundOrigin(edge);
        };
        auto candidate = advance(right ? base : Reverse(base));
        if (Side(base, Destination(candidate)) < 0)
        {
            auto next = advance(candidate);
            while (InCircle(Destination(base), Origin(base), Destination(candidate), Destination(next)))
            {
                Erase(candidate);
                candidate = next;
                next = advance(candidate);
            }
        }
        return candidate;
    }

    std::pair<Index, Index> Build(Index first, Index last)
    {
        const auto size = last - first;
        if (size <= 3)
        {
            const auto a = Add(first, first + 1);
            if (size == 2)
            {
                return {a, Reverse(a)};
            }
            const auto b = Add(first + 1, first + 2);
            Splice(Reverse(a), b);
            const auto turn = Side(a, first + 2);
            if (turn == 0)
            {
                return {a, Reverse(b)};
            }
            const auto close = Connect(b, a);
            if (turn > 0)
            {
                return {a, Reverse(b)};
            }
            return {Reverse(close), close};
        }

        const auto middle = first + size / 2;
        auto [outer_left, inner_left] = Build(first, middle);
        auto [inner_right, outer_right] = Build(middle, last);
        auto base = FindBase(inner_left, inner_right);
        if (Origin(inner_left) == Origin(outer_left))
        {
            outer_left = Reverse(base);
        }
        if (Origin(inner_right) == Origin(outer_right))
        {
            outer_right = base;
        }

        // 下側の共通接線から上へ進み、空円条件を満たす三角形で左右をつなぐ。
        for (;;)
        {
            const auto left = BorderCandidate(base, false);
            const auto right = BorderCandidate(base, true);
            const bool valid_left = Side(base, Destination(left)) < 0;
            const bool valid_right = Side(base, Destination(right)) < 0;
            if (!valid_left && !valid_right)
            {
                return {outer_left, outer_right};
            }
            if (!valid_left ||
                (valid_right && InCircle(Destination(left), Origin(left), Origin(right), Destination(right))))
            {
                base = Connect(right, Reverse(base));
            }
            else
            {
                base = Connect(Reverse(base), Reverse(left));
            }
        }
    }
};

inline void AddEuclideanMstEdges(std::span<const EuclideanMstPoint> points, std::span<const std::size_t> indices,
                                 std::vector<EuclideanMstEdge> &result)
{
    auto edges = EuclideanMstTriangulation(points).WeightedEdges();
    std::ranges::sort(edges, {}, &EuclideanMstEdge::distance_squared);

    std::vector<std::size_t> parent(points.size()), component_size(points.size(), 1);
    std::iota(parent.begin(), parent.end(), std::size_t{0});
    const auto find = [&](std::size_t v)
    {
        while (parent[v] != v)
        {
            parent[v] = parent[parent[v]];
            v = parent[v];
        }
        return v;
    };
    for (const auto &edge : edges)
    {
        auto u = find(edge.from), v = find(edge.to);
        if (u != v)
        {
            if (component_size[u] < component_size[v])
            {
                std::swap(u, v);
            }
            parent[v] = u;
            component_size[u] += component_size[v];
            result.push_back({indices[edge.from], indices[edge.to], edge.distance_squared});
        }
    }
}
} // namespace detail

inline std::vector<EuclideanMstEdge> euclidean_mst(std::span<const std::pair<std::int64_t, std::int64_t>> points)
{
    if (points.empty())
    {
        return {};
    }

    constexpr std::uint64_t kMaxSpan = 25'000;
    const auto min_x = std::ranges::min_element(points, {}, &detail::EuclideanMstPoint::first)->first;
    const auto min_y = std::ranges::min_element(points, {}, &detail::EuclideanMstPoint::second)->second;
    const auto translate = [&](std::int64_t coordinate, std::int64_t minimum)
    {
        const auto width = static_cast<std::uint64_t>(coordinate) - static_cast<std::uint64_t>(minimum);
        if (width > kMaxSpan)
        {
            throw std::overflow_error("euclidean_mst: coordinate range is too wide");
        }
        return static_cast<std::int64_t>(width);
    };

    std::vector<std::size_t> order(points.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::ranges::sort(order, {}, [&](std::size_t i)
                      {
                          return std::pair(points[i], i);
                      });

    std::vector<EuclideanMstEdge> result;
    std::vector<detail::EuclideanMstPoint> unique;
    std::vector<std::size_t> indices;
    result.reserve(points.size() - 1);
    unique.reserve(points.size());
    indices.reserve(points.size());
    for (const auto i : order)
    {
        const auto &[x, y] = points[i];
        const detail::EuclideanMstPoint point{translate(x, min_x), translate(y, min_y)};
        if (unique.empty() || unique.back() != point)
        {
            unique.push_back(point);
            indices.push_back(i);
        }
        else
        {
            result.push_back({indices.back(), i, 0});
        }
    }

    if (unique.size() >= 2)
    {
        detail::AddEuclideanMstEdges(unique, indices, result);
    }
    return result;
}

#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <compare>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <concepts>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <istream>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <ostream>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_FRACTION_DETAIL_DEFINED
#define SIRKIT_FRACTION_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {

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
    using Wide = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FractionWide;
    std::int64_t num_ = 0, den_ = 1;
    static constexpr auto max_ = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());

    static constexpr std::uint64_t magnitude(std::int64_t n) {
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_magnitude(n);
    }
    static constexpr Fraction result(Wide n, Wide d, bool negative) {
        if (!n.fits(max_ + static_cast<unsigned>(negative)) || !d.fits(max_))
            throw std::overflow_error("Fraction: result out of int64_t range");
        Fraction r;
        const auto u = n.low();
        // 負号を指定する呼出しは、分子が0の場合を処理済み。
        r.num_ = negative ? -static_cast<std::int64_t>(u - 1) - 1 : static_cast<std::int64_t>(u);
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
            g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(da, db);
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
        const auto h = g == 1 ? 1 : /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(n.divmod(g).second, g);
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
        const auto g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(u, v);
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
        const auto g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(na, db), h = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(nb, da);
        return result(Wide::product(na / g, nb / h), Wide::product(da / h, db / g), (a.num_ < 0) != (b.num_ < 0));
    }
    friend constexpr Fraction operator/(const Fraction& a, const Fraction& b) {
        if (b.num_ == 0) throw std::domain_error("Fraction: division by zero");
        if (a.num_ == 0) return {};
        const auto na = magnitude(a.num_), nb = magnitude(b.num_);
        const auto da = static_cast<std::uint64_t>(a.den_), db = static_cast<std::uint64_t>(b.den_);
        const auto g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(na, nb), h = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(da, db);
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


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <numeric>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <span>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_DELAUNAY_TRIANGULATION_DEFINED
#define SIRKIT_DELAUNAY_TRIANGULATION_DEFINED

using DelaunayPoint = std::pair<std::int64_t, std::int64_t>;
using DelaunayEdge = std::pair<std::size_t, std::size_t>;

namespace detail
{
struct DelaunayCircleTest
{
    bool operator()(const DelaunayPoint &x, const DelaunayPoint &y, const DelaunayPoint &z) const
    {
        FractionWide positive, negative;
        const auto add_term = [&](const DelaunayPoint &a, const DelaunayPoint &b, const DelaunayPoint &c)
        {
            const auto cross = EuclideanMstCross(b, c);
            const auto term = FractionWide::product(static_cast<std::uint64_t>(EuclideanMstNormSquared(a)),
                                                    fraction_magnitude(cross));
            auto &accumulator = cross < 0 ? negative : positive;
            accumulator = accumulator + term;
        };
        add_term(x, y, z);
        add_term(y, z, x);
        add_term(z, x, y);
        // 座標幅Wのとき、どちらの合計も12W^4以下で符号なし128ビットに収まる。
        return positive > negative;
    }
};
} // namespace detail

[[nodiscard]] inline std::vector<DelaunayEdge> delaunay_triangulation(std::span<const DelaunayPoint> points)
{
    if (points.size() < 2)
    {
        return {};
    }
    std::vector<std::size_t> order(points.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::ranges::sort(order, {}, [&](std::size_t i)
                      {
                          return std::pair(points[i], i);
                      });

    const auto min_x = points[order.front()].first;
    const auto min_y = std::ranges::min_element(points, {}, &DelaunayPoint::second)->second;
    const auto translate = [](std::int64_t coordinate, std::int64_t minimum)
    {
        constexpr std::uint64_t kMaxSpan = 2'000'000'000;
        const auto width = static_cast<std::uint64_t>(coordinate) - static_cast<std::uint64_t>(minimum);
        if (width > kMaxSpan)
        {
            throw std::overflow_error("delaunay_triangulation: coordinate range is too wide");
        }
        return static_cast<std::int64_t>(width);
    };
    std::vector<DelaunayPoint> unique;
    unique.reserve(order.size());
    std::size_t distinct = 0;
    for (const auto i : order)
    {
        if (distinct == 0 || points[i] != points[order[distinct - 1]])
        {
            const auto [x, y] = points[i];
            unique.emplace_back(translate(x, min_x), translate(y, min_y));
            order[distinct++] = i;
        }
    }
    if (distinct < 2)
    {
        return {};
    }

    const auto candidates = detail::EuclideanMstTriangulation<detail::DelaunayCircleTest>(unique).WeightedEdges();
    std::vector<DelaunayEdge> result;
    result.reserve(candidates.size());
    for (const auto &edge : candidates)
    {
        const auto [a, b] = std::minmax(order[edge.from], order[edge.to]);
        result.emplace_back(a, b);
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
