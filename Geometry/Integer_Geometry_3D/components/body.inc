#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>

#ifndef SIRKIT_INTEGER_GEOMETRY_3D_DEFINED
#define SIRKIT_INTEGER_GEOMETRY_3D_DEFINED

struct Point3i
{
    std::int64_t x, y, z;
};

enum class PrismPosition
{
    kOutside,
    kBoundary,
    kInside,
    kDegenerate
};

namespace detail
{
using Point3Vector = std::array<std::int64_t, 3>;

inline void CheckPoint3([[maybe_unused]] Point3i a)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = 1000000000;
    for (const auto coordinate : {a.x, a.y, a.z})
    {
        if (coordinate < -kLimit || coordinate > kLimit)
        {
            throw std::out_of_range("Point3i: coordinate out of range");
        }
    }
#endif
}

[[nodiscard]] inline Point3Vector Point3Diff(Point3i a, Point3i b) noexcept
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

[[nodiscard]] inline Point3Vector Point3Cross(const Point3Vector &a, const Point3Vector &b) noexcept
{
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

struct Point3Dot
{
    FractionWide positive, negative;

    [[nodiscard]] int sign() const noexcept
    {
        return (positive > negative) - (positive < negative);
    }

    [[nodiscard]] double to_double() const noexcept
    {
        const auto convert = [](FractionWide magnitude) -> double
        {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FRACTION_DISABLE_INT128)
            return static_cast<double>(magnitude.value);
#else
            return std::ldexp(static_cast<double>(magnitude.hi), 64) + static_cast<double>(magnitude.lo);
#endif
        };
        return positive < negative ? -convert(negative - positive) : convert(positive - negative);
    }
};

[[nodiscard]] inline Point3Dot WidePoint3Dot(const Point3Vector &a, const Point3Vector &b) noexcept
{
    Point3Dot result;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        const auto product = FractionWide::product(fraction_magnitude(a[i]), fraction_magnitude(b[i]));
        auto &sum = (a[i] < 0) == (b[i] < 0) ? result.positive : result.negative;
        sum = sum + product;
    }
    return result;
}

[[nodiscard]] inline Point3Vector Point3Normal(Point3i a, Point3i b, Point3i c, Point3i p)
{
    CheckPoint3(a);
    CheckPoint3(b);
    CheckPoint3(c);
    CheckPoint3(p);
    return Point3Cross(Point3Diff(b, a), Point3Diff(c, a));
}
}

[[nodiscard]] inline int orientation_3d(Point3i a, Point3i b, Point3i c, Point3i p)
{
    const auto normal = detail::Point3Normal(a, b, c, p);
    return detail::WidePoint3Dot(normal, detail::Point3Diff(p, a)).sign();
}

[[nodiscard]] inline PrismPosition triangular_prism_position(Point3i a, Point3i b, Point3i c, Point3i p)
{
    const auto normal = detail::Point3Normal(a, b, c, p);
    if (normal == detail::Point3Vector{})
    {
        return PrismPosition::kDegenerate;
    }
    const std::array<Point3i, 3> vertices{a, b, c};
    bool boundary = false;
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const auto start = vertices[i], end = vertices[(i + 1) % vertices.size()];
        const auto edge_cross = detail::Point3Cross(detail::Point3Diff(end, start), detail::Point3Diff(p, start));
        const auto side = detail::WidePoint3Dot(normal, edge_cross).sign();
        if (side < 0)
        {
            return PrismPosition::kOutside;
        }
        boundary |= side == 0;
    }
    return boundary ? PrismPosition::kBoundary : PrismPosition::kInside;
}

[[nodiscard]] inline std::optional<double> signed_plane_distance(Point3i a, Point3i b, Point3i c, Point3i p)
{
    const auto normal = detail::Point3Normal(a, b, c, p);
    if (normal == detail::Point3Vector{})
    {
        return std::nullopt;
    }
    const auto signed_numerator = detail::WidePoint3Dot(normal, detail::Point3Diff(p, a));
    const double x = static_cast<double>(normal[0]), y = static_cast<double>(normal[1]), z = static_cast<double>(normal[2]);
    return signed_numerator.to_double() / std::sqrt(x * x + y * y + z * z);
}

#endif
