// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PFAFFIAN_DEFINED
#include "../../../Math/Pfaffian.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Matrix = std::vector<std::vector<std::uint32_t>>;
constexpr std::uint32_t kMod = 998244353;

std::uint32_t matching_sum(const Matrix &matrix, const std::vector<std::size_t> &remaining, std::uint32_t mod)
{
    if (remaining.empty())
    {
        return 1;
    }
    std::uint64_t result = 0;
    for (std::size_t other = 1; other < remaining.size(); ++other)
    {
        std::vector<std::size_t> rest(remaining.begin() + 1, remaining.end());
        rest.erase(rest.begin() + other - 1);
        const auto tail = matching_sum(matrix, rest, mod);
        const auto term = (std::uint64_t{matrix[remaining.front()][remaining[other]]} * tail) % mod;
        result = (result + (other % 2 == 1 ? term : mod - term)) % mod;
    }
    return static_cast<std::uint32_t>(result);
}

std::uint32_t determinant(Matrix matrix, std::uint32_t mod)
{
    const auto n = matrix.size();
    std::uint64_t result = 1;
    for (std::size_t i = 0; i < n; ++i)
    {
        auto pivot = i;
        while (pivot < n && matrix[pivot][i] == 0)
        {
            ++pivot;
        }
        if (pivot == n)
        {
            return 0;
        }
        if (pivot != i)
        {
            std::swap(matrix[i], matrix[pivot]);
            result = mod - result;
        }
        const std::uint64_t diagonal = matrix[i][i];
        result = result * diagonal % mod;
        for (std::size_t below = i + 1; below < n; ++below)
        {
            const std::uint64_t factor = matrix[below][i];
            for (std::size_t j = i + 1; j < n; ++j)
            {
                const auto old = diagonal * matrix[below][j] % mod;
                const auto subtract = factor * matrix[i][j] % mod;
                matrix[below][j] = static_cast<std::uint32_t>((old + mod - subtract) % mod);
            }
        }
        // 下の各行をピボット倍した。行列式に入る余分な因子を打ち消す。
        std::uint64_t inverse = 1, base = diagonal;
        for (std::uint32_t power = mod - 2; power != 0; power >>= 1)
        {
            if (power % 2 == 1)
            {
                inverse = inverse * base % mod;
            }
            base = base * base % mod;
        }
        for (std::size_t below = i + 1; below < n; ++below)
        {
            result = result * inverse % mod;
        }
    }
    return static_cast<std::uint32_t>(result);
}

template <std::uint32_t Mod>
void compare(std::mt19937 &random)
{
    for (int test = 0; test < 110; ++test)
    {
        const std::size_t n = random() % 11;
        Matrix matrix(n, std::vector<std::uint32_t>(n));
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = i + 1; j < n; ++j)
            {
                const auto entry = static_cast<std::uint32_t>((test % 2 == 0 || random() % 3 == 0) ? random() % Mod : 0);
                matrix[i][j] = entry;
                matrix[j][i] = (Mod - entry) % Mod;
            }
        }
        const auto pf = sirkit::pfaffian_mod<Mod>(matrix);
        if (n % 2 == 0)
        {
            std::vector<std::size_t> remaining(n);
            std::iota(remaining.begin(), remaining.end(), 0uz);
            assert(pf == matching_sum(matrix, remaining, Mod));
        }
        else
        {
            assert(pf == 0);
        }
        assert(std::uint64_t{pf} * pf % Mod == determinant(matrix, Mod));
    }
}
} // namespace

int main()
{
    assert(sirkit::pfaffian_mod({}) == 1);
    assert(sirkit::pfaffian_mod({{0}}) == 0);
    assert(sirkit::pfaffian_mod({{0, 0}, {0, 0}}) == 0);
    assert(sirkit::pfaffian_mod({{0, 0, 2, 0}, {0, 0, 0, 3}, {kMod - 2, 0, 0, 0}, {0, kMod - 3, 0, 0}}) == kMod - 6);
    std::mt19937 random(521);
    compare<2>(random);
    compare<3>(random);
    compare<kMod>(random);
    compare<4294967291u>(random);

    constexpr std::size_t kMaxN = 600;
    Matrix max_matrix(kMaxN, std::vector<std::uint32_t>(kMaxN));
    for (std::size_t i = 0; i < kMaxN; ++i)
    {
        for (std::size_t j = i + 1; j < kMaxN; ++j)
        {
            max_matrix[i][j] = 1;
            max_matrix[j][i] = kMod - 1;
        }
    }
    // 完全マッチングの符号付き展開で、上三角がすべて1の交代行列は任意の偶数次元でPf=1。
    assert(sirkit::pfaffian_mod(std::move(max_matrix)) == 1);

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &invalid : {Matrix{{0, 1}}, Matrix{{1}}, Matrix{{0, kMod}, {0, 0}}, Matrix{{0, 1}, {1, 0}}})
    {
        bool failed = false;
        try
        {
            (void)sirkit::pfaffian_mod(invalid);
        }
        catch (const std::invalid_argument &)
        {
            failed = true;
        }
        assert(failed);
    }
#endif
}
