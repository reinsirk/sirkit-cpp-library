// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Affine
{
    std::uint32_t a, b;

    constexpr bool operator==(const Affine &) const noexcept = default;
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
    std::uint64_t n = 0;
    int q = 0;
    verify_in >> n >> q;
    SparseSegmentTree<Affine, Compose, Identity> data(n);
    while (q-- > 0)
    {
        int type = 0;
        std::uint64_t left = 0;
        verify_in >> type >> left;
        if (type == 0)
        {
            Affine value{};
            verify_in >> value.a >> value.b;
            data.set(left, value);
        }
        else
        {
            std::uint64_t right = 0;
            std::uint32_t x = 0;
            verify_in >> right >> x;
            const auto fold = data.prod(left, right);
            verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
        }
    }
}
