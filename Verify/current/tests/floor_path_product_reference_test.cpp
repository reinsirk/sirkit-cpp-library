// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Floor_Path_Product.cpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

int main()
{
    using sirkit::floor_path_product;
    const auto concatenate = [](const std::string &first, const std::string &second)
    {
        return first + second;
    };
    const std::string x("X"), y("Y"), identity;
    try
    {
        static_cast<void>(floor_path_product(3, 0, 1, 1, x, y, identity, concatenate));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    assert(floor_path_product(3, 5, 2, 4, x, y, identity, concatenate) == "XYXXY");
    for (std::uint32_t n = 0; n < 16; ++n)
    {
        for (std::uint32_t m = 1; m < 16; ++m)
        {
            for (std::uint64_t a = 0; a < 18; ++a)
            {
                for (std::uint64_t b = 0; b < 18; ++b)
                {
                    std::string expected(b / m, 'Y');
                    for (std::uint32_t i = 0; i < n; ++i)
                    {
                        expected += 'X';
                        expected.append((a * (i + 1) + b) / m - (a * i + b) / m, 'Y');
                    }
                    assert(floor_path_product(n, m, a, b, x, y, identity, concatenate) == expected);
                }
            }
        }
    }

    using Count = std::array<std::uint64_t, 2>;
    const auto sum = [](const Count &a, const Count &b)
    {
        return Count{a[0] + b[0], a[1] + b[1]};
    };
    using boost::multiprecision::cpp_int;
    constexpr auto kN = std::numeric_limits<std::uint32_t>::max();
    constexpr auto kA = std::numeric_limits<std::uint64_t>::max();
    const cpp_int modulus = cpp_int(1) << 64;
    for (const auto slope : {kA, kA - 1, static_cast<std::uint64_t>(kN - 1)})
    {
        const auto counted = floor_path_product(kN, kN, slope, kA, Count{1, 0}, Count{0, 1}, Count{0, 0}, sum);
        const cpp_int height = (cpp_int(slope) * kN + kA) / kN % modulus;
        assert(counted[0] == kN && counted[1] == height.convert_to<std::uint64_t>());
    }
}
