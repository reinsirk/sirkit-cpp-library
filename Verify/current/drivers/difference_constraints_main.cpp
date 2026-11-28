// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{}, target{};
    verify_in >> n >> m >> target;
    struct Point
    {
        std::int64_t a, b, radius;
    };
    std::vector<Point> points(n);
    for (auto &point : points)
    {
        verify_in >> point.a >> point.b >> point.radius;
    }
    std::vector<sirkit::DifferenceConstraint> constraints;
    constraints.reserve(2 * (n + m));
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        std::int64_t radius{};
        verify_in >> u >> v >> radius;
        constraints.push_back({u, v, radius});
        constraints.push_back({v, u, radius});
    }

    std::int64_t answer = 0;
    for (const int sign : {1, -1})
    {
        constraints.resize(2 * m);
        for (int i = 1; i <= n; ++i)
        {
            const auto [a, b, radius] = points[i - 1];
            const auto center = a + sign * b;
            constraints.push_back({0, i, center + radius});
            constraints.push_back({i, 0, radius - center});
        }
        const auto state = sirkit::solve_difference_constraints(n + 1, constraints);
        if (!state.feasible)
        {
            verify_out << -1 << '\n';
            return 0;
        }
        const auto &range = state.bounds[target];
        answer = std::max({answer, range.upper.value(), -range.lower.value()});
    }
    verify_out << answer << '\n';
}
