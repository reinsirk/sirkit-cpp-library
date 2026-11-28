// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
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
#if defined(__GNUC__) && !defined(__clang__)
template <bool Subtract, class T>
[[gnu::always_inline]] inline void set_transform_eight(T* out, T* in) {
    if constexpr (Subtract) out[0] -= in[0]; else out[0] += in[0];
    if constexpr (Subtract) out[1] -= in[1]; else out[1] += in[1];
    if constexpr (Subtract) out[2] -= in[2]; else out[2] += in[2];
    if constexpr (Subtract) out[3] -= in[3]; else out[3] += in[3];
    if constexpr (Subtract) out[4] -= in[4]; else out[4] += in[4];
    if constexpr (Subtract) out[5] -= in[5]; else out[5] += in[5];
    if constexpr (Subtract) out[6] -= in[6]; else out[6] += in[6];
    if constexpr (Subtract) out[7] -= in[7]; else out[7] += in[7];
}
#endif
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
#if defined(__GNUC__) && !defined(__clang__)
    if constexpr (Subset && std::is_trivially_copyable_v<T> && sizeof(T) <= 8) {
        // 依存する段の順序を保って2段をまとめる。
        for (; half >= 8 && half < n / 2; half *= 4) {
            for (std::size_t base = 0; base < n; base += half * 4) {
                auto* a = f.data() + base;
                auto* b = a + half;
                auto* c = b + half;
                auto* d = c + half;
                for (std::size_t j = 0; j < half; j += 8) {
                    set_transform_eight<Subtract>(b+j,a+j);
                    set_transform_eight<Subtract>(d+j,c+j);
                    set_transform_eight<Subtract>(c+j,a+j);
                    set_transform_eight<Subtract>(d+j,b+j);
                }
            }
        }
    }
#endif
    for (; half < n; half *= 2) {
        for (std::size_t base = 0; base < n; base += half * 2) {
#if defined(__GNUC__) && !defined(__clang__)
            // 小さな単純型は8要素ずつ展開し、GCCの自動ベクトル化を促す。
            if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) <= 8) {
                auto* out = f.data() + base + (Subset ? half : 0);
                auto* in = f.data() + base + (Subset ? 0 : half);
                std::size_t j = 0;
                for (; half - j >= 8; j += 8) {
                    if constexpr (Subtract) out[j] -= in[j];
                    else out[j] += in[j];
                    if constexpr (Subtract) out[j+1] -= in[j+1];
                    else out[j+1] += in[j+1];
                    if constexpr (Subtract) out[j+2] -= in[j+2];
                    else out[j+2] += in[j+2];
                    if constexpr (Subtract) out[j+3] -= in[j+3];
                    else out[j+3] += in[j+3];
                    if constexpr (Subtract) out[j+4] -= in[j+4];
                    else out[j+4] += in[j+4];
                    if constexpr (Subtract) out[j+5] -= in[j+5];
                    else out[j+5] += in[j+5];
                    if constexpr (Subtract) out[j+6] -= in[j+6];
                    else out[j+6] += in[j+6];
                    if constexpr (Subtract) out[j+7] -= in[j+7];
                    else out[j+7] += in[j+7];
                }
                for (; j < half; ++j) {
                    if constexpr (Subtract) out[j] -= in[j];
                    else out[j] += in[j];
                }
            } else
#endif
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
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <bool Subset, bool Subtract, std::size_t Half, class T>
[[gnu::always_inline]] inline void set_transform_small_stage(T* f, std::size_t n) {
    for (std::size_t base=0; base<n; base+=Half*2) {
        T* out=f+base+(Subset?Half:0);
        const T* in=f+base+(Subset?0:Half);
        for (std::size_t j=0;j<Half;++j) {
            if constexpr(Subtract) out[j]-=in[j];else out[j]+=in[j];
        }
    }
}
template <bool Subset, bool Subtract, std::size_t Size, class T>
[[gnu::always_inline]] inline void set_transform_small(T* f) {
    set_transform_small_stage<Subset,Subtract,1>(f,Size);
    set_transform_small_stage<Subset,Subtract,2>(f,Size);
    set_transform_small_stage<Subset,Subtract,4>(f,Size);
    set_transform_small_stage<Subset,Subtract,8>(f,Size);
    if constexpr (Size == 32) set_transform_small_stage<Subset,Subtract,16>(f,Size);
}
template <bool Subset, bool Subtract, unsigned Bit, class U, class V, std::size_t... I>
[[gnu::target("avx2"),gnu::always_inline]] inline V set_transform_stage(V v, std::index_sequence<I...>) {
    const V swapped = __builtin_shufflevector(v, v, (I ^ Bit)...);
    const V mask{static_cast<U>((static_cast<bool>(I & Bit) == Subset) ? static_cast<U>(~U{0}) : U{0})...};
    if constexpr (Subtract) v -= swapped & mask;
    else v += swapped & mask;
    if constexpr (Bit * 2 < sizeof(V) / sizeof(U))
        return set_transform_stage<Subset,Subtract,Bit*2,U>(v, std::index_sequence<I...>{});
    return v;
}
template <bool Subset, bool Subtract, class T>
[[gnu::target("avx2")]] void set_transform_simd(T* f, std::size_t n) {
    using U = std::make_unsigned_t<T>;
    typedef U V __attribute__((vector_size(32)));
    constexpr std::size_t width = sizeof(V) / sizeof(T);
    for (std::size_t base = 0; base < n; base += width) {
        V v;
        std::memcpy(&v, f + base, sizeof(v));
        v = set_transform_stage<Subset,Subtract,1,U>(v, std::make_index_sequence<width>{});
        std::memcpy(f + base, &v, sizeof(v));
    }
    // 3段ずつレジスタ内で伝播し、未変更の端のベクトルは書き戻さない。
    std::size_t half = width;
    for (; half < n / 4; half *= 8) {
        for (std::size_t base = 0; base < n; base += half * 8) {
            for (std::size_t j = 0; j < half; j += width) {
                V v0; std::memcpy(&v0, f + base + half * 0 + j, sizeof(V));
                V v1; std::memcpy(&v1, f + base + half * 1 + j, sizeof(V));
                V v2; std::memcpy(&v2, f + base + half * 2 + j, sizeof(V));
                V v3; std::memcpy(&v3, f + base + half * 3 + j, sizeof(V));
                V v4; std::memcpy(&v4, f + base + half * 4 + j, sizeof(V));
                V v5; std::memcpy(&v5, f + base + half * 5 + j, sizeof(V));
                V v6; std::memcpy(&v6, f + base + half * 6 + j, sizeof(V));
                V v7; std::memcpy(&v7, f + base + half * 7 + j, sizeof(V));
                if constexpr (Subset) {
                    if constexpr (Subtract) v1 -= v0; else v1 += v0;
                    if constexpr (Subtract) v3 -= v2; else v3 += v2;
                    if constexpr (Subtract) v5 -= v4; else v5 += v4;
                    if constexpr (Subtract) v7 -= v6; else v7 += v6;
                    if constexpr (Subtract) v2 -= v0; else v2 += v0;
                    if constexpr (Subtract) v3 -= v1; else v3 += v1;
                    if constexpr (Subtract) v6 -= v4; else v6 += v4;
                    if constexpr (Subtract) v7 -= v5; else v7 += v5;
                    if constexpr (Subtract) v4 -= v0; else v4 += v0;
                    if constexpr (Subtract) v5 -= v1; else v5 += v1;
                    if constexpr (Subtract) v6 -= v2; else v6 += v2;
                    if constexpr (Subtract) v7 -= v3; else v7 += v3;
                }
                if constexpr (!Subset) {
                    if constexpr (Subtract) v0 -= v1; else v0 += v1;
                    if constexpr (Subtract) v2 -= v3; else v2 += v3;
                    if constexpr (Subtract) v4 -= v5; else v4 += v5;
                    if constexpr (Subtract) v6 -= v7; else v6 += v7;
                    if constexpr (Subtract) v0 -= v2; else v0 += v2;
                    if constexpr (Subtract) v1 -= v3; else v1 += v3;
                    if constexpr (Subtract) v4 -= v6; else v4 += v6;
                    if constexpr (Subtract) v5 -= v7; else v5 += v7;
                    if constexpr (Subtract) v0 -= v4; else v0 += v4;
                    if constexpr (Subtract) v1 -= v5; else v1 += v5;
                    if constexpr (Subtract) v2 -= v6; else v2 += v6;
                    if constexpr (Subtract) v3 -= v7; else v3 += v7;
                }
                if constexpr (!Subset) std::memcpy(f + base + half * 0 + j, &v0, sizeof(V));
                std::memcpy(f + base + half * 1 + j, &v1, sizeof(V));
                std::memcpy(f + base + half * 2 + j, &v2, sizeof(V));
                std::memcpy(f + base + half * 3 + j, &v3, sizeof(V));
                std::memcpy(f + base + half * 4 + j, &v4, sizeof(V));
                std::memcpy(f + base + half * 5 + j, &v5, sizeof(V));
                std::memcpy(f + base + half * 6 + j, &v6, sizeof(V));
                if constexpr (Subset) std::memcpy(f + base + half * 7 + j, &v7, sizeof(V));
            }
        }
    }
    for (; half < n / 2; half *= 4) {
        for (std::size_t base = 0; base < n; base += half * 4) {
            for (std::size_t j = 0; j < half; j += width) {
                V v0; std::memcpy(&v0, f + base + half * 0 + j, sizeof(V));
                V v1; std::memcpy(&v1, f + base + half * 1 + j, sizeof(V));
                V v2; std::memcpy(&v2, f + base + half * 2 + j, sizeof(V));
                V v3; std::memcpy(&v3, f + base + half * 3 + j, sizeof(V));
                if constexpr (Subset) {
                    if constexpr (Subtract) v1 -= v0; else v1 += v0;
                    if constexpr (Subtract) v3 -= v2; else v3 += v2;
                    if constexpr (Subtract) v2 -= v0; else v2 += v0;
                    if constexpr (Subtract) v3 -= v1; else v3 += v1;
                }
                if constexpr (!Subset) {
                    if constexpr (Subtract) v0 -= v1; else v0 += v1;
                    if constexpr (Subtract) v2 -= v3; else v2 += v3;
                    if constexpr (Subtract) v0 -= v2; else v0 += v2;
                    if constexpr (Subtract) v1 -= v3; else v1 += v3;
                }
                if constexpr (!Subset) std::memcpy(f + base + half * 0 + j, &v0, sizeof(V));
                std::memcpy(f + base + half * 1 + j, &v1, sizeof(V));
                std::memcpy(f + base + half * 2 + j, &v2, sizeof(V));
                if constexpr (Subset) std::memcpy(f + base + half * 3 + j, &v3, sizeof(V));
            }
        }
    }
    for (; half < n; half *= 2) {
        for (std::size_t base = 0; base < n; base += half * 2) {
            T* out = f + base + (Subset ? half : 0);
            const T* in = f + base + (Subset ? 0 : half);
            for (std::size_t j = 0; j < half; j += width) {
                V a, b;
                std::memcpy(&a, out + j, sizeof(a));
                std::memcpy(&b, in + j, sizeof(b));
                if constexpr (Subtract) a -= b;
                else a += b;
                std::memcpy(out + j, &a, sizeof(a));
            }
        }
    }
}
}
#endif
#endif


#ifndef SIRKIT_SUPERSET_ZETA_TRANSFORM_DEFINED
#define SIRKIT_SUPERSET_ZETA_TRANSFORM_DEFINED
template <class T> void superset_zeta_transform(std::vector<T>& f) {
#ifndef SKIP_RUNTIME_CHECKS
    if (!f.empty() && !std::has_single_bit(f.size()))
        throw std::invalid_argument("superset_zeta_transform: length must be a power of two");
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T,bool> && sizeof(T)<=8) {
        if (f.size()>=8) {
        if(f.size()==8){f[0]+=f[1];f[2]+=f[3];f[4]+=f[5];f[6]+=f[7];f[0]+=f[2];f[1]+=f[3];f[4]+=f[6];f[5]+=f[7];f[0]+=f[4];f[1]+=f[5];f[2]+=f[6];f[3]+=f[7];return;}
        if (f.size() < 64) {
            const auto size = f.size();
#ifdef SKIP_RUNTIME_CHECKS
            // 固定サイズの処理は対応する長さでだけ呼ぶ。
            if (size != 16 && size != 32) return;
#endif
            if (size == 16) /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<false,false,16>(f.data());
            else /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<false,false,32>(f.data());
            return;
        }
        if (__builtin_cpu_supports("avx2")) {
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_simd<false,false>(f.data(), f.size());
            return;
        }
        }
    }
#endif
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_scalar<false, false>(f);
}
#endif

#ifndef SIRKIT_SUPERSET_MOBIUS_TRANSFORM_DEFINED
#define SIRKIT_SUPERSET_MOBIUS_TRANSFORM_DEFINED
template <class T> void superset_mobius_transform(std::vector<T>& f) {
#ifndef SKIP_RUNTIME_CHECKS
    if (!f.empty() && !std::has_single_bit(f.size()))
        throw std::invalid_argument("superset_mobius_transform: length must be a power of two");
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T,bool> && sizeof(T)<=8) {
        if (f.size()>=8) {
        if(f.size()==8){f[0]-=f[1];f[2]-=f[3];f[4]-=f[5];f[6]-=f[7];f[0]-=f[2];f[1]-=f[3];f[4]-=f[6];f[5]-=f[7];f[0]-=f[4];f[1]-=f[5];f[2]-=f[6];f[3]-=f[7];return;}
        if (f.size() < 64) {
            const auto size = f.size();
#ifdef SKIP_RUNTIME_CHECKS
            // 固定サイズの処理は対応する長さでだけ呼ぶ。
            if (size != 16 && size != 32) return;
#endif
            if (size == 16) /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<false,true,16>(f.data());
            else /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<false,true,32>(f.data());
            return;
        }
        if (__builtin_cpu_supports("avx2")) {
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_simd<false,true>(f.data(), f.size());
            return;
        }
        }
    }
#endif
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_scalar<false, true>(f);
}
#endif

#ifndef SIRKIT_SUBSET_ZETA_TRANSFORM_DEFINED
#define SIRKIT_SUBSET_ZETA_TRANSFORM_DEFINED
template <class T> void subset_zeta_transform(std::vector<T>& f) {
#ifndef SKIP_RUNTIME_CHECKS
    if (!f.empty() && !std::has_single_bit(f.size()))
        throw std::invalid_argument("subset_zeta_transform: length must be a power of two");
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T,bool> && sizeof(T)<=8) {
        if (f.size()>=8) {
        if(f.size()==8){f[1]+=f[0];f[3]+=f[2];f[5]+=f[4];f[7]+=f[6];f[2]+=f[0];f[3]+=f[1];f[6]+=f[4];f[7]+=f[5];f[4]+=f[0];f[5]+=f[1];f[6]+=f[2];f[7]+=f[3];return;}
        if (f.size() < 64) {
            const auto size = f.size();
#ifdef SKIP_RUNTIME_CHECKS
            // 固定サイズの処理は対応する長さでだけ呼ぶ。
            if (size != 16 && size != 32) return;
#endif
            if (size == 16) /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<true,false,16>(f.data());
            else /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<true,false,32>(f.data());
            return;
        }
        if (__builtin_cpu_supports("avx2")) {
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_simd<true,false>(f.data(), f.size());
            return;
        }
        }
    }
#endif
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_scalar<true, false>(f);
}
#endif

#ifndef SIRKIT_SUBSET_MOBIUS_TRANSFORM_DEFINED
#define SIRKIT_SUBSET_MOBIUS_TRANSFORM_DEFINED
template <class T> void subset_mobius_transform(std::vector<T>& f) {
#ifndef SKIP_RUNTIME_CHECKS
    if (!f.empty() && !std::has_single_bit(f.size()))
        throw std::invalid_argument("subset_mobius_transform: length must be a power of two");
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T,bool> && sizeof(T)<=8) {
        if (f.size()>=8) {
        if(f.size()==8){f[1]-=f[0];f[3]-=f[2];f[5]-=f[4];f[7]-=f[6];f[2]-=f[0];f[3]-=f[1];f[6]-=f[4];f[7]-=f[5];f[4]-=f[0];f[5]-=f[1];f[6]-=f[2];f[7]-=f[3];return;}
        if (f.size() < 64) {
            const auto size = f.size();
#ifdef SKIP_RUNTIME_CHECKS
            // 固定サイズの処理は対応する長さでだけ呼ぶ。
            if (size != 16 && size != 32) return;
#endif
            if (size == 16) /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<true,true,16>(f.data());
            else /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_small<true,true,32>(f.data());
            return;
        }
        if (__builtin_cpu_supports("avx2")) {
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::set_transform_simd<true,true>(f.data(), f.size());
            return;
        }
        }
    }
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
    if (f.size() != g.size()) throw std::invalid_argument("bitwise_and_convolution: different lengths");
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
    if (f.size() != g.size()) throw std::invalid_argument("bitwise_or_convolution: different lengths");
#endif
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
