// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix_transform.md
//
// API一覧
// [行列変換]
// 以下の全操作はvector<vector<T>>版とvector<string>版を持つ。
// 全APIは矩形が前提。コピー版のvector<vector<T>>はコピー構築可能なT（boolも可）、vector<string>版は文字盤面。
// コピー版は入力不変・独立した所有権を返す。要素のコピー・swapをO(1)とし、H行W列のコピー・回転はO(H+HW)。
// 形状確認は通常処理で参照する行だけ。SKIP_RUNTIME_CHECKSで除外。inplaceは例外までの変更が残り得る。
// 幅0の90度回転・転置は先頭以外の行を見ない。上下inplaceは奇数行数の中央行を見ない。
// rotate_90r(a): 時計回り90度の新行列。外側が空なら互換性のため{{}}、H本の空行なら{}。
// flip_j(a): 左右反転の新行列。外側が空なら互換性のため{{}}、H本の空行は維持。
// flip_i(a): 上下反転の新行列。外側が空なら互換性のため{{}}、H本の空行は維持。
// rotate_90l(a): 反時計回り90度の新行列。外側が空または幅0なら{}。
// rotate_180(a): 180度回転の新行列。外側が空なら{}、H本の空行は維持。
// rotate_quarter(a, int clockwise_turns): 時計回りの回数をmod4で正規化（負数・INT_MIN可）。外側が空なら{}。
// transpose(a): 転置した新行列。外側が空または幅0なら{}。
// flip_i_inplace(a): 行の交換で上下反転しvoidを返す。要素のswap不要。O(H)時間・追加O(1)、外側が空なら何もしない。
// flip_j_inplace(a): 各行を左右反転しvoidを返す。Tはswap可能。O(H+HW)時間・追加O(1)、外側が空なら何もしない。
// SIRKIT_DISABLE_SIMD: 追加AVX2実装・専用ヘッダー・CPU判定を除き、標準C++の処理を使う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
#include <immintrin.h>
#endif



// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MATRIX_TRANSFORM_SIMD_DETAIL_DEFINED
#define SIRKIT_MATRIX_TRANSFORM_SIMD_DETAIL_DEFINED
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline bool matrix_avx2_available() noexcept {
    return __builtin_cpu_supports("avx2");
}
template <int Bytes, bool High>
[[gnu::target("avx2")]]
inline __m128i matrix_unpack_small(__m128i a, __m128i b) {
    if constexpr (Bytes == 1) return High ? _mm_unpackhi_epi8(a,b) : _mm_unpacklo_epi8(a,b);
    else if constexpr (Bytes == 2) return High ? _mm_unpackhi_epi16(a,b) : _mm_unpacklo_epi16(a,b);
    else if constexpr (Bytes == 4) return High ? _mm_unpackhi_epi32(a,b) : _mm_unpacklo_epi32(a,b);
    else return High ? _mm_unpackhi_epi64(a,b) : _mm_unpacklo_epi64(a,b);
}
#if defined(__clang__)
template <int Bytes, std::size_t Side, std::size_t Step = 1, std::size_t... K>
[[gnu::target("avx2")]]
inline void matrix_transpose_small(__m128i (&rows)[Side], std::index_sequence<K...>) {
    __m128i next[Side];
    ((next[(K/Step)*2*Step + 2*(K%Step)] = matrix_unpack_small<Bytes*Step, false>(rows[(K/Step)*2*Step + K%Step], rows[(K/Step)*2*Step + K%Step + Step]),
      next[(K/Step)*2*Step + 2*(K%Step) + 1] = matrix_unpack_small<Bytes*Step, true>(rows[(K/Step)*2*Step + K%Step], rows[(K/Step)*2*Step + K%Step + Step])), ...);
    ((rows[2*K] = next[2*K], rows[2*K+1] = next[2*K+1]), ...);
    if constexpr (Step * 2 < Side) matrix_transpose_small<Bytes, Side, Step*2>(rows, std::index_sequence<K...>{});
}
template <int Mode, class Row, std::size_t Tile>
[[gnu::target("avx2")]]
std::vector<Row> matrix_turn_small_avx2(const std::vector<Row>& a) {
    using Value = typename Row::value_type;
    constexpr std::size_t side = 16 / sizeof(Value);
    const std::size_t h = a.size(), w = a.front().size();
    std::vector<Row> b; b.reserve(w);
    for (std::size_t j = 0; j < w; ++j) b.emplace_back(h, Value{});
    for (std::size_t ib = 0; ib < h;) {
        const auto ie = ib + std::min(Tile, h - ib);
        for (std::size_t jb = 0; jb < w;) {
            const auto je = jb + std::min(Tile, w - jb);
            std::size_t i = ib;
            for (; ie - i >= side; i += side) {
                const Value* source[side];
                for (std::size_t r = 0; r < side; ++r) {
                    const auto& row = a[Mode == 1 ? h - 1 - i - r : i + r];
#ifndef SKIP_RUNTIME_CHECKS
                    if (jb == 0 && row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
                    source[r] = row.data();
                }
                std::size_t j = jb;
                for (; je - j >= side; j += side) {
                    __m128i rows[side];
                    for (std::size_t r = 0; r < side; ++r)
                        rows[r] = _mm_loadu_si128(reinterpret_cast<const __m128i*>(source[r] + j));
                    matrix_transpose_small<sizeof(Value)>(rows, std::make_index_sequence<side/2>{});
                    for (std::size_t c = 0; c < side; ++c)
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(b[Mode == 3 ? w - 1 - j - c : j + c].data() + i), rows[c]);
                }
                for (; j < je; ++j) {
                    const auto row = Mode == 3 ? w - 1 - j : j;
                    for (std::size_t r = 0; r < side; ++r) b[row][i+r] = source[r][j];
                }
            }
            for (; i < ie; ++i) {
                const auto& row = a[Mode == 1 ? h - 1 - i : i];
#ifndef SKIP_RUNTIME_CHECKS
                if (jb == 0 && row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
                for (std::size_t j = jb; j < je; ++j)
                    b[Mode == 3 ? w - 1 - j : j][i] = row[j];
            }
            jb = je;
        }
        ib = ie;
    }
    return b;
}
#else
template <int Bytes, std::size_t A, std::size_t B, std::size_t Side>
[[gnu::target("avx2")]]
inline void matrix_transpose_small_pair(__m128i (&rows)[Side]) {
    const auto low = matrix_unpack_small<Bytes, false>(rows[A], rows[B]);
    const auto high = matrix_unpack_small<Bytes, true>(rows[A], rows[B]);
    rows[A] = low; rows[B] = high;
}
template <int Bytes, std::size_t Side, std::size_t Step = 1, std::size_t... K>
[[gnu::target("avx2")]]
inline void matrix_transpose_small(__m128i (&rows)[Side], std::index_sequence<K...>) {
    (matrix_transpose_small_pair<Bytes*Step, (K/Step)*2*Step + K%Step, (K/Step)*2*Step + K%Step + Step>(rows), ...);
    if constexpr (Step * 2 < Side) matrix_transpose_small<Bytes, Side, Step*2>(rows, std::index_sequence<K...>{});
}
constexpr std::size_t matrix_small_reverse(std::size_t value, std::size_t side) noexcept {
    std::size_t result = 0;
    for (; side > 1; side >>= 1) { result = (result << 1) | (value & 1); value >>= 1; }
    return result;
}
template <int Mode, class Row, std::size_t Side, std::size_t... C>
[[gnu::target("avx2")]]
inline void matrix_store_small(std::vector<Row>& b, __m128i (&rows)[Side], std::size_t i, std::size_t j, std::size_t w, std::index_sequence<C...>) {
    (_mm_storeu_si128(reinterpret_cast<__m128i*>(b[Mode == 3 ? w - 1 - j - C : j + C].data() + i), rows[matrix_small_reverse(C, Side)]), ...);
}
template <int Mode, class Row, std::size_t Tile>
[[gnu::target("avx2")]]
std::vector<Row> matrix_turn_small_avx2(const std::vector<Row>& a) {
    using Value = typename Row::value_type;
    constexpr std::size_t side = 16 / sizeof(Value);
    const std::size_t h = a.size(), w = a.front().size();
    std::vector<Row> b; b.reserve(w);
    for (std::size_t j = 0; j < w; ++j) b.emplace_back(h, Value{});
    for (std::size_t ib = 0; ib < h;) {
        const auto ie = ib + std::min(Tile, h - ib);
        const Value* cached[Tile];
        for (std::size_t r = 0; r < ie - ib; ++r) {
            const auto& row = a[Mode == 1 ? h - 1 - ib - r : ib + r];
#ifndef SKIP_RUNTIME_CHECKS
            if (row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
            cached[r] = row.data();
        }
        for (std::size_t jb = 0; jb < w;) {
            const auto je = jb + std::min(Tile, w - jb);
            std::size_t i = ib;
            for (; ie - i >= side; i += side) {
                const auto* source = cached + (i - ib);
                std::size_t j = jb;
                for (; je - j >= side; j += side) {
                    __m128i rows[side];
                    for (std::size_t r = 0; r < side; ++r)
                        rows[r] = _mm_loadu_si128(reinterpret_cast<const __m128i*>(source[r] + j));
                    matrix_transpose_small<sizeof(Value)>(rows, std::make_index_sequence<side/2>{});
                    matrix_store_small<Mode>(b, rows, i, j, w, std::make_index_sequence<side>{});
                }
                for (; j < je; ++j) {
                    const auto row = Mode == 3 ? w - 1 - j : j;
                    for (std::size_t r = 0; r < side; ++r) b[row][i+r] = source[r][j];
                }
            }
            for (; i < ie; ++i) {
                const auto* row = cached[i - ib];
                for (std::size_t j = jb; j < je; ++j)
                    b[Mode == 3 ? w - 1 - j : j][i] = row[j];
            }
            jb = je;
        }
        ib = ie;
    }
    return b;
}
#endif
template <int Mode, class Row, bool Strip = false>
[[gnu::target("avx2")]]
std::vector<Row> matrix_turn64_avx2(const std::vector<Row>& a) {
    using Value = typename Row::value_type;
    const std::size_t h = a.size(), w = a.front().size();
    std::vector<Row> b; b.reserve(w);
    for (std::size_t j = 0; j < w; ++j) b.emplace_back(h, Value{});
    for (std::size_t jb = 0; jb < w;) {
        const auto je = jb + (Strip ? std::min(std::size_t{32}, w - jb) : w - jb);
        std::size_t i = 0;
        for (; h - i >= 4; i += 4) {
            const Value* source[4];
            for (std::size_t r = 0; r < 4; ++r) {
                const auto& row = a[Mode == 1 ? h - 1 - i - r : i + r];
#ifndef SKIP_RUNTIME_CHECKS
                if (jb == 0 && row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
                source[r] = row.data();
            }
            const auto column = i;
            std::size_t j = jb;
            for (; je - j >= 4; j += 4) {
                const auto r0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[0] + j));
                const auto r1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[1] + j));
                const auto r2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[2] + j));
                const auto r3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[3] + j));
                const auto t0 = _mm256_unpacklo_epi64(r0, r1), t1 = _mm256_unpackhi_epi64(r0, r1);
                const auto t2 = _mm256_unpacklo_epi64(r2, r3), t3 = _mm256_unpackhi_epi64(r2, r3);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j : j].data() + column), _mm256_permute2x128_si256(t0,t2,0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 2 - j : j+1].data() + column), _mm256_permute2x128_si256(t1,t3,0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 3 - j : j+2].data() + column), _mm256_permute2x128_si256(t0,t2,0x31));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 4 - j : j+3].data() + column), _mm256_permute2x128_si256(t1,t3,0x31));
            }
            for (; j < je; ++j) {
                const auto row = Mode == 3 ? w - 1 - j : j;
                for (std::size_t r = 0; r < 4; ++r) b[row][column+r] = source[r][j];
            }
        }
        for (; i < h; ++i) {
            const auto& row = a[Mode == 1 ? h - 1 - i : i];
#ifndef SKIP_RUNTIME_CHECKS
            if (jb == 0 && row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
            for (std::size_t j = jb; j < je; ++j) {
                if constexpr (Mode == 1) b[j][i] = row[j];
                else if constexpr (Mode == 3) b[w-1-j][i] = row[j];
                else b[j][i] = row[j];
            }
        }
        jb = je;
    }
    return b;
}
template <int Mode, class Row, bool Strip = false, bool Forward = false>
[[gnu::target("avx2")]]
std::vector<Row> matrix_turn_avx2(const std::vector<Row>& a) {
    using Value = typename Row::value_type;
    const std::size_t h = a.size(), w = a.front().size();
    std::vector<Row> b;
    b.reserve(w);
    for (std::size_t j = 0; j < w; ++j) b.emplace_back(h, Value{});
    // 横長の大行列だけ列を分割し、出力行を続けて使う。
    for (std::size_t jb = 0; jb < w;) {
        const auto je = jb + (Strip ? std::min(std::size_t{32}, w - jb) : w - jb);
        std::size_t i = 0;
        for (; h - i >= 8; i += 8) {
            const Value* source[8];
            for (std::size_t r = 0; r < 8; ++r) {
                const auto& row = a[Mode == 1 && Forward ? h - 1 - i - r : i + (Mode == 1 ? 7 - r : r)];
    #ifndef SKIP_RUNTIME_CHECKS
                if (jb == 0 && row.size() != w) throw std::invalid_argument("matrix must be rectangular");
    #endif
                source[r] = row.data();
            }
            const auto column = Mode == 1 && !Forward ? h - i - 8 : i;
            std::size_t j = jb;
            for (; je - j >= 8; j += 8) {
                const auto r0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[0] + j));
                const auto r1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[1] + j));
                const auto r2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[2] + j));
                const auto r3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[3] + j));
                const auto r4 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[4] + j));
                const auto r5 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[5] + j));
                const auto r6 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[6] + j));
                const auto r7 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source[7] + j));
                const auto t0 = _mm256_unpacklo_epi32(r0, r1);
                const auto t1 = _mm256_unpackhi_epi32(r0, r1);
                const auto t2 = _mm256_unpacklo_epi32(r2, r3);
                const auto t3 = _mm256_unpackhi_epi32(r2, r3);
                const auto t4 = _mm256_unpacklo_epi32(r4, r5);
                const auto t5 = _mm256_unpackhi_epi32(r4, r5);
                const auto t6 = _mm256_unpacklo_epi32(r6, r7);
                const auto t7 = _mm256_unpackhi_epi32(r6, r7);
                const auto s0 = _mm256_unpacklo_epi64(t0, t2);
                const auto s1 = _mm256_unpackhi_epi64(t0, t2);
                const auto s2 = _mm256_unpacklo_epi64(t1, t3);
                const auto s3 = _mm256_unpackhi_epi64(t1, t3);
                const auto s4 = _mm256_unpacklo_epi64(t4, t6);
                const auto s5 = _mm256_unpackhi_epi64(t4, t6);
                const auto s6 = _mm256_unpacklo_epi64(t5, t7);
                const auto s7 = _mm256_unpackhi_epi64(t5, t7);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 0 : j + 0].data() + column),
                    _mm256_permute2x128_si256(s0, s4, 0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 1 : j + 1].data() + column),
                    _mm256_permute2x128_si256(s1, s5, 0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 2 : j + 2].data() + column),
                    _mm256_permute2x128_si256(s2, s6, 0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 3 : j + 3].data() + column),
                    _mm256_permute2x128_si256(s3, s7, 0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 4 : j + 4].data() + column),
                    _mm256_permute2x128_si256(s0, s4, 0x31));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 5 : j + 5].data() + column),
                    _mm256_permute2x128_si256(s1, s5, 0x31));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 6 : j + 6].data() + column),
                    _mm256_permute2x128_si256(s2, s6, 0x31));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(b[Mode == 3 ? w - 1 - j - 7 : j + 7].data() + column),
                    _mm256_permute2x128_si256(s3, s7, 0x31));
            }
            for (; j < je; ++j) {
                const auto row = Mode == 3 ? w - 1 - j : j;
                for (std::size_t r = 0; r < 8; ++r) b[row][column + r] = source[r][j];
            }
        }
        for (; i < h; ++i) {
            const auto& row = a[Mode == 1 && Forward ? h - 1 - i : i];
    #ifndef SKIP_RUNTIME_CHECKS
            if (jb == 0 && row.size() != w) throw std::invalid_argument("matrix must be rectangular");
    #endif
            for (std::size_t j = jb; j < je; ++j) {
                if constexpr (Mode == 1) b[j][Forward ? i : h - 1 - i] = row[j];
                else if constexpr (Mode == 3) b[w - 1 - j][i] = row[j];
                else b[j][i] = row[j];
            }
        }
        jb = je;
    }
    return b;
}
}
#endif
#endif

#ifndef SIRKIT_MATRIX_TRANSFORM_DETAIL_DEFINED
#define SIRKIT_MATRIX_TRANSFORM_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {

// 内部用。Mode: 0=転置、1=時計回り、3=反時計回り。
template <int Mode, class Row>
std::vector<Row> matrix_turn(const std::vector<Row>& a) {
    const std::size_t h = a.size();
    if (h == 0) return {};
    const std::size_t w = a.front().size();
    if (w == 0) return {};
    using Value = typename Row::value_type;
    constexpr bool simple_value = std::is_trivially_copyable_v<Value>
        && std::is_trivially_default_constructible_v<Value>
        && std::is_trivially_copy_assignable_v<Value>;
    // 背の低い行列は出力行を順に構築し、各行の確保と書込みをまとめる。
    if constexpr (simple_value) {
        if (h < 32 && w >= 32 && !std::is_same_v<Value, bool>) {
            std::vector<Row> b;
            b.reserve(w);
            const auto append = [&]<bool Check>(std::size_t col) {
                b.emplace_back();
                auto& dest = b.back();
                dest.reserve(h);
                const std::size_t j = Mode == 3 ? w - 1 - col : col;
                for (std::size_t i = 0; i < h; ++i) {
                    const auto& row = a[Mode == 1 ? h - 1 - i : i];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr (Check) {
                        if (row.size() != w) throw std::invalid_argument("matrix must be rectangular");
                    }
#endif
                    dest.push_back(row[j]);
                }
            };
            append.template operator()<true>(0);
            for (std::size_t j = 1; j < w; ++j) append.template operator()<false>(j);
            return b;
        }
    }
    // 大きな単純型の行列は、部分行列単位で書込み先をキャッシュに保つ。
    if constexpr (simple_value) {
        if (h >= 32 && w >= 32) {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
            if constexpr ((sizeof(Value) == 1 || sizeof(Value) == 2) && !std::is_same_v<Value, bool> &&
                          (std::is_integral_v<Value> || std::is_enum_v<Value>)) {
                if (h > 4095 / w && matrix_avx2_available()) return matrix_turn_small_avx2<Mode, Row, 32>(a);
            } else if constexpr (sizeof(Value) == 4 &&
                          (std::is_arithmetic_v<Value> || std::is_enum_v<Value>)) {
                if (h > 4095 / w && matrix_avx2_available()) {
                    if constexpr (Mode == 1) if (w >= 128 && h > 1048575 / w) {
                        if (h <= 64 && w >= 8192 && h > 524287 / w) return matrix_turn_avx2<Mode, Row, true, true>(a);
                        return matrix_turn_avx2<Mode, Row, false, true>(a);
                    }
                    if (h <= 64 && w >= 8192 && h > 524287 / w) return matrix_turn_avx2<Mode, Row, true>(a);
                    return matrix_turn_avx2<Mode>(a);
                }
            } else if constexpr (sizeof(Value) == 8 &&
                          (std::is_arithmetic_v<Value> || std::is_enum_v<Value>)) {
                if (h > 4095 / w && matrix_avx2_available()) {
                    if (w >= 128) return matrix_turn64_avx2<Mode, Row, true>(a);
                    return matrix_turn64_avx2<Mode>(a);
                }
            }
#endif

            std::vector<Row> b;
            b.reserve(w);
            for (std::size_t j = 0; j < w; ++j) b.emplace_back(h, Value{});
            constexpr std::size_t block = 32;
            for (std::size_t ib = 0; ib < h;) {
                const std::size_t ie = ib + std::min(block, h - ib);
                for (std::size_t jb = 0; jb < w;) {
                    const std::size_t je = jb + std::min(block, w - jb);
                    for (std::size_t i = ib; i < ie; ++i) {
                        const auto& row = a[i];
#ifndef SKIP_RUNTIME_CHECKS
                        if (jb == 0 && row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
                        for (std::size_t j = jb; j < je; ++j) {
                            if constexpr (Mode == 1) b[j][h - 1 - i] = row[j];
                            else if constexpr (Mode == 3) b[w - 1 - j][i] = row[j];
                            else b[j][i] = row[j];
                        }
                    }
                    jb = je;
                }
                ib = ie;
            }
            return b;
        }
    }
    // 単純型の小・縦長行列は各行を一度だけ初期化し、要素追加の分岐を省く。
    if constexpr (simple_value && !std::is_same_v<Value, bool>) {
        if (h >= 8 && (w >= 8 || h >= 32)) {
            std::vector<Row> b;
            b.reserve(w);
            for (std::size_t j = 0; j < w; ++j) b.emplace_back(h, Value{});
            for (std::size_t i = 0; i < h; ++i) {
                const auto& row = a[i];
#ifndef SKIP_RUNTIME_CHECKS
                if (row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
                for (std::size_t j = 0; j < w; ++j) {
                    if constexpr (Mode == 1) b[j][h - 1 - i] = row[j];
                    else if constexpr (Mode == 3) b[w - 1 - j][i] = row[j];
                    else b[j][i] = row[j];
                }
            }
            return b;
        }
    }
    std::vector<Row> b(w);
    for (auto& row : b) row.reserve(h);
    for (std::size_t i = 0; i < h; ++i) {
        const auto& row = a[Mode == 1 ? h - 1 - i : i];
#ifndef SKIP_RUNTIME_CHECKS
        if (row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
        for (std::size_t j = 0; j < w; ++j) {
            if constexpr (Mode == 3) b[w - 1 - j].push_back(row[j]);
            else b[j].push_back(row[j]);
        }
    }
    return b;
}

// 内部用。行のコピーと形状確認を同時に行う。
template <bool ReverseRows, bool ReverseColumns, class Row>
std::vector<Row> matrix_copy(const std::vector<Row>& a) {
    const std::size_t h = a.size();
    std::vector<Row> b;
    b.reserve(h);
    if (h == 0) return b;
#ifndef SKIP_RUNTIME_CHECKS
    const std::size_t w = a.front().size();
#endif
    for (std::size_t i = 0; i < h; ++i) {
        const auto& row = a[ReverseRows ? h - 1 - i : i];
#ifndef SKIP_RUNTIME_CHECKS
        if (row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
        if constexpr (ReverseColumns) b.emplace_back(row.rbegin(), row.rend());
        else b.push_back(row);
    }
    return b;
}

template <class Row>
void matrix_flip_i_inplace(std::vector<Row>& a) {
    const std::size_t h = a.size();
    if (h == 0) return;
#ifndef SKIP_RUNTIME_CHECKS
    const std::size_t w = a.front().size();
#endif
    for (std::size_t i = 0; i < h / 2; ++i) {
        auto& first = a[i];
        auto& last = a[h - 1 - i];
#ifndef SKIP_RUNTIME_CHECKS
        if (first.size() != w || last.size() != w)
            throw std::invalid_argument("matrix must be rectangular");
#endif
        first.swap(last);
    }
}

template <class Row>
void matrix_flip_j_inplace(std::vector<Row>& a) {
    if (a.empty()) return;
#ifndef SKIP_RUNTIME_CHECKS
    const std::size_t w = a.front().size();
#endif
    for (auto& row : a) {
#ifndef SKIP_RUNTIME_CHECKS
        if (row.size() != w) throw std::invalid_argument("matrix must be rectangular");
#endif
        std::reverse(row.begin(), row.end());
    }
}

template <class Row>
std::vector<Row> matrix_quarter(const std::vector<Row>& a, int turns) {
    const int r = turns % 4;
    switch (r < 0 ? r + 4 : r) {
        case 0: return matrix_copy<false, false>(a);
        case 1: return matrix_turn<1>(a);
        case 2: return matrix_copy<true, true>(a);
        default: return matrix_turn<3>(a);
    }
}
} // namespace reinsirk::detail
#endif

#ifndef SIRKIT_ROTATE_90R_DEFINED
#define SIRKIT_ROTATE_90R_DEFINED
template <class T>
std::vector<std::vector<T>> rotate_90r(const std::vector<std::vector<T>>& a) {
    if (a.empty()) return std::vector<std::vector<T>>(1);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_turn<1>(a);
}
inline std::vector<std::string> rotate_90r(const std::vector<std::string>& a) {
    if (a.empty()) return std::vector<std::string>(1);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_turn<1>(a);
}
#endif

#ifndef SIRKIT_FLIP_J_DEFINED
#define SIRKIT_FLIP_J_DEFINED
template <class T>
std::vector<std::vector<T>> flip_j(const std::vector<std::vector<T>>& a) {
    if (a.empty()) return std::vector<std::vector<T>>(1);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_copy<false, true>(a);
}
inline std::vector<std::string> flip_j(const std::vector<std::string>& a) {
    if (a.empty()) return std::vector<std::string>(1);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_copy<false, true>(a);
}
#endif

#ifndef SIRKIT_FLIP_I_DEFINED
#define SIRKIT_FLIP_I_DEFINED
template <class T>
std::vector<std::vector<T>> flip_i(const std::vector<std::vector<T>>& a) {
    if (a.empty()) return std::vector<std::vector<T>>(1);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_copy<true, false>(a);
}
inline std::vector<std::string> flip_i(const std::vector<std::string>& a) {
    if (a.empty()) return std::vector<std::string>(1);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_copy<true, false>(a);
}
#endif

#ifndef SIRKIT_ROTATE_90L_DEFINED
#define SIRKIT_ROTATE_90L_DEFINED
template <class T>
std::vector<std::vector<T>> rotate_90l(const std::vector<std::vector<T>>& a) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_turn<3>(a);
}
inline std::vector<std::string> rotate_90l(const std::vector<std::string>& a) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_turn<3>(a);
}
#endif

#ifndef SIRKIT_ROTATE_180_DEFINED
#define SIRKIT_ROTATE_180_DEFINED
template <class T>
std::vector<std::vector<T>> rotate_180(const std::vector<std::vector<T>>& a) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_copy<true, true>(a);
}
inline std::vector<std::string> rotate_180(const std::vector<std::string>& a) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_copy<true, true>(a);
}
#endif

#ifndef SIRKIT_ROTATE_QUARTER_DEFINED
#define SIRKIT_ROTATE_QUARTER_DEFINED
template <class T>
std::vector<std::vector<T>> rotate_quarter(const std::vector<std::vector<T>>& a, int clockwise_turns) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_quarter(a, clockwise_turns);
}
inline std::vector<std::string> rotate_quarter(const std::vector<std::string>& a, int clockwise_turns) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_quarter(a, clockwise_turns);
}
#endif

#ifndef SIRKIT_TRANSPOSE_DEFINED
#define SIRKIT_TRANSPOSE_DEFINED
template <class T>
std::vector<std::vector<T>> transpose(const std::vector<std::vector<T>>& a) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_turn<0>(a);
}
inline std::vector<std::string> transpose(const std::vector<std::string>& a) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_turn<0>(a);
}
#endif

#ifndef SIRKIT_FLIP_I_INPLACE_DEFINED
#define SIRKIT_FLIP_I_INPLACE_DEFINED
template <class T>
void flip_i_inplace(std::vector<std::vector<T>>& a) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_flip_i_inplace(a);
}
inline void flip_i_inplace(std::vector<std::string>& a) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_flip_i_inplace(a);
}
#endif

#ifndef SIRKIT_FLIP_J_INPLACE_DEFINED
#define SIRKIT_FLIP_J_INPLACE_DEFINED
template <class T>
void flip_j_inplace(std::vector<std::vector<T>>& a) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_flip_j_inplace(a);
}
inline void flip_j_inplace(std::vector<std::string>& a) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::matrix_flip_j_inplace(a);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
