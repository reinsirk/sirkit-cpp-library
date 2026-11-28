// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Matrix_Pencil_Determinant.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Matrix = std::vector<std::vector<std::int64_t>>;
using Polynomial = std::vector<std::uint32_t>;

template <std::uint32_t Mod>
std::uint64_t Normalize(std::int64_t value)
{
    const auto remainder = value % static_cast<std::int64_t>(Mod);
    return static_cast<std::uint64_t>((remainder + Mod) % Mod);
}

template <std::uint32_t Mod>
Polynomial MultiplyLinear(const Polynomial &poly, std::int64_t a, std::int64_t b)
{
    const auto a0 = Normalize<Mod>(a);
    const auto a1 = Normalize<Mod>(b);
    Polynomial product(poly.size() + 1);
    for (std::size_t i = 0; i < poly.size(); ++i)
    {
        product[i] = static_cast<std::uint32_t>((product[i] + a0 * poly[i] % Mod) % Mod);
        product[i + 1] = static_cast<std::uint32_t>(a1 * poly[i] % Mod);
    }
    return product;
}

template <std::uint32_t Mod>
Polynomial Enumerate(const Matrix &a, const Matrix &b)
{
    const auto n = a.size();
    Polynomial result(n + 1);
    std::vector<std::size_t> order(n);
    std::iota(order.begin(), order.end(), 0);
    do
    {
        bool odd = false;
        Polynomial product{1};
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < i; ++j)
            {
                odd ^= order[j] > order[i];
            }
            product = MultiplyLinear<Mod>(product, a[i][order[i]], b[i][order[i]]);
        }
        for (std::size_t i = 0; i <= n; ++i)
        {
            const auto accumulated = std::uint64_t{result[i]} + (odd ? Mod - product[i] : product[i]);
            result[i] = static_cast<std::uint32_t>(accumulated % Mod);
        }
    } while (std::next_permutation(order.begin(), order.end()));
    return result;
}

template <std::uint32_t Mod>
void Verify(const Matrix &a, const Matrix &b)
{
    assert(sirkit::matrix_pencil_determinant<Mod>(a, b) == Enumerate<Mod>(a, b));
}
} // namespace

int main()
{
    constexpr std::uint32_t kMod = 998244353;
    constexpr std::uint32_t kLargePrime = 4'294'967'291;
    using sirkit::matrix_pencil_determinant;
    assert(matrix_pencil_determinant({}, {}) == Polynomial({1}));
    Verify<kMod>({{1, 2}, {3, 4}}, {{1, 0}, {0, 0}});
    Verify<kMod>({{1, 0}, {0, 0}}, {{0, 1}, {0, 0}});
    Verify<kMod>({{0, 0, 1}, {1, 0, 0}, {0, 1, 0}}, {{1, 0, 0}, {0, 1, 0}, {0, 0, 0}});
    Verify<kMod>({{0, 0}, {0, 0}}, {{0, 0}, {0, 0}});
    Verify<kMod>({{0, 0}, {0, 0}}, {{0, 1}, {1, 0}});
    Verify<kMod>({{0, 0}, {0, 0}}, {{0, 0}, {0, 1}});
    Verify<2>({{1, 2}, {-1, 0}}, {{0, 7}, {-3, 2}});
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    Verify<kLargePrime>({{kHigh, kLow}, {kLow, kHigh}}, {{kLow, kHigh}, {kHigh, kHigh}});

    std::mt19937 random(989769);
    for (int trial = 0; trial < 450; ++trial)
    {
        const auto n = static_cast<std::size_t>(random() % 6 + 1);
        Matrix a(n, std::vector<std::int64_t>(n)), b(a);
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < n; ++j)
            {
                a[i][j] = static_cast<std::int64_t>(random() % 21) - 10;
                if (random() % 3 != 0)
                {
                    b[i][j] = static_cast<std::int64_t>(random() % 13) - 6;
                }
            }
        }
        if (trial % 4 == 0)
        {
            b.back() = b.front();
        }
        if (trial % 7 == 0)
        {
            a.back() = a.front();
        }
        Verify<2>(a, b);
        Verify<7>(a, b);
        Verify<kMod>(a, b);
        Verify<kLargePrime>(a, b);
    }

    constexpr std::size_t kSide = 128;
    Matrix a(kSide, std::vector<std::int64_t>(kSide)), b(a);
    Polynomial expected{1};
    for (std::size_t i = 0; i < kSide; ++i)
    {
        a[i][i] = static_cast<std::int64_t>(random() % 101) - 50;
        b[i][i] = static_cast<std::int64_t>(random() % 9) - 4;
        expected = MultiplyLinear<kMod>(expected, a[i][i], b[i][i]);
    }
    assert(matrix_pencil_determinant(a, b) == expected);

    bool rejected = false;
    try
    {
        static_cast<void>(matrix_pencil_determinant(Matrix{{1}}, {}));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
    rejected = false;
    try
    {
        static_cast<void>(matrix_pencil_determinant(Matrix{{1}}, Matrix{{1, 2}}));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
}
