// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Prefix_Dominant_Compositions.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prefix_dominant_compositions.md
// 参考（巡回シフトによる数え上げ）: https://info.atcoder.jp/entry/algorithm_lectures/cycle_lemma
//
// API一覧
// prefix_dominant_composition_counts<Mod=998244353>(max_length,p,q):
// 長さn=0..max_lengthでΣA=p、ΣB=qかつ全ての接頭辞でqΣA>=pΣBとなる非負整数列の組数を法Modで返す。
// p,qは正の互いに素なuint64_t。Modは素数、max_length<Mod。長さ0では0通り。
// O(max_length+1)時間・領域。p,qが不正ならinvalid_argument、長さの上限超過はlength_error。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_DOMINANT_COMPOSITIONS_DEFINED
#define SIRKIT_PREFIX_DOMINANT_COMPOSITIONS_DEFINED

namespace detail
{
consteval bool PrefixDominancePrime(std::uint32_t modulus)
{
    if (modulus < 2)
    {
        return false;
    }
    for (std::uint32_t factor = 2; factor <= modulus / factor; ++factor)
    {
        if (modulus % factor == 0)
        {
            return false;
        }
    }
    return true;
}
} // namespace detail

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::vector<std::uint32_t> prefix_dominant_composition_counts(
    std::size_t max_length, std::uint64_t p, std::uint64_t q)
{
    static_assert(detail::PrefixDominancePrime(Mod), "prefix_dominant_composition_counts: Mod must be prime");
    if (p == 0 || q == 0 || std::gcd(p, q) != 1)
    {
        throw std::invalid_argument("prefix_dominant_composition_counts: positive coprime totals required");
    }
    if (max_length >= Mod)
    {
        throw std::length_error("prefix_dominant_composition_counts: length must be below Mod");
    }

    std::vector<std::uint32_t> count(max_length + 1);
    if (max_length == 0)
    {
        return count;
    }
    count[1] = 1;
    for (std::size_t i = 2; i <= max_length; ++i)
    {
        count[i] = Mod - static_cast<std::uint32_t>(std::uint64_t{Mod} / i * count[Mod % i] % Mod);
    }

    // 長さnの答えの増分は C(p+n-1,n-1)*C(q+n-1,n-1)/n。
    // (0,0)の列を除いた巡回シフトの代表を数え、零列を戻す二項変換で得る。
    const auto reduced_p = p % Mod;
    const auto reduced_q = q % Mod;
    std::uint64_t compositions = 1, prefix = 0;
    for (std::size_t length = 1; length <= max_length; ++length)
    {
        const auto inverse = count[length];
        prefix = (prefix + compositions * inverse % Mod) % Mod;
        count[length] = static_cast<std::uint32_t>(prefix);
        compositions = compositions * ((reduced_p + length) % Mod) % Mod * inverse % Mod;
        compositions = compositions * ((reduced_q + length) % Mod) % Mod * inverse % Mod;
    }
    return count;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
