// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Pollard_Rho.cpp"
#include "../../../Math/Two_Square_Sum.cpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

int main()
{
    using Pairs = std::vector<std::pair<std::uint64_t, std::uint64_t>>;
    constexpr std::uint64_t kLimit = 30000;
    std::vector<Pairs> reference(kLimit + 1);
    for (std::uint64_t x = 0; x * x <= kLimit; ++x)
    {
        for (std::uint64_t y = 0, first = x * x; first + y * y <= kLimit; ++y)
        {
            reference[first + y * y].emplace_back(x, y);
        }
    }
    for (std::uint64_t n = 0; n <= kLimit; ++n)
    {
        assert(sirkit::two_square_sum(n) == reference[n]);
    }
    assert(sirkit::two_square_sum(std::numeric_limits<std::uint64_t>::max()).empty());
    constexpr std::uint64_t kPrime = 18446744073709551557ULL;
    const Pairs largest_prime{{1576450879, 3995190446}, {3995190446, 1576450879}};
    assert(sirkit::two_square_sum(kPrime) == largest_prime);
    assert((sirkit::two_square_sum(std::uint64_t{1} << 63) == Pairs{{1u << 31, 1u << 31}}));
    const std::uint64_t scale = 1073741824;
    const auto scaled = sirkit::two_square_sum(13 * scale * scale);
    assert((scaled == Pairs{{2 * scale, 3 * scale}, {3 * scale, 2 * scale}}));
}
