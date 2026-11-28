// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Incremental_APSP.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_apsp.md
//
// API一覧
// IncrementalAPSPEdge{from,to,weight}: 0始まりの端点とuint32_tの非負重み。
// IncrementalAPSP(n,edges={},undirected=false): 初期の全点対最短距離を構築する。
//   初期辺がある場合O(n³+m)時間、なければO(n²)時間。保持領域O(n²)。
// add_edge(from,to,weight): 辺を追加し、既存の辺より小さい重みなら距離を更新する。O(n²)。
//   既存の最短距離以下にできない辺はO(1)で無視する。辺の削除や重みの増加は扱わない。
// distance(from,to): int64_tの最短距離。到達不能ならnullopt。size(): 頂点数。ともにO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
#include <immintrin.h>
#endif


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

#if defined(__SIZEOF_INT128__)
using FloydWarshallWide = __int128_t;
#else
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
#endif

// 広幅実装でも下限の絶対値は単純路の最大絶対値より大きい。
inline FloydWarshallWide FloydWarshallLimit(std::size_t n)
{
#if defined(__SIZEOF_INT128__)
    return static_cast<FloydWarshallWide>(n) * kInf;
#else
    return {static_cast<std::int64_t>(n >> 4), static_cast<std::uint64_t>(n) << 60};
#endif
}

// n<=INT_MAX、inf<=INT64_MAXなので積は94bit以内。標準経路も積を分割する。
inline FloydWarshallWide FloydWarshallCustomLimit(std::size_t n, std::int64_t inf)
{
#if defined(__SIZEOF_INT128__)
    return static_cast<FloydWarshallWide>(n) * inf;
#else
    const auto value = static_cast<std::uint64_t>(inf);
    const auto first = static_cast<std::uint64_t>(n) * (value & 0xffffffffu);
    const auto second = static_cast<std::uint64_t>(n) * (value >> 32);
    const auto low = first + (second << 32);
    return {static_cast<std::int64_t>((second >> 32) + (low < first)), low};
#endif
}

#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
template <bool RestorePath, bool HasNegative,
          bool Dense>
[[gnu::target("avx2")]]
inline void FloydWarshallRelaxAvx2(std::vector<std::vector<std::int64_t>> &distance,
                                 std::int64_t unreachable, std::int64_t lower,
                                 std::vector<std::vector<int>> *next)
{
    const std::size_t n = distance.size();
    const __m256i v_unreachable = _mm256_set1_epi64x(unreachable);
    const __m256i v_lower = _mm256_set1_epi64x(lower);
    for (std::size_t via = 0; via < n; ++via)
    {
        const auto *through = distance[via].data();
        for (std::size_t from = 0; from < n; ++from)
        {
            auto *row = distance[from].data();
            const auto prefix = row[via];
            if (prefix == unreachable)
                continue;
            const __m256i v_prefix = _mm256_set1_epi64x(prefix);
            int *path = nullptr;
            int following = 0;
            if constexpr (RestorePath)
            {
                path = (*next)[from].data();
                following = path[via];
            }
            std::size_t to = 0;
            for (; to + 4 <= n; to += 4)
            {
                const __m256i values = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(through + to));
                const __m256i old = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(row + to));
                __m256i candidate = _mm256_add_epi64(v_prefix, values);
                if constexpr (HasNegative)
                    candidate = _mm256_blendv_epi8(candidate, v_lower, _mm256_cmpgt_epi64(v_lower, candidate));
                __m256i improve = _mm256_cmpgt_epi64(old, candidate);
                // 未到達laneの加算結果は捨て、到達済みのlaneだけ更新する。
                if constexpr (!Dense)
                    improve = _mm256_andnot_si256(_mm256_cmpeq_epi64(values, v_unreachable), improve);
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(row + to), _mm256_blendv_epi8(old, candidate, improve));
                if constexpr (RestorePath)
                {
                    const __m128 lo = _mm_castsi128_ps(_mm256_castsi256_si128(improve));
                    const __m128 hi = _mm_castsi128_ps(_mm256_extracti128_si256(improve, 1));
                    const __m128i mask = _mm_castps_si128(_mm_shuffle_ps(lo, hi, _MM_SHUFFLE(2, 0, 2, 0)));
                    const __m128i previous = _mm_loadu_si128(reinterpret_cast<const __m128i *>(path + to));
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(path + to),
                                     _mm_blendv_epi8(previous, _mm_set1_epi32(following), mask));
                }
            }
            for (; to < n; ++to)
            {
                if constexpr (!Dense)
                    if (through[to] == unreachable)
                        continue;
                auto candidate = prefix + through[to];
                if constexpr (HasNegative)
                    candidate = std::max(candidate, lower);
                if (candidate < row[to])
                {
                    row[to] = candidate;
                    if constexpr (RestorePath)
                        path[to] = following;
                }
            }
        }
    }
}
#endif
template <bool RestorePath, bool HasNegative, bool FiniteRange = false, bool Dense = false, class Distance, auto UseSimd = false, bool CustomInf = false>
inline bool FloydWarshallRun(std::vector<std::vector<Distance>> &distance,
                                    Distance unreachable, Distance lower,
                                    std::vector<std::vector<int>> *next, std::int64_t requested_inf = kInf)
{
    const std::int64_t inf = CustomInf ? requested_inf : kInf;
    const std::size_t n = distance.size();
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
    if constexpr (static_cast<bool>(UseSimd) && std::is_same_v<Distance, std::int64_t>)
        FloydWarshallRelaxAvx2<RestorePath, HasNegative,
                               Dense || (FiniteRange && std::is_same_v<decltype(UseSimd), FloydWarshallSimdMode>)>(distance, unreachable, lower, next);
    else
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
        if (distance[0].size() != 1)
            throw std::invalid_argument("FloydWarshall: matrix must be square");
        if (distance[0][0] != inf && (distance[0][0] <= -inf || distance[0][0] >= inf))
            throw std::out_of_range("FloydWarshall: weight must be in (-INF, INF)");
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
        if (row.size() != n)
            throw std::invalid_argument("FloydWarshall: matrix must be square");
#endif
        for (std::size_t to = 0; to < n; ++to)
        {
            auto &value = row[to];
#ifndef SKIP_RUNTIME_CHECKS
            if (value != inf && (value <= -inf || value >= inf))
                throw std::out_of_range("FloydWarshall: weight must be in (-INF, INF)");
#endif
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

#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
template <bool RestorePath>
[[gnu::noinline]] inline bool FloydWarshallVectorized(
    std::vector<std::vector<std::int64_t>> &distance, std::vector<std::vector<int>> *next)
{
    return FloydWarshallImpl<RestorePath, true>(distance, next);
}
#endif

} // namespace reinsirk::detail

inline bool FloydWarshall(std::vector<std::vector<std::int64_t>> &distances,
                          std::int64_t inf = (std::int64_t{1} << 60))
{
    if (inf <= 0) throw std::invalid_argument("FloydWarshall: inf must be positive");
    if (inf != kInf) return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<false, false, true>(distances, nullptr, inf);
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__))) && (!(defined(__AVX2__)))
    constexpr std::size_t threshold = 32;
    if (distances.size() >= threshold && __builtin_cpu_supports("avx2")) {
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallVectorized<false>(distances, nullptr);
    }
#endif
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<false>(distances, nullptr);
}

inline bool FloydWarshall(std::vector<std::vector<std::int64_t>> &distances,
                          std::vector<std::vector<int>> &next,
                          std::int64_t inf = (std::int64_t{1} << 60))
{
    if (inf <= 0) throw std::invalid_argument("FloydWarshall: inf must be positive");
    if (inf != kInf) return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<true, false, true>(distances, &next, inf);
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
    constexpr std::size_t threshold = 16;
    if (distances.size() >= threshold && __builtin_cpu_supports("avx2")) {
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallVectorized<true>(distances, &next);
    }
#endif
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FloydWarshallImpl<true>(distances, &next);
}

inline std::vector<int> FloydWarshallPath(const std::vector<std::vector<int>> &next,
                                        int from, int to)
{
    const std::size_t n = next.size();
#ifndef SKIP_RUNTIME_CHECKS
    if (from < 0 || to < 0 || static_cast<std::size_t>(from) >= n || static_cast<std::size_t>(to) >= n)
        throw std::out_of_range("FloydWarshallPath: invalid vertex");
#endif
    std::vector<int> path;
    const auto target = static_cast<std::size_t>(to);
    for (std::size_t steps = 0; steps <= n; ++steps)
    {
        const auto current = static_cast<std::size_t>(from);
#ifndef SKIP_RUNTIME_CHECKS
        if (next[current].size() != n)
            throw std::invalid_argument("FloydWarshallPath: table must be square");
#endif
        const int following = next[current][target];
        if (following == -1)
            return {};
        path.push_back(from);
        if (from == to)
            return path;
#ifndef SKIP_RUNTIME_CHECKS
        if (following < 0 || static_cast<std::size_t>(following) >= n)
            throw std::out_of_range("FloydWarshallPath: invalid next vertex");
#endif
        from = following;
    }
    throw std::invalid_argument("FloydWarshallPath: cyclic next table");
}
#endif

#ifndef SIRKIT_INCREMENTAL_APSP_DEFINED
#define SIRKIT_INCREMENTAL_APSP_DEFINED

struct IncrementalAPSPEdge
{
    int from, to;
    std::uint32_t weight;
};

class IncrementalAPSP
{
public:
    explicit IncrementalAPSP(int n, std::span<const IncrementalAPSPEdge> edges = {}, bool undirected = false)
        : undirected_(undirected)
    {
        if (n < 0)
        {
            throw std::invalid_argument("incremental APSP: negative vertex count");
        }
        distance_.assign(n, std::vector<std::int64_t>(n, kAbsent));
        tail_.resize(n);
        for (int vertex = 0; vertex < n; ++vertex)
        {
            distance_[vertex][vertex] = 0;
        }
        for (const auto &[u, v, weight] : edges)
        {
            check_vertex(u);
            check_vertex(v);
            const auto w = static_cast<std::int64_t>(weight);
            distance_[u][v] = std::min(distance_[u][v], w);
            if (undirected)
            {
                distance_[v][u] = std::min(distance_[v][u], w);
            }
        }
        if (!edges.empty())
        {
            FloydWarshall(distance_, kAbsent);
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return distance_.size();
    }

    void add_edge(int from, int to, std::uint32_t weight)
    {
        check_vertex(from);
        check_vertex(to);
        add_arc(from, to, weight);
        if (undirected_)
        {
            add_arc(to, from, weight);
        }
    }

    [[nodiscard]] std::optional<std::int64_t> distance(int from, int to) const
    {
        check_vertex(from);
        check_vertex(to);
        const auto value = distance_[from][to];
        if (value == kAbsent)
        {
            return std::nullopt;
        }
        return value;
    }

private:
    static constexpr auto kAbsent = std::numeric_limits<std::int64_t>::max();

    void check_vertex([[maybe_unused]] int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= size())
        {
            throw std::out_of_range("incremental APSP: vertex out of range");
        }
#endif
    }

    void add_arc(int from, int to, std::int64_t weight)
    {
        if (distance_[from][to] <= weight)
        {
            return;
        }
        // 新しい辺を使う最短路の後半を、行を書き換える前に保存する。
        std::ranges::copy(distance_[to], tail_.begin());
        for (auto &row : distance_)
        {
            if (row[from] >= kAbsent - weight)
            {
                continue;
            }
            const auto prefix = row[from] + weight;
            const auto limit = kAbsent - prefix;
            for (std::size_t j = 0; j < size(); ++j)
            {
                if (tail_[j] < limit)
                {
                    row[j] = std::min(row[j], prefix + tail_[j]);
                }
            }
        }
    }

    bool undirected_;
    std::vector<std::vector<std::int64_t>> distance_;
    std::vector<std::int64_t> tail_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
