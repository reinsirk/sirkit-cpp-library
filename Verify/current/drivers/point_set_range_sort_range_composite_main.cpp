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
    using Sortable = RangeSortAggregation<Affine, Compose, Identity>;
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Sortable::Item> items(n);
    for (auto &item : items)
    {
        verify_in >> item.key >> item.value.a >> item.value.b;
    }
    Sortable data(items);
    while (q-- > 0)
    {
        int type = 0;
        verify_in >> type;
        if (type == 0)
        {
            int index = 0;
            std::int32_t key = 0;
            Affine value{};
            verify_in >> index >> key >> value.a >> value.b;
            data.set(index, key, value);
        }
        else
        {
            int left = 0, right = 0;
            verify_in >> left >> right;
            if (type == 1)
            {
                std::uint32_t x = 0;
                verify_in >> x;
                const auto fold = data.prod(left, right);
                verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
            }
            else
            {
                data.sort(left, right, type == 3);
            }
        }
    }
}
