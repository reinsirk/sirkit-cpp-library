#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/1901

// Library: Math/Xor_Convolution_Without_Division.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
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
    for (; half < n; half *= 2) {
        for (std::size_t base = 0; base < n; base += half * 2) {
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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
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


// Library: Math/Convolution_F2_64.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Convolution_F2_64.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/convolution_f2_64.md
// 参考（Cantor基底の加法FFT）: https://arxiv.org/abs/1802.03932
//
// API一覧
// multiply_f2_64(a,b): F_(2^64)の積。O(1)。和はXOR。
// convolution_f2_64(a,b): 同じ体の係数列の畳み込み。空列なら空列。結果長は2^21まで。
// L=bit_ceil(|a|+|b|-1) としてO(L log^2 L)時間、O(L)領域。短い方が16以下ならO(|a||b|)。
// uint64_tの第iビットはx^iの係数。既約多項式はx^64+x^4+x^3+x+1。
// 対応するGCC/Clang x86-64ではPCLMULQDQを使用。SIRKIT_DISABLE_SIMDで標準C++経路を指定できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__x86_64__))
#include <wmmintrin.h>
#endif


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CONVOLUTION_F2_64_DEFINED
#define SIRKIT_CONVOLUTION_F2_64_DEFINED
namespace detail
{
inline constexpr std::size_t kF2ConvolutionMax = std::size_t{1} << 21;

constexpr std::uint64_t F2Carryless32(std::uint32_t a, std::uint32_t b) noexcept
{
    constexpr auto kMask0 = std::uint64_t{0x1111111111111111ULL};
    constexpr auto kMask1 = kMask0 << 1, kMask2 = kMask0 << 2, kMask3 = kMask0 << 3;
    const auto a0 = a & kMask0, a1 = a & kMask1, a2 = a & kMask2, a3 = a & kMask3;
    const auto b0 = b & kMask0, b1 = b & kMask1, b2 = b & kMask2, b3 = b & kMask3;
    // 各積の同じ桁に重なる項は8個以下。4ビット間隔なら次の有効ビットに桁上がりしない。
    return ((a0 * b0 ^ a1 * b3 ^ a2 * b2 ^ a3 * b1) & kMask0) ^
           ((a0 * b1 ^ a1 * b0 ^ a2 * b3 ^ a3 * b2) & kMask1) ^
           ((a0 * b2 ^ a1 * b1 ^ a2 * b0 ^ a3 * b3) & kMask2) ^
           ((a0 * b3 ^ a1 * b2 ^ a2 * b1 ^ a3 * b0) & kMask3);
}

constexpr std::uint64_t F2ReduceProduct(std::uint64_t low, std::uint64_t high) noexcept
{
    const auto overflow = (high >> 60) ^ (high >> 61) ^ (high >> 63);
    const auto fold = high ^ overflow;
    return low ^ fold ^ (fold << 1) ^ (fold << 3) ^ (fold << 4);
}

constexpr std::uint64_t F2MultiplyPortable(std::uint64_t a, std::uint64_t b) noexcept
{
    const auto a0 = static_cast<std::uint32_t>(a), a1 = static_cast<std::uint32_t>(a >> 32);
    const auto b0 = static_cast<std::uint32_t>(b), b1 = static_cast<std::uint32_t>(b >> 32);
    const auto low = F2Carryless32(a0, b0), high = F2Carryless32(a1, b1);
    const auto middle = F2Carryless32(a0 ^ a1, b0 ^ b1) ^ low ^ high;
    return F2ReduceProduct(low ^ (middle << 32), high ^ (middle >> 32));
}

#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__x86_64__))
inline bool F2PclmulAvailable() noexcept
{
    return __builtin_cpu_supports("pclmul");
}

__attribute__((target("pclmul,sse2"))) inline std::uint64_t F2MultiplyPclmul(std::uint64_t a, std::uint64_t b) noexcept
{
    const auto product = _mm_clmulepi64_si128(_mm_set_epi64x(0, std::bit_cast<std::int64_t>(a)),
                                              _mm_set_epi64x(0, std::bit_cast<std::int64_t>(b)), 0);
    return F2ReduceProduct(static_cast<std::uint64_t>(_mm_cvtsi128_si64(product)),
                           static_cast<std::uint64_t>(_mm_cvtsi128_si64(_mm_srli_si128(product, 8))));
}
#endif

// x→x²+x のGF(2)線形系を下から順に解いた基底。β_0=1、β_i²+β_i=β_(i-1)。
inline constexpr std::array<std::uint64_t, 21> kF2CantorBasis{
    0x0000000000000001ULL,
    0x19c9369f278adc02ULL,
    0xa181e7d66f5ff794ULL,
    0x5db84357ce785d08ULL,
    0xb973d466f5c9d0caULL,
    0x521ac889831a075eULL,
    0x033ce8beddc8a656ULL,
    0xb5846c4e07b91010ULL,
    0x4087b8cbb37a32ecULL,
    0x00d0d3888c0ae17cULL,
    0xafd5ac70237f2222ULL,
    0xe3f5af99cc3aaaf8ULL,
    0x5a1db3b16a0b58b8ULL,
    0x09947c54fe7ee248ULL,
    0x0e8eaf0e0068f544ULL,
    0xa2a113500b4b4f5aULL,
    0xe96f9805d6ce0bb0ULL,
    0x53496f8b5c9edd4cULL,
    0xad325cb6f4ac2a9eULL,
    0x4a8dcf8bd7ede826ULL,
    0xa3e9c552b6434210ULL,
};

static_assert([]
              {
                  if (kF2CantorBasis.front() != 1)
                  {
                      return false;
                  }
                  for (std::size_t i = 1; i < kF2CantorBasis.size(); ++i)
                  {
                      const auto x = kF2CantorBasis[i];
                      if ((F2MultiplyPortable(x, x) ^ x) != kF2CantorBasis[i - 1])
                      {
                          return false;
                      }
                  }
                  return true;
              }());

inline void F2ApplyBasisTerm(std::span<std::uint64_t> values, std::size_t index, std::size_t half, unsigned depth) noexcept
{
    // S_depth(x)=(Frob+1)^depth(x) の下位項は、depthの真部分マスクに対応する。
    for (unsigned mask = depth; mask != 0;)
    {
        mask = (mask - 1) & depth;
        values[index - half + (std::size_t{1} << mask)] ^= values[index];
    }
}

template <bool Inverse>
void F2MonomialBasis(std::span<std::uint64_t> values) noexcept
{
    const auto size = values.size();
    for (auto half = Inverse ? 2U : size / 2; Inverse ? half < size : half > 1;)
    {
        const auto depth = static_cast<unsigned>(std::countr_zero(half));
        for (std::size_t base = 0; base < size; base += 2 * half)
        {
            if constexpr (Inverse)
            {
                for (auto i = base + half; i < base + 2 * half; ++i)
                {
                    F2ApplyBasisTerm(values, i, half, depth);
                }
            }
            else
            {
                for (auto i = base + 2 * half; i-- > base + half;)
                {
                    F2ApplyBasisTerm(values, i, half, depth);
                }
            }
        }
        half = Inverse ? 2 * half : half / 2;
    }
}

template <bool Inverse, auto Multiply>
void F2AdditiveTransform(std::span<std::uint64_t> values, std::span<const std::uint64_t> offsets) noexcept
{
    const auto size = values.size();
    for (auto half = Inverse ? 1U : size / 2; Inverse ? half < size : half != 0;)
    {
        for (std::size_t base = 0; base < size; base += 2 * half)
        {
            const auto scale = offsets[base / half];
            if (scale == 0)
            {
                for (std::size_t i = base; i < base + half; ++i)
                {
                    values[i + half] ^= values[i];
                }
                continue;
            }
            // 左の剰余はa+scale*b、右の剰余はa+(scale+1)*b。
            for (std::size_t i = base; i < base + half; ++i)
            {
                if constexpr (Inverse)
                {
                    values[i + half] ^= values[i];
                    values[i] ^= Multiply(scale, values[i + half]);
                }
                else
                {
                    values[i] ^= Multiply(scale, values[i + half]);
                    values[i + half] ^= values[i];
                }
            }
        }
        half = Inverse ? 2 * half : half / 2;
    }
}

template <auto Multiply>
std::vector<std::uint64_t> F2Convolution(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b,
                                         std::size_t result_size)
{
    if (std::min(a.size(), b.size()) <= 16)
    {
        std::vector<std::uint64_t> result(result_size);
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            for (std::size_t j = 0; j < b.size(); ++j)
            {
                result[i + j] ^= Multiply(a[i], b[j]);
            }
        }
        return result;
    }
    const auto length = std::bit_ceil(result_size);
    std::vector<std::uint64_t> left(length), right(length), offsets(length);
    std::copy(a.begin(), a.end(), left.begin());
    std::copy(b.begin(), b.end(), right.begin());
    for (std::size_t i = 1; i < length; ++i)
    {
        offsets[i] = offsets[i & (i - 1)] ^ kF2CantorBasis[std::countr_zero(i)];
    }
    F2MonomialBasis<false>(left);
    F2MonomialBasis<false>(right);
    F2AdditiveTransform<false, Multiply>(left, offsets);
    F2AdditiveTransform<false, Multiply>(right, offsets);
    for (std::size_t i = 0; i < length; ++i)
    {
        left[i] = Multiply(left[i], right[i]);
    }
    F2AdditiveTransform<true, Multiply>(left, offsets);
    F2MonomialBasis<true>(left);
    left.resize(result_size);
    return left;
}

inline std::uint64_t F2Multiply(std::uint64_t a, std::uint64_t b) noexcept
{
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__x86_64__))
    if (F2PclmulAvailable())
    {
        return F2MultiplyPclmul(a, b);
    }
#endif
    return F2MultiplyPortable(a, b);
}

inline std::vector<std::uint64_t> F2ConvolutionChecked(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    if (a.size() > kF2ConvolutionMax || b.size() - 1 > kF2ConvolutionMax - a.size())
    {
        throw std::length_error("convolution_f2_64: result length exceeds 2^21");
    }
    const auto size = a.size() + b.size() - 1;
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__x86_64__))
    if (F2PclmulAvailable())
    {
        return F2Convolution<F2MultiplyPclmul>(a, b, size);
    }
#endif
    return F2Convolution<F2MultiplyPortable>(a, b, size);
}
}

inline std::uint64_t multiply_f2_64(std::uint64_t a, std::uint64_t b) noexcept
{
    return detail::F2Multiply(a, b);
}

inline std::vector<std::uint64_t> convolution_f2_64(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
{
    return detail::F2ConvolutionChecked(a, b);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif


#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace
{
struct BinaryPolynomial
{
    std::uint64_t bits{};

    BinaryPolynomial &operator+=(BinaryPolynomial right)
    {
        bits ^= right.bits;
        return *this;
    }
    BinaryPolynomial &operator-=(BinaryPolynomial right)
    {
        return *this += right;
    }
    BinaryPolynomial &operator*=(BinaryPolynomial right)
    {
        // F_2[x] の積は次数62以下なので、既約多項式での剰余を取っても変化しない。
        bits = sirkit::multiply_f2_64(bits, right.bits);
        return *this;
    }
};
} // namespace

int main()
{
    unsigned dimensions{};
    verify_in >> dimensions;
    const auto length = std::size_t{1} << dimensions;
    std::vector<BinaryPolynomial> left(length), right(length);
    for (auto *input : {&left, &right})
    {
        for (auto &polynomial : *input)
        {
            for (unsigned bit = 0; bit < 32; ++bit)
            {
                std::uint64_t coefficient{};
                verify_in >> coefficient;
                polynomial.bits |= coefficient << bit;
            }
        }
    }
    auto product = sirkit::xor_convolution_without_division<true>(std::move(left), std::move(right));
    for (const auto polynomial : product)
    {
        for (unsigned bit = 0; bit < 63; ++bit)
        {
            verify_out << static_cast<char>('0' + ((polynomial.bits >> bit) & 1u));
            verify_out << (bit == 62 ? '\n' : ' ');
        }
    }
}
