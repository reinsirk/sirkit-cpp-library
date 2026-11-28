// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Math/Modular_Shift_Minimum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_shift_minimum.md
// 参考（円環上の各区間を数える帰着）: https://atcoder.jp/contests/arc182/editorial/10477
//
// API一覧
// sum_minimum_modular_shifts(count,modulus,step,offsets,start=0):
// k=0..count-1に対し、(start+step*k+offset) mod modulusのoffsetに関する最小値を取り、全kで総和。
// 結果はuint64_tで厳密。countとmodulusはuint32_t。step/start/offsetは負のint64_tも可。
// modulus=0または空のoffsetsはinvalid_argument。有効な入力でcount=0なら0。
// offsetsがn個、剰余が異なる値がu個ならO(n log(n+1)+u log(modulus+1))時間・O(n)領域。
// count=1またはstepがmodulusの倍数ならO(n)時間。
//
// [同梱のFloor Sum]
// floor_sum(n,m,a,b): i=0..n-1 の floor((a*i+b)/m) の総和。a,bは負でもよい。
// 0<=n<2^32、1<=m<2^32。ACLと同じく結果は2^64を法とする符号付き64ビット値。
// min_of_mod_of_linear(n,m,a,b): i=0..n-1 の (a*i+b) mod m の最小値。1<=n<2^32。
// 両方ともO(log(m+1))時間・O(1)領域。検査有効時、n,mが範囲外ならstd::invalid_argument。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include <span>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_SUM_DEFINED
#define SIRKIT_FLOOR_SUM_DEFINED

inline std::int64_t floor_sum(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n < 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("floor_sum: n or m");
    }
#endif
    if (n == 0)
    {
        return 0;
    }
    const auto count = static_cast<std::uint64_t>(n);
    const auto triangular = count * (count - 1) / 2;
    std::uint64_t sum = 0;
    if (a < 0)
    {
        const auto remainder = a % m;
        const auto quotient = a / m;
        sum += triangular * static_cast<std::uint64_t>(quotient - (remainder != 0));
        a = remainder == 0 ? 0 : remainder + m;
    }
    if (b < 0)
    {
        const auto remainder = b % m;
        const auto quotient = b / m;
        sum += count * static_cast<std::uint64_t>(quotient - (remainder != 0));
        b = remainder == 0 ? 0 : remainder + m;
    }
    auto length = count;
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = static_cast<std::uint64_t>(a);
    auto offset = static_cast<std::uint64_t>(b);
    while (true)
    {
        if (slope >= modulus)
        {
            sum += (length * (length - 1) / 2) * (slope / modulus);
            slope %= modulus;
        }
        if (offset >= modulus)
        {
            sum += length * (offset / modulus);
            offset %= modulus;
        }
        const auto limit = slope * length + offset;
        if (limit < modulus)
        {
            break;
        }
        length = limit / modulus; // 格子点を横方向から数え直す。
        offset = limit % modulus;
        std::swap(modulus, slope);
    }
    return std::bit_cast<std::int64_t>(sum);
}

inline std::int64_t min_of_mod_of_linear(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n <= 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("min_of_mod_of_linear: n or m");
    }
#endif
    const auto normalize = [](std::int64_t x, std::int64_t modulus)
    {
        x %= modulus;
        return static_cast<std::uint64_t>(x < 0 ? x + modulus : x);
    };
    auto length = static_cast<std::uint64_t>(n);
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = normalize(a, m);
    auto offset = normalize(b, m);
    auto minimum = offset;
    while (minimum != 0 && slope != 0)
    {
        if (slope * 2 > modulus) // 逆から走査して次の法を半分以下にする。
        {
            offset = (slope * (length - 1) + offset) % modulus;
            slope = modulus - slope;
        }
        minimum = std::min(minimum, offset);
        const auto wraps = (slope * (length - 1) + offset) / modulus;
        if (wraps == 0)
        {
            break;
        }
        const auto reverse_step = (slope - modulus % slope) % slope; // 次の法は slope。
        length = wraps;
        offset = (offset % slope + reverse_step) % slope; // y回目の折り返しは (offset - modulus*y) mod slope。
        modulus = slope;
        slope = reverse_step;
        minimum = std::min(minimum, offset);
    }
    return static_cast<std::int64_t>(minimum);
}

#endif

#ifndef SIRKIT_MODULAR_SHIFT_MINIMUM_DEFINED
#define SIRKIT_MODULAR_SHIFT_MINIMUM_DEFINED

namespace detail
{
inline std::uint32_t ShiftMinimumResidue(std::int64_t value, std::uint32_t modulus)
{
    const auto remainder = value % static_cast<std::int64_t>(modulus);
    return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus : remainder);
}
} // namespace detail

[[nodiscard]] inline std::uint64_t sum_minimum_modular_shifts(
    std::uint32_t count, std::uint32_t modulus, std::int64_t step,
    std::span<const std::int64_t> offsets, std::int64_t start = 0)
{
    if (modulus == 0 || offsets.empty())
    {
        throw std::invalid_argument("sum_minimum_modular_shifts: zero modulus or no offsets");
    }
    if (count == 0)
    {
        return 0;
    }
    const auto phase = detail::ShiftMinimumResidue(start, modulus);
    std::vector<std::uint32_t> residues;
    residues.reserve(offsets.size());
    for (const auto offset : offsets)
    {
        const auto base = detail::ShiftMinimumResidue(offset, modulus);
        residues.push_back(static_cast<std::uint32_t>((std::uint64_t{base} + phase) % modulus));
    }
    const auto slope = detail::ShiftMinimumResidue(step, modulus);
    const std::uint64_t length = count;
    if (slope == 0 || count == 1)
    {
        return length * *std::ranges::min_element(residues);
    }
    std::ranges::sort(residues);
    const auto duplicates = std::ranges::unique(residues);
    residues.erase(duplicates.begin(), duplicates.end());

    // 一周の各区切りで、その次の剰余までの距離だけ最小値が減る。
    // 中間結果は2^64で折り返すが、最終結果はcount*(modulus-1)以下。
    auto total = (length * (length - 1) / 2) * slope + length * residues.front();
    for (std::size_t i = 0; i < residues.size(); ++i)
    {
        const auto prior = residues[i];
        const auto gap = i + 1 == residues.size() ? modulus - prior + residues.front() : residues[i + 1] - prior;
        const auto drops = static_cast<std::uint64_t>(floor_sum(count, modulus, slope, prior));
        total -= std::uint64_t{gap} * drops;
    }
    return total;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
