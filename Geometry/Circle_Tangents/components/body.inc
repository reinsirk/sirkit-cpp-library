#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_GEO_CIRCLE_TANGENTS_DEFINED
#define SIRKIT_GEO_CIRCLE_TANGENTS_DEFINED

namespace geo
{
struct CircleTangent
{
    RealPoint normal;
    RealPoint first;
    RealPoint second;
};

namespace tangent_detail
{
inline void RequireCircle(const Circle &circle)
{
    validate(circle);
    if (circle.radius == 0)
    {
        throw std::domain_error("circle tangents: radius must be positive");
    }
}

inline void AppendTangents(std::vector<CircleTangent> &result, const Circle &a, const Circle &b,
                           RealPoint unit, long double distance, long double signed_radius, Epsilon eps)
{
    const long double offset = a.radius - signed_radius;
    const long double gap = distance - std::abs(offset);
    const long double tolerance = eps.tolerance(std::max(distance, std::abs(offset)));
    if (gap < -tolerance)
    {
        return;
    }
    const auto append = [&](RealPoint normal)
    {
        result.push_back({normal, a.center + normal * a.radius, b.center + normal * signed_radius});
    };
    if (gap <= tolerance)
    {
        append(unit * std::copysign(1.0L, offset));
        return;
    }
    const long double along = offset / distance;
    const long double height = std::sqrt((gap / distance) * (1 + std::abs(along)));
    const RealPoint perpendicular{-unit.y * height, unit.x * height};
    append(unit * along + perpendicular);
    append(unit * along - perpendicular);
}
} // namespace tangent_detail

[[nodiscard]] inline std::vector<CircleTangent> point_circle_tangents(const Circle &circle, RealPoint point,
                                                                      Epsilon eps = {})
{
#ifndef SKIP_RUNTIME_CHECKS
    tangent_detail::RequireCircle(circle);
    if (!std::isfinite(point.x) || !std::isfinite(point.y))
    {
        throw std::domain_error("circle tangents: invalid point");
    }
#endif
    const auto delta = point - circle.center;
    const auto distance = delta.norm();
    if (distance == 0)
    {
        return {};
    }
    std::vector<CircleTangent> result;
    tangent_detail::AppendTangents(result, circle, Circle{point, 0}, delta / distance, distance, 0, eps);
    return result;
}

[[nodiscard]] inline std::optional<std::vector<CircleTangent>> circle_tangents(const Circle &a, const Circle &b,
                                                                               Epsilon eps = {})
{
#ifndef SKIP_RUNTIME_CHECKS
    tangent_detail::RequireCircle(a);
    tangent_detail::RequireCircle(b);
#endif
    const auto delta = b.center - a.center;
    const auto distance = delta.norm();
    if (distance == 0)
    {
        if (std::abs(a.radius - b.radius) <= eps.tolerance(std::max(a.radius, b.radius)))
        {
            return std::nullopt;
        }
        return std::vector<CircleTangent>{};
    }
    const auto unit = delta / distance;
    std::vector<CircleTangent> result;
    result.reserve(4);
    tangent_detail::AppendTangents(result, a, b, unit, distance, b.radius, eps);
    tangent_detail::AppendTangents(result, a, b, unit, distance, -b.radius, eps);
    return result;
}
} // namespace geo

#endif
