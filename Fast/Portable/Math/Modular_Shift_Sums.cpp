// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Modular_Shift_Sums.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_shift_sums.md
// 参考（合同剰余を周期的な差分として数える）: https://atcoder.jp/contests/abc483/tasks/abc483_f/editorial
//
// API一覧
// ModularShiftTerm{offset,modulus}: offsetはint64_t、modulusは正のuint32_t。
// modular_shift_sums(limit,terms): x=0..limitの各点でΣ(x+offset) mod modulusを求めたvector<uint64_t>。
// 各項の剰余は0..modulus-1、総和はuint64_tで表現できる範囲で厳密。溢れる場合は2^64で折り返す。
// 同じ項は重複の回数分数える。空のtermsなら全項0、法0はinvalid_argument。
// 入力n個・上限L=limitでO(n log(n+1)+(L+1)√(n+1))時間・O(n+L+1)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MODULAR_SHIFT_SUMS_DEFINED
#define SIRKIT_MODULAR_SHIFT_SUMS_DEFINED

struct ModularShiftTerm
{
    std::int64_t offset;
    std::uint32_t modulus;
};

namespace detail
{
struct ModularResidueDrop
{
    std::uint32_t position;
    std::size_t frequency;
};
} // namespace detail

[[nodiscard]] inline std::vector<std::uint64_t> modular_shift_sums(
    std::uint32_t limit, std::span<const ModularShiftTerm> terms)
{
    std::vector<std::uint64_t> packed;
    packed.reserve(terms.size());
    for (const auto &[offset, modulus] : terms)
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("modular_shift_sums: zero modulus");
        }
        const auto remainder = offset % static_cast<std::int64_t>(modulus);
        const auto residue = static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus : remainder);
        packed.push_back((std::uint64_t{modulus} << 32) | residue);
    }
    std::ranges::sort(packed);

    std::vector<std::uint64_t> differences(std::size_t{limit} + 1);
    std::vector<std::uint64_t> period;
    std::vector<detail::ModularResidueDrop> drops;
    std::uint64_t value = 0;
    std::uint64_t slope = 0;
    for (std::size_t i = 0; i < packed.size();)
    {
        const auto first = i;
        const auto modulus = static_cast<std::uint32_t>(packed[i] >> 32);
        drops.clear();
        do
        {
            const auto pair = packed[i];
            const auto residue = static_cast<std::uint32_t>(pair);
            auto next = i + 1;
            while (next < packed.size() && packed[next] == pair)
            {
                ++next;
            }
            const auto frequency = next - i;
            value += std::uint64_t{residue} * frequency;
            drops.push_back({modulus - residue, frequency});
            i = next;
        } while (i < packed.size() && packed[i] >> 32 == modulus);

        if (drops.size() == modulus &&
            std::ranges::all_of(drops, [&](const detail::ModularResidueDrop &drop)
                                {
                                    return drop.frequency == drops.front().frequency;
                                }))
        {
            continue; // 剰余ごとの出現個数が等しいと、xを変えても総和は変わらない。
        }
        slope += i - first;
        if (modulus <= limit && std::uint64_t{drops.size()} * (limit / modulus) >= limit / 3)
        {
            period.assign(modulus, 0);
            for (const auto &drop : drops)
            {
                period[drop.position % modulus] = std::uint64_t{modulus} * drop.frequency;
            }
            for (std::size_t start = 0; start < differences.size(); start += modulus)
            {
                const auto size = std::min(period.size(), differences.size() - start);
                auto destination = differences.begin() + static_cast<std::ptrdiff_t>(start);
                std::transform(period.begin(), period.begin() + static_cast<std::ptrdiff_t>(size),
                               destination, destination, std::plus<>{});
            }
        }
        else
        {
            for (const auto &drop : drops)
            {
                const auto amount = std::uint64_t{modulus} * drop.frequency;
                for (std::uint64_t point = drop.position; point <= limit; point += modulus)
                {
                    differences[static_cast<std::size_t>(point)] += amount;
                }
            }
        }
    }
    differences.front() = value;
    for (std::size_t x = 1; x < differences.size(); ++x)
    {
        value += slope - differences[x];
        differences[x] = value;
    }
    return differences;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
