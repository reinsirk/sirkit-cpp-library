// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/rotate.cpp
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



#ifndef SIRKIT_MATRIX_TRANSFORM_SIMD_DETAIL_DEFINED
#define SIRKIT_MATRIX_TRANSFORM_SIMD_DETAIL_DEFINED
#endif


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
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
    for (std::size_t i = 0; i < h; ++i) {
        const auto& row = a[ReverseRows ? h - 1 - i : i];
        if constexpr (ReverseColumns) b.emplace_back(row.rbegin(), row.rend());
        else b.push_back(row);
    }
    return b;
}

template <class Row>
void matrix_flip_i_inplace(std::vector<Row>& a) {
    const std::size_t h = a.size();
    if (h == 0) return;
    for (std::size_t i = 0; i < h / 2; ++i) {
        auto& first = a[i];
        auto& last = a[h - 1 - i];
        first.swap(last);
    }
}

template <class Row>
void matrix_flip_j_inplace(std::vector<Row>& a) {
    if (a.empty()) return;
    for (auto& row : a) {
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
