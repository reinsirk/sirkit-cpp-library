// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Matrix
{
    std::uint32_t a, b, c, d;
    bool operator==(const Matrix &) const = default;
};

constexpr std::uint32_t Dot(std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d)
{
    return static_cast<std::uint32_t>((std::uint64_t{a} * b + std::uint64_t{c} * d) % kMod);
}

constexpr Matrix Product(const Matrix &a, const Matrix &b)
{
    return {
        Dot(a.a, b.a, a.b, b.c),
        Dot(a.a, b.b, a.b, b.d),
        Dot(a.c, b.a, a.d, b.c),
        Dot(a.c, b.b, a.d, b.d),
    };
}

constexpr Matrix Identity()
{
    return {1, 0, 0, 1};
}

constexpr std::uint32_t Negate(std::uint32_t value)
{
    return value == 0 ? 0 : kMod - value;
}

constexpr Matrix Inverse(const Matrix &value)
{
    return {value.d, Negate(value.b), Negate(value.c), value.a};
}
}

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    NoncommutativeWeightedUnionFind<Matrix, Product, Identity, Inverse> uf(n);
    while (q-- > 0)
    {
        int type = 0, u = 0, v = 0;
        verify_in >> type >> u >> v;
        if (type == 0)
        {
            Matrix relation{};
            verify_in >> relation.a >> relation.b >> relation.c >> relation.d;
            verify_out << static_cast<int>(uf.constrain(v, u, relation)) << '\n';
        }
        else
        {
            const auto result = uf.try_diff(v, u);
            if (result)
            {
                verify_out << result->a << ' ' << result->b << ' ' << result->c << ' ' << result->d << '\n';
            }
            else
            {
                verify_out << -1 << '\n';
            }
        }
    }
}
