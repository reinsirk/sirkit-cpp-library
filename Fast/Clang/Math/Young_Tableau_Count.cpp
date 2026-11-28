// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Young_Tableau_Count.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/young_tableau_count.md
// 参考（フック長の公式）: https://info.atcoder.jp/entry/algorithm_lectures/hook_length_formula
//
// API一覧
// standard_young_tableau_count<Mod=998244353>(shape): 各行・列が増加する標準ヤング盤の個数を法Modで返す。
// domino_tableau_count<Mod=998244353>(shape): 図形を保って右端の横・縦ドミノを順に除く操作列の個数を法Modで返す。
// shapeは行長の広義単調減少列。末尾0を許す。空図形は1通り。Modは正のuint32_tで合成数も可。
// 行数H、幅W、面積N、Modの異なる素因数数kに対しO(H+N(k+1)+(k+1)log(Mod+1))時間、O(H+W+k+1)領域。
// 面積がuint32_tを超えればlength_error、行長の順序が不正ならinvalid_argument。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_YOUNG_TABLEAU_COUNT_DEFINED
#define SIRKIT_YOUNG_TABLEAU_COUNT_DEFINED

namespace detail
{
inline std::uint32_t YoungTableauCells(std::span<const std::uint32_t> shape)
{
    auto previous = std::numeric_limits<std::uint32_t>::max();
    std::uint64_t cells = 0;
    for (const auto length : shape)
    {
        if (length > previous)
        {
            throw std::invalid_argument("Young tableau: nonincreasing row lengths required");
        }
        cells += length;
        if (cells > std::numeric_limits<std::uint32_t>::max())
        {
            throw std::length_error("Young tableau: too many cells");
        }
        previous = length;
    }
    return static_cast<std::uint32_t>(cells);
}

template <std::uint32_t Mod>
struct YoungTableauFactors
{
    static_assert(Mod > 0, "Young tableau: Mod must be positive");
    std::array<std::uint32_t, std::numeric_limits<std::uint32_t>::digits> primes{};
    std::size_t size{};
    std::uint32_t totient = Mod;

    consteval YoungTableauFactors()
    {
        auto rest = Mod;
        for (std::uint32_t divisor = 2; divisor <= rest / divisor; ++divisor)
        {
            if (rest % divisor == 0)
            {
                primes[size++] = divisor;
                totient -= totient / divisor;
                do
                {
                    rest /= divisor;
                } while (rest % divisor == 0);
            }
        }
        if (rest > 1)
        {
            primes[size++] = rest;
            totient -= totient / rest;
        }
    }
};

template <std::uint32_t Mod>
std::uint32_t YoungTableauPower(std::uint32_t value, std::uint64_t exponent)
{
    std::uint32_t result = 1 % Mod;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = static_cast<std::uint32_t>(std::uint64_t{result} * value % Mod);
        }
        value = static_cast<std::uint32_t>(std::uint64_t{value} * value % Mod);
    }
    return result;
}

template <class Consume>
void ForEachYoungTableauHook(std::span<const std::uint32_t> shape, Consume consume)
{
    if (shape.empty())
    {
        return;
    }
    std::vector<std::uint32_t> below(shape.front());
    for (auto row = shape.size(); row != 0; --row)
    {
        const auto length = shape[row - 1];
        for (std::uint32_t column = 0; column < length; ++column)
        {
            consume(length - column + below[column]);
            ++below[column];
        }
    }
}

template <std::uint32_t Mod>
std::uint32_t YoungTableauHookRatio(std::uint32_t cells, std::span<const std::uint32_t> first,
                                    std::span<const std::uint32_t> second)
{
    static_assert(Mod > 0, "Young tableau: Mod must be positive");
    if constexpr (Mod == 1)
    {
        return 0;
    }
    else
    {
        constexpr YoungTableauFactors<Mod> factors;
        std::size_t active = 0;
        while (active < factors.size && factors.primes[active] <= cells)
        {
            ++active;
        }
        const auto primes = std::span(factors.primes).first(active);
        std::array<std::int64_t, std::numeric_limits<std::uint32_t>::digits> powers{};
        const auto strip = [&](std::uint32_t value, int direction)
        {
            for (std::size_t i = 0; i < primes.size(); ++i)
            {
                const auto prime = primes[i];
                while (value % prime == 0)
                {
                    value /= prime;
                    powers[i] += direction;
                }
            }
            return value;
        };
        std::uint64_t numerator = 1, denominator = 1;
        for (std::uint64_t i = 2; i <= cells; ++i)
        {
            numerator = numerator * strip(static_cast<std::uint32_t>(i), 1) % Mod;
        }
        const auto divide_hook = [&](std::uint32_t hook)
        {
            denominator = denominator * strip(hook, -1) % Mod;
        };
        ForEachYoungTableauHook(first, divide_hook);
        ForEachYoungTableauHook(second, divide_hook);
        // 法と共通の素因数を除いた分母だけを逆元にし、差し引いた素因数を掛け直す。
        auto result = numerator * YoungTableauPower<Mod>(static_cast<std::uint32_t>(denominator), factors.totient - 1) % Mod;
        for (std::size_t i = 0; i < active; ++i)
        {
            result = result * YoungTableauPower<Mod>(primes[i], static_cast<std::uint64_t>(powers[i])) % Mod;
        }
        return static_cast<std::uint32_t>(result);
    }
}
} // namespace detail

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::uint32_t standard_young_tableau_count(std::span<const std::uint32_t> shape)
{
    const auto cells = detail::YoungTableauCells(shape);
    return detail::YoungTableauHookRatio<Mod>(cells, shape, {});
}

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::uint32_t domino_tableau_count(std::span<const std::uint32_t> shape)
{
    const auto cells = detail::YoungTableauCells(shape);
    if ((cells & 1u) != 0)
    {
        return 0;
    }
    while (!shape.empty() && shape.back() == 0)
    {
        shape = shape.first(shape.size() - 1);
    }
    // ベータ数を偶奇に分けると、ドミノの除去は各列の1マス除去になる。
    std::array<std::vector<std::uint32_t>, 2> quotient;
    for (std::size_t i = 0; i < shape.size(); ++i)
    {
        const auto beta = std::uint64_t{shape[i]} + (shape.size() - 1 - i);
        quotient[beta & 1u].push_back(static_cast<std::uint32_t>(beta / 2));
    }
    if (quotient[0].size() != (shape.size() + 1) / 2)
    {
        return 0;
    }
    for (auto &part : quotient)
    {
        for (std::size_t i = 0; i < part.size(); ++i)
        {
            part[i] -= static_cast<std::uint32_t>(part.size() - 1 - i);
        }
    }
    return detail::YoungTableauHookRatio<Mod>(cells / 2, quotient[0], quotient[1]);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
