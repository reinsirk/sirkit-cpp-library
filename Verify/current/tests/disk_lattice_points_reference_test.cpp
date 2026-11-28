// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Disk_Lattice_Points.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

static void check(std::int64_t x, std::int64_t y, std::int64_t radius, std::int64_t unit)
{
    std::uint64_t closed = 0, boundary = 0;
    const auto rr = radius * radius;
    const auto reach = radius / unit + 2;
    for (auto column = x / unit - reach; column <= x / unit + reach; ++column)
    {
        for (auto row = y / unit - reach; row <= y / unit + reach; ++row)
        {
            const auto dx = column * unit - x, dy = row * unit - y;
            const auto distance = dx * dx + dy * dy;
            closed += static_cast<std::uint64_t>(distance <= rr);
            boundary += static_cast<std::uint64_t>(distance == rr);
        }
    }
    const auto answer = sirkit::geo::count_disk_lattice_points(x, y, radius, unit);
    assert(answer.closed == closed && answer.boundary == boundary);
}

int main()
{
    check(0, 0, 0, 1);
    check(0, 0, 15, 1);
    check(0, 0, 66, 3);
    check(1, 1, 1, 2);
    check(-2, 9, 0, 5);
    check(15, 20, 1, 30);
    check(1, 2, 1, 8);
    check(4, 5, 32, 7);
    check(-194, 281, 38, 19);

    std::mt19937 rng(31286);
    for (int i = 0; i < 1000; ++i)
    {
        const auto unit = std::int64_t{1} + static_cast<std::int64_t>(rng() % 20);
        const auto x = static_cast<std::int64_t>(rng() % 700) - 350;
        const auto y = static_cast<std::int64_t>(rng() % 700) - 350;
        check(x, y, rng() % 111, unit);
    }

    constexpr auto kLarge = std::numeric_limits<std::int64_t>::max();
    constexpr auto kSmall = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMaxRadius = std::int64_t{std::numeric_limits<std::int32_t>::max()};
    auto answer = sirkit::geo::count_disk_lattice_points(kLarge, kSmall, 1, kLarge);
    assert(answer.closed == 1 && answer.boundary == 1);
    answer = sirkit::geo::count_disk_lattice_points(kLarge, kSmall, 1);
    assert(answer.closed == 5 && answer.boundary == 4);
    answer = sirkit::geo::count_disk_lattice_points(0, kLarge / 2, kMaxRadius, kLarge);
    assert(answer.closed == 0 && answer.boundary == 0);
    answer = sirkit::geo::count_disk_lattice_points(kLarge, kSmall, kMaxRadius, kLarge);
    assert(answer.closed == 1 && answer.boundary == 0);
    answer = sirkit::geo::count_disk_lattice_points(kLarge, kSmall, kMaxRadius, kMaxRadius);
    assert(answer.closed == 3 && answer.boundary == 0);

    using Bad = std::array<std::int64_t, 2>;
    for (const auto &bad : {Bad{5, 0}, Bad{5, -1}, Bad{-1, 5}, Bad{kMaxRadius + 1, 5}})
    {
        bool caught = false;
        try
        {
            (void)sirkit::geo::count_disk_lattice_points(0, 0, bad[0], bad[1]);
        }
        catch (const std::invalid_argument &)
        {
            caught = true;
        }
        assert(caught);
    }
}
