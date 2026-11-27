// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Affine
{
    std::uint32_t a, b;
};

struct Compose
{
    constexpr Affine operator()(Affine left, Affine right) const noexcept
    {
        return {
            static_cast<std::uint32_t>(std::uint64_t{right.a} * left.a % kMod),
            static_cast<std::uint32_t>((std::uint64_t{right.a} * left.b + right.b) % kMod),
        };
    }
};
}

int main()
{
    int q = 0;
    verify_in >> q;
    DequeAggregation deque(Affine{1, 0}, Compose{});
    while (q-- > 0)
    {
        int type = 0;
        verify_in >> type;
        switch (type)
        {
        case 0:
        {
            Affine f{};
            verify_in >> f.a >> f.b;
            deque.push_front(f);
            break;
        }
        case 1:
        {
            Affine f{};
            verify_in >> f.a >> f.b;
            deque.push_back(f);
            break;
        }
        case 2:
            deque.pop_front();
            break;
        case 3:
            deque.pop_back();
            break;
        case 4:
        {
            std::uint32_t x = 0;
            verify_in >> x;
            const auto fold = deque.all_prod();
            verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
            break;
        }
        }
    }
}
