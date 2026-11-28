// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Xor_Convolution_Without_Division.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/xor_convolution_without_division.md
// 参考（基底変換とランク付き畳み込み）: https://info.atcoder.jp/entry/algorithm_lectures/division_free_xor_convolution
//
// API一覧
// xor_convolution_without_division(a,b): c[k]=sum_(i^j=k) a[i]*b[j]を除算せずに計算。
// 入力は同じ長さの空列または2冪。T()は加法単位元、Tには+=、-=、*=が必要。
// xor_convolution_without_division<true>(a,b): すべての元についてx+x=0の場合に積の回数を減らす。
// 符号付き整数型では計算途中の値も表現可能であること。符号なし整数型は型の幅で剰余を取る。
// 長さNに対しO(1+N log²(N+1))時間、O(N log(N+1))領域。

// SIRKIT_BUNDLED_DEPENDENCIES


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
//
// [同梱の集合の畳み込み]
// [集合のゼータ／メビウス変換]
// 集合4変換: 空配列または長さn=2^kを直接更新、O(1+n log(n+1))時間・O(1)追加領域。
// superset_zeta_transform(f): f[S]を上位集合の総和へ更新、Tの+=が必要。
// superset_mobius_transform(f): 上位集合ゼータ変換の逆、Tの-=が必要。
// subset_zeta_transform(f): f[S]を部分集合の総和へ更新、Tの+=が必要。
// subset_mobius_transform(f): 部分集合ゼータ変換の逆、Tの-=が必要。
// Tはbool以外の加法型。符号付き整数の全途中結果は型の範囲内、符号なしは型の幅の剰余。値演算送出時は部分更新。
//
// [AND／OR畳み込み]
// bitwise_and_convolution(f,g): h[k]=sum_{i&j=k}f[i]g[j]を返す。同長の空配列または長さn=2^k。
// bitwise_or_convolution(f,g): h[k]=sum_{i|j=k}f[i]g[j]を返す。同長の空配列または長さn=2^k。
// bitwise_and_square(f)/bitwise_or_square(f): 同じ式のg=fを返す。入力1本、変換2回（通常の畳み込みは3回）。
// 各APIはO(1+n log(n+1))時間・O(n+1)領域。値渡しのためlvalue入力は保持、rvalueは領域を移譲できる。
// Tはbool以外の環型で+=/-=/*=が必要。符号付き整数の全途中結果は型の範囲内、符号なしは型の幅の剰余。

#include <bit>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SET_TRANSFORM_SCALAR_DETAIL_DEFINED
#define SIRKIT_SET_TRANSFORM_SCALAR_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <bool Subset, bool Subtract, class T>
inline void set_transform_scalar(std::vector<T>& f) {
    const std::size_t n = f.size();
    if (n >= 8) {
        for (std::size_t base = 0; base < n; base += 8) {
            auto add = [&](std::size_t a, std::size_t b) {
                if constexpr (Subtract) f[base + a] -= f[base + b];
                else f[base + a] += f[base + b];
            };
            if constexpr (Subset) {
                add(1, 0); add(3, 2); add(5, 4); add(7, 6);
                add(2, 0); add(3, 1); add(6, 4); add(7, 5);
                add(4, 0); add(5, 1); add(6, 2); add(7, 3);
            } else {
                add(0, 1); add(2, 3); add(4, 5); add(6, 7);
                add(0, 2); add(1, 3); add(4, 6); add(5, 7);
                add(0, 4); add(1, 5); add(2, 6); add(3, 7);
            }
        }
    }
    std::size_t half = n >= 8 ? 8 : 1;
    for (; half < n; half *= 2) {
        for (std::size_t base = 0; base < n; base += half * 2) {
            {
                for (std::size_t j = 0; j < half; ++j) {
                    auto& out = f[base + j + (Subset ? half : 0)];
                    auto& in = f[base + j + (Subset ? 0 : half)];
                    if constexpr (Subtract) out -= in;
                    else out += in;
                }
            }
        }
    }
}
}
#endif

#ifndef SIRKIT_SET_TRANSFORM_SIMD_DETAIL_DEFINED
#define SIRKIT_SET_TRANSFORM_SIMD_DETAIL_DEFINED
#endif


#ifndef SIRKIT_SUPERSET_ZETA_TRANSFORM_DEFINED
#define SIRKIT_SUPERSET_ZETA_TRANSFORM_DEFINED
template <class T> void superset_zeta_transform(std::vector<T>& f) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_scalar<false, false>(f);
}
#endif

#ifndef SIRKIT_SUPERSET_MOBIUS_TRANSFORM_DEFINED
#define SIRKIT_SUPERSET_MOBIUS_TRANSFORM_DEFINED
template <class T> void superset_mobius_transform(std::vector<T>& f) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_scalar<false, true>(f);
}
#endif

#ifndef SIRKIT_SUBSET_ZETA_TRANSFORM_DEFINED
#define SIRKIT_SUBSET_ZETA_TRANSFORM_DEFINED
template <class T> void subset_zeta_transform(std::vector<T>& f) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_scalar<true, false>(f);
}
#endif

#ifndef SIRKIT_SUBSET_MOBIUS_TRANSFORM_DEFINED
#define SIRKIT_SUBSET_MOBIUS_TRANSFORM_DEFINED
template <class T> void subset_mobius_transform(std::vector<T>& f) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_scalar<true, true>(f);
}
#endif

#ifndef SIRKIT_BITWISE_MULTIPLY_DETAIL_DEFINED
#define SIRKIT_BITWISE_MULTIPLY_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <class T> inline void bitwise_multiply_assign(T& left, T& right) {
    if constexpr (std::is_unsigned_v<T> && sizeof(T) < sizeof(unsigned int))
        left = static_cast<T>(static_cast<unsigned int>(left) * static_cast<unsigned int>(right));
    else left *= right;
}
}
#endif

#ifndef SIRKIT_BITWISE_AND_CONVOLUTION_DEFINED
#define SIRKIT_BITWISE_AND_CONVOLUTION_DEFINED
template <class T> std::vector<T> bitwise_and_convolution(std::vector<T> f, std::vector<T> g) {
    superset_zeta_transform(f);
    superset_zeta_transform(g);
    for (std::size_t i = 0; i < f.size(); ++i) /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::bitwise_multiply_assign(f[i], g[i]);
    superset_mobius_transform(f);
    return f;
}
#endif

#ifndef SIRKIT_BITWISE_OR_CONVOLUTION_DEFINED
#define SIRKIT_BITWISE_OR_CONVOLUTION_DEFINED
template <class T> std::vector<T> bitwise_or_convolution(std::vector<T> f, std::vector<T> g) {
    subset_zeta_transform(f);
    subset_zeta_transform(g);
    for (std::size_t i = 0; i < f.size(); ++i) /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::bitwise_multiply_assign(f[i], g[i]);
    subset_mobius_transform(f);
    return f;
}
#endif


#ifndef SIRKIT_BITWISE_AND_SQUARE_DEFINED
#define SIRKIT_BITWISE_AND_SQUARE_DEFINED
template <class T> std::vector<T> bitwise_and_square(std::vector<T> f) {
    superset_zeta_transform(f);
    for (std::size_t i = 0; i < f.size(); ++i)
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::bitwise_multiply_assign(f[i], f[i]);
    superset_mobius_transform(f);
    return f;
}
#endif

#ifndef SIRKIT_BITWISE_OR_SQUARE_DEFINED
#define SIRKIT_BITWISE_OR_SQUARE_DEFINED
template <class T> std::vector<T> bitwise_or_square(std::vector<T> f) {
    subset_zeta_transform(f);
    for (std::size_t i = 0; i < f.size(); ++i)
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::bitwise_multiply_assign(f[i], f[i]);
    subset_mobius_transform(f);
    return f;
}
#endif

#ifndef SIRKIT_XOR_CONVOLUTION_WITHOUT_DIVISION_DEFINED
#define SIRKIT_XOR_CONVOLUTION_WITHOUT_DIVISION_DEFINED

template <bool CharacteristicTwo = false, class T>
    requires(!std::same_as<T, bool>)
[[nodiscard]] std::vector<T> xor_convolution_without_division(std::vector<T> a, std::vector<T> b)
{
    const auto size = a.size();
    if (size != b.size())
    {
        throw std::invalid_argument("xor_convolution_without_division: inputs differ in length");
    }
    if (size == 0)
    {
        return {};
    }
    if (!std::has_single_bit(size))
    {
        throw std::invalid_argument("xor_convolution_without_division: input length is not a power of two");
    }

    // x_i²=1 の基底から y_i=1+x_i に替えると y_i²=2*y_i。
    superset_mobius_transform(a);
    superset_mobius_transform(b);
    const auto dimensions = static_cast<std::size_t>(std::countr_zero(size));
    std::vector<std::uint8_t> ranks(size);
    std::vector<std::vector<T>> left(dimensions + 1), right(dimensions + 1);
    for (std::size_t rank = 0; rank <= dimensions; ++rank)
    {
        left[rank].resize(size);
        right[rank].resize(size);
    }
    for (std::size_t mask = 0; mask < size; ++mask)
    {
        const auto rank = static_cast<std::uint8_t>(std::popcount(mask));
        ranks[mask] = rank;
        left[rank][mask] = a[mask];
        right[rank][mask] = b[mask];
        a[mask] = T{};
    }
    for (std::size_t rank = 0; rank <= dimensions; ++rank)
    {
        subset_zeta_transform(left[rank]);
        subset_zeta_transform(right[rank]);
    }

    // 次数の和dの寄与を逆変換後に得て、残った重複数d-|mask|だけ2倍する。
    const auto max_degree = CharacteristicTwo ? dimensions : 2 * dimensions;
    std::vector<T> combined(size);
    for (std::size_t remaining = max_degree + 1; remaining != 0; --remaining)
    {
        const auto degree = remaining - 1;
        const auto first = degree > dimensions ? degree - dimensions : 0U;
        const auto last = std::min(degree, dimensions);
        std::fill(combined.begin(), combined.end(), T{});
        for (std::size_t rank = first; rank <= last; ++rank)
        {
            const auto &l = left[rank];
            auto &r = right[degree - rank];
            for (std::size_t mask = 0; mask < size; ++mask)
            {
                auto product = l[mask];
                detail::bitwise_multiply_assign(product, r[mask]);
                combined[mask] += product;
            }
        }
        subset_mobius_transform(combined);
        for (std::size_t mask = 0; mask < size; ++mask)
        {
            if constexpr (CharacteristicTwo)
            {
                if (ranks[mask] == degree)
                {
                    a[mask] = combined[mask];
                }
            }
            else
            {
                if (ranks[mask] <= degree)
                {
                    a[mask] += a[mask];
                    a[mask] += combined[mask];
                }
            }
        }
    }
    superset_zeta_transform(a);
    return a;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
