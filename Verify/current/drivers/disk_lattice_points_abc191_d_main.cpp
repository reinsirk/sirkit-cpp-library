// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <string>
#include <string_view>

static std::int64_t read_scaled_decimal(std::string_view text)
{
    const bool negative = text.starts_with('-');
    if (negative || text.starts_with('+'))
    {
        text.remove_prefix(1);
    }
    std::int64_t value = 0;
    int fractional_digits = 0;
    bool fractional = false;
    for (char c : text)
    {
        if (c == '.')
        {
            fractional = true;
        }
        else
        {
            value = 10 * value + (c - '0');
            fractional_digits += fractional;
        }
    }
    for (; fractional_digits < 4; ++fractional_digits)
    {
        value *= 10;
    }
    return negative ? -value : value;
}

int main()
{
    std::string x, y, radius;
    verify_in >> x >> y >> radius;
    constexpr std::int64_t kUnit = 10000;
    const auto points = sirkit::geo::count_disk_lattice_points(read_scaled_decimal(x), read_scaled_decimal(y),
                                                               read_scaled_decimal(radius), kUnit);
    verify_out << points.closed << '\n';
}
