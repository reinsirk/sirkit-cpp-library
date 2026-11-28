// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Polygon_Chord_Moments.cpp
// 参考: https://atcoder.jp/contests/abc472/editorial/24789
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polygon_chord_moments.md
//
// API一覧
// PolygonChordMoments(points): 境界順の整数頂点を前処理。O(n)時間・領域。
// slice(from,to): fromからtoまで入力の順に進み、to→fromの辺で閉じる部分の符号付き面積・重心。O(1)。
// total(): 元の多角形の符号付き面積・重心。O(1)。size(): 頂点数。O(1)。
// 結果のsigned_area2は符号付き面積の2倍（正確なint64_t）、centroidは重心座標。面積0はnullopt。
// 頂点はpair<int64_t,int64_t>で|座標|<=10^9。凸多角形を周回順に渡す。時計回りも可。
// slice(i,i)は面積0。最後の頂点に先頭を再掲しない。丸めは重心計算時のみ。
// コピーはO(1)で前処理を共有。移動元は空。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_POLYGON_CHORD_MOMENTS_DEFINED
#define SIRKIT_POLYGON_CHORD_MOMENTS_DEFINED

namespace detail
{
struct PolygonMomentInteger
{
    std::uint64_t low = 0;
    std::uint64_t high = 0;

    PolygonMomentInteger operator+(const PolygonMomentInteger &other) const noexcept
    {
        const auto sum = low + other.low;
        return {sum, high + other.high + static_cast<unsigned>(sum < low)};
    }

    PolygonMomentInteger operator-(const PolygonMomentInteger &other) const noexcept
    {
        return {low - other.low, high - other.high - static_cast<unsigned>(low < other.low)};
    }

    static PolygonMomentInteger FromSigned(std::int64_t value) noexcept
    {
        return {static_cast<std::uint64_t>(value),
                value < 0 ? std::numeric_limits<std::uint64_t>::max() : 0};
    }

    static PolygonMomentInteger Product(std::int64_t a, std::int64_t b) noexcept
    {
        const auto x = Magnitude(a);
        const auto y = Magnitude(b); // |b| <= 2e9
        const auto low_product = (x & 0xffffffffULL) * y;
        const auto high_product = (x >> 32) * y;
        const auto product = PolygonMomentInteger{low_product, 0} +
                             PolygonMomentInteger{high_product << 32, high_product >> 32};
        return (a < 0) == (b < 0) ? product : PolygonMomentInteger{} - product;
    }

    std::int64_t ToSigned() const
    {
        const auto value = std::bit_cast<std::int64_t>(low);
        if (high != FromSigned(value).high)
        {
            throw std::overflow_error("PolygonChordMoments: twice area outside int64");
        }
        return value;
    }

    double ToDouble() const noexcept
    {
        const bool negative = (high >> 63) != 0;
        const auto magnitude = negative ? PolygonMomentInteger{} - *this : *this;
        const double result = std::ldexp(static_cast<double>(magnitude.high), 64) +
                              static_cast<double>(magnitude.low);
        return negative ? -result : result;
    }

private:
    static std::uint64_t Magnitude(std::int64_t value) noexcept
    {
        const auto bits = static_cast<std::uint64_t>(value);
        return value < 0 ? std::uint64_t{0} - bits : bits;
    }
};
}

class PolygonChordMoments
{
public:
    using Point = std::pair<std::int64_t, std::int64_t>;

    struct Result
    {
        std::int64_t signed_area2;
        std::optional<std::pair<double, double>> centroid;
    };

    explicit PolygonChordMoments(std::span<const Point> points)
        : state_(std::make_shared<const Storage>(points))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return state_ ? state_->points.size() : 0;
    }

    [[nodiscard]] Result total() const
    {
        return state_ ? Summarize(state_->prefix.back()) : Result{0, std::nullopt};
    }

    [[nodiscard]] Result slice(std::size_t from, std::size_t to) const
    {
        const auto &prefix = state_->prefix;
        auto moments = prefix[to] - prefix[from];
        if (from > to)
        {
            moments = moments + prefix.back();
        }
        return Summarize(moments + EdgeMoments(state_->points[to], state_->points[from]));
    }

private:
    using Integer = detail::PolygonMomentInteger;

    struct Moments
    {
        Integer area;
        Integer x;
        Integer y;

        Moments operator+(const Moments &other) const noexcept
        {
            return {area + other.area, x + other.x, y + other.y};
        }

        Moments operator-(const Moments &other) const noexcept
        {
            return {area - other.area, x - other.x, y - other.y};
        }
    };

    struct Storage
    {
        std::vector<Point> points;
        std::vector<Moments> prefix;

        explicit Storage(std::span<const Point> input) : points(input.begin(), input.end())
        {
            for (const auto &[x, y] : points)
            {
                CheckCoordinate(x);
                CheckCoordinate(y);
            }
            prefix.reserve(points.size() + 1);
            prefix.emplace_back();
            for (std::size_t to = 1; to < points.size(); ++to)
            {
                prefix.push_back(prefix.back() + EdgeMoments(points[to - 1], points[to]));
            }
            if (!points.empty())
            {
                prefix.push_back(prefix.back() + EdgeMoments(points.back(), points.front()));
            }
        }
    };

    std::shared_ptr<const Storage> state_;

    static void CheckCoordinate(std::int64_t value)
    {
        constexpr std::int64_t kLimit = 1000000000;
        if (value < -kLimit || value > kLimit)
        {
            throw std::out_of_range("PolygonChordMoments: coordinate magnitude");
        }
    }

    static Moments EdgeMoments(const Point &from, const Point &to) noexcept
    {
        const auto cross = from.first * to.second - from.second * to.first;
        return {Integer::FromSigned(cross),
                Integer::Product(cross, from.first + to.first),
                Integer::Product(cross, from.second + to.second)};
    }

    static Result Summarize(const Moments &moments)
    {
        const auto area = moments.area.ToSigned();
        if (area == 0)
        {
            return {0, std::nullopt};
        }
        const double divisor = 3 * static_cast<double>(area);
        return {area, std::pair{moments.x.ToDouble() / divisor, moments.y.ToDouble() / divisor}};
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
