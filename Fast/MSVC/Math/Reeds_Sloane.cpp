// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Reeds_Sloane.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/reeds_sloane.md
// 参考（素数冪ごとの最短漸化式）: https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_reconstruction_mod
//
// API一覧
// reeds_sloane(s,modulus): s[i]=sum(c[j]*s[i-j-1]) (i>=c.size())を満たす最短の係数列c。
// 入力はuint32_t列。法は1..UINT32_MAX、係数は0..modulus-1。入力は正規化不要。
// 空列と法の下ですべて0の列は空の係数列。法0はinvalid_argument。
// N=s.size()、E=素因数の指数の総和としてO(sqrt(modulus)+NE(N+log(modulus+1)))時間、O(N+NE)領域。
// 有限の入力列に適合することを保証する。未知の後続項が従うことは入力だけからは分からない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_REEDS_SLOANE_DEFINED
#define SIRKIT_REEDS_SLOANE_DEFINED

namespace detail
{
struct ReedsPrimePower
{
    std::uint32_t prime;
    std::vector<std::uint32_t> powers;
};

inline std::vector<ReedsPrimePower> ReedsFactor(std::uint32_t remaining)
{
    std::vector<ReedsPrimePower> result;
    for (std::uint32_t prime = 2; prime <= remaining / prime; prime += (prime == 2 ? 1U : 2U))
    {
        if (remaining % prime != 0)
        {
            continue;
        }
        ReedsPrimePower factor{prime, {1}};
        do
        {
            remaining /= prime;
            factor.powers.push_back(factor.powers.back() * prime);
        } while (remaining % prime == 0);
        result.push_back(std::move(factor));
    }
    if (remaining > 1)
    {
        result.push_back({remaining, {1, remaining}});
    }
    return result;
}

inline std::uint32_t ReedsModularPower(std::uint32_t base, std::uint32_t exponent, std::uint32_t modulus)
{
    std::uint32_t value = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            value = static_cast<std::uint32_t>(std::uint64_t{value} * base % modulus);
        }
        base = static_cast<std::uint32_t>(std::uint64_t{base} * base % modulus);
    }
    return value;
}

inline std::uint32_t ReedsSubtract(std::uint32_t a, std::uint32_t b, std::uint32_t modulus)
{
    return a >= b ? a - b : modulus - (b - a);
}

struct ReedsPolynomial
{
    std::vector<std::uint32_t> coefficients;
    std::size_t order{};
};

struct ReedsHistory
{
    ReedsPolynomial polynomial;
    std::size_t prefix{};
    std::uint32_t inverse_unit{};
};

struct ReedsDiscrepancy
{
    std::uint32_t unit;
    std::size_t valuation;
};

inline ReedsDiscrepancy ReedsEvaluate(std::span<const std::uint32_t> sequence, std::span<const std::uint32_t> polynomial,
                                      std::size_t position, const ReedsPrimePower &factor)
{
    const auto modulus = factor.powers.back();
    std::uint64_t discrepancy = 0;
    for (std::size_t i = 0; i < polynomial.size(); ++i)
    {
        discrepancy = (discrepancy + std::uint64_t{polynomial[i]} * sequence[position - i]) % modulus;
    }
    const auto exponent = factor.powers.size() - 1;
    if (discrepancy == 0)
    {
        return {0, exponent};
    }
    std::size_t valuation = 0;
    while (discrepancy % factor.prime == 0)
    {
        discrepancy /= factor.prime;
        ++valuation;
    }
    return {static_cast<std::uint32_t>(discrepancy), valuation};
}

inline std::vector<std::uint32_t> ReedsPrimePowerRecurrence(std::span<const std::uint32_t> sequence,
                                                            const ReedsPrimePower &factor)
{
    const auto modulus = factor.powers.back();
    const auto totient = modulus - modulus / factor.prime;
    const auto exponent = factor.powers.size() - 1;
    std::vector<std::uint32_t> reduced(sequence.size());
    for (std::size_t i = 0; i < reduced.size(); ++i)
    {
        reduced[i] = sequence[i] % modulus;
    }
    std::vector<ReedsPolynomial> current(exponent), updated(exponent);
    std::vector<ReedsHistory> history(exponent);
    std::vector<ReedsDiscrepancy> discrepancies(exponent);
    for (std::size_t j = 0; j < exponent; ++j)
    {
        current[j].coefficients = {factor.powers[j]};
    }
    for (std::size_t pos = 0; pos < reduced.size(); ++pos)
    {
        for (std::size_t j = 0; j < exponent; ++j)
        {
            discrepancies[j] = ReedsEvaluate(reduced, current[j].coefficients, pos, factor);
        }
        updated = current;
        for (std::size_t j = 0; j < exponent; ++j)
        {
            const auto [unit, valuation] = discrepancies[j];
            if (unit == 0)
            {
                continue;
            }

            // 付値が同じ保存済みの差分で、現在位置の差分だけを打ち消す。
            const auto &previous = history[exponent - 1 - valuation];
            auto &next = updated[j];
            if (previous.prefix == 0)
            {
                next.order = pos + 1;
                continue;
            }
            const auto shift = pos + 1 - previous.prefix;
            const auto correction = std::uint64_t{unit} * previous.inverse_unit % modulus;
            const auto &polynomial = previous.polynomial;
            next.order = std::max(next.order, polynomial.order + shift);
            auto &coefficients = next.coefficients;
            coefficients.resize(std::max(coefficients.size(), polynomial.coefficients.size() + shift));
            for (std::size_t i = 0; i < polynomial.coefficients.size(); ++i)
            {
                auto &value = coefficients[i + shift];
                const auto replacement = static_cast<std::uint32_t>(correction * polynomial.coefficients[i] % modulus);
                value = ReedsSubtract(value, replacement, modulus);
            }
        }
        for (std::size_t j = 0; j < exponent; ++j)
        {
            if (updated[j].order > current[j].order)
            {
                const auto k = exponent - 1 - discrepancies[j].valuation;
                const auto inverse = ReedsModularPower(discrepancies[k].unit, totient - 1, modulus);
                history[j] = {current[k], pos + 1, inverse};
            }
        }
        current.swap(updated);
    }

    auto polynomial = std::move(current.front());
    polynomial.coefficients.resize(polynomial.order + 1);
    std::vector<std::uint32_t> recurrence(polynomial.order);
    for (std::size_t i = 0; i < recurrence.size(); ++i)
    {
        recurrence[i] = ReedsSubtract(0, polynomial.coefficients[i + 1], modulus);
    }
    return recurrence;
}
} // namespace detail

[[nodiscard]] inline std::vector<std::uint32_t> reeds_sloane(std::span<const std::uint32_t> sequence, std::uint32_t modulus)
{
    if (modulus == 0)
    {
        throw std::invalid_argument("reeds_sloane: zero modulus");
    }
    if (modulus == 1 || sequence.empty())
    {
        return {};
    }

    std::vector<std::uint32_t> result;
    std::uint32_t combined_modulus = 1;
    for (const auto &factor : detail::ReedsFactor(modulus))
    {
        const auto local_modulus = factor.powers.back();
        const auto local = detail::ReedsPrimePowerRecurrence(sequence, factor);
        const auto totient = local_modulus - local_modulus / factor.prime;
        const auto inverse = detail::ReedsModularPower(combined_modulus % local_modulus, totient - 1, local_modulus);
        result.resize(std::max(result.size(), local.size()));
        for (std::size_t i = 0; i < result.size(); ++i)
        {
            const auto residue = i < local.size() ? local[i] : 0;
            const auto difference = detail::ReedsSubtract(residue, result[i] % local_modulus, local_modulus);
            const auto multiplier = std::uint64_t{inverse} * difference % local_modulus;
            result[i] = static_cast<std::uint32_t>(result[i] + combined_modulus * multiplier);
        }
        combined_modulus *= local_modulus;
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
