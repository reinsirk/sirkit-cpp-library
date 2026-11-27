// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Affine
{
    std::uint32_t a, b;
};

constexpr Affine Compose(Affine left, Affine right) noexcept
{
    return {
        static_cast<std::uint32_t>(std::uint64_t{right.a} * left.a % kMod),
        static_cast<std::uint32_t>((std::uint64_t{right.a} * left.b + right.b) % kMod),
    };
}

constexpr Affine Identity() noexcept
{
    return {1, 0};
}
}

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Affine> values(n);
    for (auto &value : values)
    {
        verify_in >> value.a >> value.b;
    }
    RangeAssignRangeProduct<Affine, Compose, Identity> data(values);
    while (q-- > 0)
    {
        int type = 0;
        std::uint64_t left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            Affine value{};
            verify_in >> value.a >> value.b;
            data.assign(left, right, value);
        }
        else
        {
            std::uint32_t x = 0;
            verify_in >> x;
            const auto fold = data.prod(left, right);
            verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
        }
    }
}
