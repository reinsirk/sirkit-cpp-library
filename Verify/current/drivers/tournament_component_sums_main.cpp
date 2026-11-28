// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

namespace
{
struct Count
{
    static constexpr std::uint64_t kMod = 998244353;
    std::uint64_t value;
    explicit Count(std::uint64_t count) : value(count % kMod)
    {
    }
    friend Count operator+(Count a, Count b)
    {
        return Count(a.value + b.value);
    }
    friend Count operator*(Count a, Count b)
    {
        return Count(a.value * b.value);
    }
};
} // namespace

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    verify_out << sirkit::tournament_component_sums<Count>(n)[m].value << '\n';
}
