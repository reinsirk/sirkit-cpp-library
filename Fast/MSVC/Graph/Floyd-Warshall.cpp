// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Floyd-Warshall.cpp
// 参考: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/floyd-warshall
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/floyd_warshall.md
//
// API一覧
// kInf=std::int64_t{1}<<60。
// [Floyd-Warshall] V=頂点数、L=返す経路の頂点数。
// FloydWarshall(distances, inf=2^60): 正方行列を全点対最短距離で上書きし、グラフ内に負閉路があればtrue。O(V^3)時間。追加領域は下記。
// FloydWarshall(distances, next, inf=2^60): 次頂点表nextも作る。O(V^3)時間・O(V^2)追加領域。
// FloydWarshallPath(next, from, to): 次頂点表から頂点列をO(L+1)時間・領域で復元。未到達・負閉路の影響下なら空。
// 入力はinf>0が未接続、有限重みは(-inf, inf)。対角はmin(入力, 0)とする。
// 出力は未到達inf、負閉路を経由できる組だけ-inf。有限距離が(-inf, inf)外ならoverflow_error。
// 算術範囲の算出を兼ねて初期行列を走査する。SKIP_RUNTIME_CHECKSで形状・入力値・頂点の検査を除く。
// 通常版の追加領域はO(V)。大きな重みで広幅演算が必要な場合はO(V^2)。例外時の行列の内容は未規定。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include <type_traits>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INF_DEFINED
#define SIRKIT_INF_DEFINED
inline constexpr std::int64_t kInf = std::int64_t{1} << 60;
#endif

#ifndef SIRKIT_FLOYD_WARSHALL_DEFINED
#define SIRKIT_FLOYD_WARSHALL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {

// boolは既存の実行経路、専用の列挙型は有限範囲のSIMD最適化を選ぶ。
enum class FloydWarshallSimdMode { Finite = 1 };

// 必要なのは最大約92ビット。符号付き上位語と符号なし下位語で標準C++でも扱う。
struct FloydWarshallWide
{
    std::int64_t high = 0;
    std::uint64_t low = 0;
    constexpr FloydWarshallWide() = default;
    constexpr FloydWarshallWide(std::int64_t value)
        : high(value < 0 ? -1 : 0), low(static_cast<std::uint64_t>(value)) {}
    constexpr FloydWarshallWide(std::int64_t upper, std::uint64_t lower)
        : high(upper), low(lower) {}
    friend constexpr FloydWarshallWide operator+(FloydWarshallWide a,
                                                        FloydWarshallWide b)
    {
        const auto lower = a.low + b.low;
        return {a.high + b.high + (lower < a.low), lower};
    }
    friend constexpr FloydWarshallWide operator-(FloydWarshallWide value)
    {
        const auto lower = std::uint64_t{0} - value.low;
        return {~value.high + (lower == 0), lower};
    }
    friend constexpr bool operator<(FloydWarshallWide a, FloydWarshallWide b)
    {
        return a.high < b.high || (a.high == b.high && a.low < b.low);
    }
    friend constexpr bool operator==(FloydWarshallWide, FloydWarshallWide) = default;
    explicit constexpr operator std::int64_t() const { return std::bit_cast<std::int64_t>(low); }
};

// 広幅実装でも下限の絶対値は単純路の最大絶対値より大きい。
inline FloydWarshallWide FloydWarshallLimit(std::size_t n)
{
    return {static_cast<std::int64_t>(n >> 4), static_cast<std::uint64_t>(n) << 60};
}

// n<=INT_MAX、inf<=INT64_MAXなので積は94bit以内。標準経路も積を分割する。
inline FloydWarshallWide FloydWarshallCustomLimit(std::size_t n, std::int64_t inf)
{
    const auto value = static_cast<std::uint64_t>(inf);
    const auto first = static_cast<std::uint64_t>(n) * (value & 0xffffffffu);
    const auto second = static_cast<std::uint64_t>(n) * (value >> 32);
    const auto low = first + (second << 32);
    return {static_cast<std::int64_t>((second >> 32) + (low < first)), low};
}

template <bool RestorePath, bool HasNegative, bool FiniteRange = false, bool Dense = false, class Distance, auto UseSimd = false, bool CustomInf = false>
inline bool FloydWarshallRun(std::vector<std::vector<Distance>> &distance,
                                    Distance unreachable, Distance lower,
                                    std::vector<std::vector<int>> *next, std::int64_t requested_inf = kInf)
{
    const std::int64_t inf = CustomInf ? requested_inf : kInf;
    const std::size_t n = distance.size();
    {
        for (std::size_t via = 0; via < n; ++via)
        {
            const auto *through = distance[via].data();
            for (std::size_t from = 0; from < n; ++from)
            {
                auto *row = distance[from].data();
                const Distance prefix = row[via];
                if (prefix == unreachable)
                    continue;
                for (std::size_t to = 0; to < n; ++to)
                {
                    if constexpr (!Dense) if (through[to] == unreachable)
                        continue;
                    Distance candidate = prefix + through[to];
                    if constexpr (HasNegative)
                        candidate = std::max(candidate, lower);
                    if constexpr (RestorePath)
                    {
                        if (candidate < row[to])
                        {
                            row[to] = candidate;
                            (*next)[from][to] = (*next)[from][via];
                        }
                    }
                    else
                        row[to] = std::min(row[to], candidate);
                }
            }
        }
    }
    if constexpr (FiniteRange) return false;
    std::vector<std::size_t> negative;
    for (std::size_t vertex = 0; vertex < n; ++vertex)
        if (distance[vertex][vertex] < Distance{0})
            negative.push_back(vertex);
    const Distance affected = lower + Distance{-1};
    for (const std::size_t via : negative)
    {
        // 同じ強連結成分の負閉路は一度の伝播で足りる。
        if (distance[via][via] == affected)
            continue;
        for (std::size_t from = 0; from < n; ++from)
        {
            if (distance[from][via] == unreachable)
                continue;
            for (std::size_t to = 0; to < n; ++to)
                if (!(distance[via][to] == unreachable))
                {
                    distance[from][to] = affected;
                    if constexpr (RestorePath)
                        (*next)[from][to] = -1;
                }
        }
    }
    for (auto &row : distance)
        for (auto &value : row)
        {
            if (value == unreachable)
                value = Distance{inf};
            else if (value == affected)
                value = Distance{-inf};
            else if (!(Distance{-inf} < value) || !(value < Distance{inf}))
                throw std::overflow_error("FloydWarshall: finite distance collides with INF/-INF");
        }
    return !negative.empty();
}

inline void CheckFloydVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("FloydWarshall: too many vertices");
}

template <bool RestorePath, auto UseSimd = false, bool CustomInf = false>
inline bool FloydWarshallImpl(std::vector<std::vector<std::int64_t>> &distance,
                                     std::vector<std::vector<int>> *next, std::int64_t requested_inf = kInf)
{
    const std::int64_t inf = CustomInf ? requested_inf : kInf;
    const std::size_t n = distance.size();
    CheckFloydVertexCount(n);
    if constexpr (RestorePath)
        next->assign(n, std::vector<int>(n, -1));
    if (n == 0)
        return false;
    if (n == 1)
    {
        const bool negative = distance[0][0] < 0;
        distance[0][0] = negative ? -inf : 0;
        if constexpr (RestorePath)
            (*next)[0][0] = negative ? -1 : 0;
        return negative;
    }
    std::int64_t maximum = 0;
    bool has_negative = false;
    bool dense = true;
    constexpr auto unreachable = std::numeric_limits<std::int64_t>::max();
    for (std::size_t from = 0; from < n; ++from)
    {
        auto &row = distance[from];
        for (std::size_t to = 0; to < n; ++to)
        {
            auto &value = row[to];
            if (from == to)
                value = std::min(value, std::int64_t{0});
            if (value == inf) {
                dense = false;
                continue;
            }
            maximum = std::max(maximum, value < 0 ? -value : value);
            has_negative = has_negative || value < 0;
            if constexpr (RestorePath)
                (*next)[from][to] = static_cast<int>(to);
        }
    }
    // 非負で最長単純路もinf未満なら、未接続値と出力の変換が要らない。
    if (!has_negative && maximum <= (inf - 1) / static_cast<std::int64_t>(n) &&
        (!CustomInf || maximum <= unreachable / (2 * static_cast<std::int64_t>(n)))) {
        if (dense) return FloydWarshallRun<RestorePath, false, true, true, std::int64_t, UseSimd, CustomInf>(distance, inf, std::int64_t{0}, next, inf);
        return FloydWarshallRun<RestorePath, false, true, false, std::int64_t, UseSimd, CustomInf>(distance, inf, std::int64_t{0}, next, inf);
    }
    for (auto& row : distance)
        for (auto& value : row)
            if (value == inf) value = unreachable;
    // 単純路はn-1辺以下。下限は-n*maximumで、負閉路での際限ない減少を抑える。
    if (maximum <= unreachable / (2 * static_cast<std::int64_t>(n)))
    {
        const auto lower = -static_cast<std::int64_t>(n) * maximum;
        if (has_negative)
            return FloydWarshallRun<RestorePath, true, false, false, std::int64_t, UseSimd, CustomInf>(distance, unreachable, lower, next, inf);
        return FloydWarshallRun<RestorePath, false, false, false, std::int64_t, UseSimd, CustomInf>(distance, unreachable, lower, next, inf);
    }

    using Wide = FloydWarshallWide;
    const Wide wide_unreachable = (CustomInf ? FloydWarshallCustomLimit(n, inf) : FloydWarshallLimit(n)) + Wide{1};
    std::vector<std::vector<Wide>> wide(n, std::vector<Wide>(n));
    for (std::size_t from = 0; from < n; ++from)
        for (std::size_t to = 0; to < n; ++to)
            wide[from][to] = distance[from][to] == unreachable ? wide_unreachable : Wide{distance[from][to]};
    const bool negative = has_negative
        ? FloydWarshallRun<RestorePath, true, false, false, Wide, false, CustomInf>(wide, wide_unreachable, -(CustomInf ? FloydWarshallCustomLimit(n, inf) : FloydWarshallLimit(n)), next, inf)
        : FloydWarshallRun<RestorePath, false, false, false, Wide, false, CustomInf>(wide, wide_unreachable, -(CustomInf ? FloydWarshallCustomLimit(n, inf) : FloydWarshallLimit(n)), next, inf);
    for (std::size_t from = 0; from < n; ++from)
        for (std::size_t to = 0; to < n; ++to)
            distance[from][to] = static_cast<std::int64_t>(wide[from][to]);
    return negative;
}


} // namespace reinsirk::detail

inline bool FloydWarshall(std::vector<std::vector<std::int64_t>> &distances,
                          std::int64_t inf = (std::int64_t{1} << 60))
{
    if (inf <= 0) throw std::invalid_argument("FloydWarshall: inf must be positive");
    if (inf != kInf) return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<false, false, true>(distances, nullptr, inf);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<false>(distances, nullptr);
}

inline bool FloydWarshall(std::vector<std::vector<std::int64_t>> &distances,
                          std::vector<std::vector<int>> &next,
                          std::int64_t inf = (std::int64_t{1} << 60))
{
    if (inf <= 0) throw std::invalid_argument("FloydWarshall: inf must be positive");
    if (inf != kInf) return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<true, false, true>(distances, &next, inf);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<true>(distances, &next);
}

inline std::vector<int> FloydWarshallPath(const std::vector<std::vector<int>> &next,
                                        int from, int to)
{
    const std::size_t n = next.size();
    std::vector<int> path;
    const auto target = static_cast<std::size_t>(to);
    for (std::size_t steps = 0; steps <= n; ++steps)
    {
        const auto current = static_cast<std::size_t>(from);
        const int following = next[current][target];
        if (following == -1)
            return {};
        path.push_back(from);
        if (from == to)
            return path;
        from = following;
    }
    throw std::invalid_argument("FloydWarshallPath: cyclic next table");
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
