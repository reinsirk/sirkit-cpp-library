#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/polynomial_interpolation

// Library: Math/Polynomial.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Polynomial.cpp
// 参考（積木による多点評価・補間の数式とVerify問題。所有構造・評価再帰は独自実装）:
// https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html
// https://judge.yosupo.jp/problem/polynomial_interpolation
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial.md
//
// API一覧
// ProductTree(): 空の点列。ProductTree(points)/assign(points): 整数vectorを998244353へ正規化し積木を構築。
// 構築O(1+M(n)log(n+1))、保持O(n log(n+1))、M(n)=O(n log(n+1))。n<=max_points()=2^22-1。
// size()/empty()/points(): 点数/空か/正規化した点列のconst参照。O(1)。コピーは独立、別オブジェクトへの移動元は空。assign/コピー代入は失敗時に元を保つ。
// swap(other): 点列と積木をO(1)で交換する。
// polynomial(): 積Π(x-point)のFormalPowerSeriesをコピーして返す。空点列では{1}。O(n)。
// evaluate(poly): FormalPowerSeriesまたは整数係数vectorを各点で評価しvector<uint32_t>を返す。重複点も可。
// evaluateは係数数d<=2^22、O(1+M(max(n,d))log(n+2)+n+d)、追加O(n+d)。固定点列で積木を再利用する。
// interpolate(values): 各点の整数値vectorから次数n未満のFormalPowerSeriesを返す。出力はn係数（末尾0を保持）。
// 補間には法で相異なる点が必要。重複は数式上の分母0として検査省略時もdomain_error。O(1+M(n)log(n+1))、追加O(n log(n+1))。
// MultipointEvaluate(poly,points)/PolynomialInterpolate(points,values): 一時積木を使う簡易版。
// TaylorShift(poly,shift): f(x+shift)のFormalPowerSeries。係数数は入力と同じ。O(1+M(d))時間・O(d)追加、d<=2^22。
// TaylorShiftはFormalPowerSeries/整数vectorに対応。全て法998244353、Mint不要。容量/数学的0判定は検査省略時も維持。
//
// [同梱FPS/NTT]
// FormalPowerSeries: 法998244353、Mint不要の形式的冪級数/多項式。係数はuint32_tの独立所有vector。
// ()/size_t n/vector<整数>/initializer_list<int64_t>: 空/n個の0/法へ正規化した係数で生成。コピーは独立。
// modulus()/max_precision(): 法/精度操作の上限をO(1)で返す。
// size()/empty()/coefficients(): 係数数/空か/係数vectorのconst参照。operator[](i)/set(i,value): 既存添字の読取/正規化した代入。
// get(i): 範囲外は0。resize(n): 末尾を0で拡張/切詰め。prefix(n): n係数へ0拡張/切詰めしたコピー。
// trim(): 末尾の0を除く。空vectorは零多項式。長さを含めて等値比較するため、精度の異なる0列は区別する。
// +,-,*,/,%, +=,-=,*=,/=,%=: 係数和/差/積/多項式商/剰余。和・差は長い方の長さ、積は空を含めば空。
// scale(value): 全係数を整数倍。derivative()/integral(): 微分/積分（積分定数0）。積分は係数数<998244353が必要。
// inverse(n)/log(n)/exp(n): x^n未満で逆数/形式log/形式exp。inverseは定数項!=0、logは=1、expは=0。
// pow(k,n): 非負uint64_t乗をx^nで切る。0^0=1、先頭0と大きいkを扱う。sqrt(n): 平方根かnullopt。
// sqrtは先頭非0次数が偶数かつ定数部分が平方剰余なら存在。根の先頭非0係数は2候補の小さい方、未確定末尾は0。
// inverse/log/exp/pow/sqrtの引数n省略は現在のsize()。n=0は常に空、精度nは2^22以下。
// divmod(divisor): 末尾0を除いた商/剰余のpair。0除算は検査省略時もdomain_error。
// 線形操作はO(n)。精度操作はM(n)=O(n log n)としてO(M(n))、冪乗は加えてO(log k)。作業領域O(n)。
// 積と商剰余のNTT長は2^23以下。短い積/除数では直接計算。長さ・数学的0判定は検査省略時も維持。
// clear_cache(): このスレッドのNTT根/整数逆数cacheを解放。cacheは同一スレッドで共有し、係数は共有しない。
//
// [同梱NTT]
// NTT<Mod=998244353,Root=3>: Mint不要の素数法NTT。3<=Mod<2^31、Rootは最大2冪位数の根を生成する値。
// NTT(): 動的キャッシュなしで生成。コピーはキャッシュを独立に所有し、時間・領域は保持数に比例。
// modulus()/max_length(): 法/対応する最大変換長をO(1)で返す。
// prepare(length): 必要な根を保持。lengthは0または2冪、上限は2^v2(Mod-1)。新規分O(length)時間・保持O(length)。
// transform(a,inverse=false): 正順の係数と正順の周波数を相互変換しvoidを返す。aはuint32_tのvector。
// transformは空または対応する2冪長、各値0..Mod-1が前提。O(n log n)時間、根以外の追加領域O(1)。
// convolution(a,b): 整数vectorを法Modへ正規化し畳み込みを返す。負数/uint64_t最大値も可。空を含めば{}。
// convolution_normalized(a,b): uint32_tの正規化済みvectorを値で受け取り再利用。std::moveで入力領域を譲渡可。
// square(a): 同じ列の畳み込み。大規模では順変換を1回にする。返却値はvector<uint32_t>。
// 畳み込みは短い方32要素以下ならO(nm)、それ以外はL=bit_ceil(n+m-1)としてO(L log L)、追加O(L)。
// NTT経路のLとtransform長は上限以下。長さ・容量の上限検査はSKIP_RUNTIME_CHECKSでも維持。
// ConvolutionMod<Mod,Root>(a,b): 一時NTTを使う簡易版。同じ長さを繰り返すならNTTオブジェクトを再利用する。
// roots_capacity()/clear_cache(): 動的な根のキャッシュの要素数を返す/解放。通常操作は入力不変、transformは例外までの変更が残り得る。
// Mod<2^30では標準C++のMontgomery演算を使用。GCC/Clangの対応x86 CPUではAVX2で8係数ずつ処理する。
// SIRKIT_DISABLE_SIMDで追加の専用命令とCPU判定を除外できる。代替経路も同じAPI・結果を保つ。

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
#include <immintrin.h>
#endif

#ifndef SIRKIT_NTT_DETAIL_DEFINED
#define SIRKIT_NTT_DETAIL_DEFINED
namespace reinsirk::detail {
constexpr bool ntt_prime(std::uint32_t n) noexcept {
    if (n < 2 || n % 2 == 0) return n == 2;
    for (std::uint32_t d = 3; d <= n / d; d += 2) if (n % d == 0) return false;
    return true;
}
constexpr std::uint32_t ntt_power(std::uint32_t a, std::uint32_t exponent, std::uint32_t mod) noexcept {
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1) {
        if (exponent & 1) result = static_cast<std::uint32_t>(std::uint64_t{result} * a % mod);
        a = static_cast<std::uint32_t>(std::uint64_t{a} * a % mod);
    }
    return result;
}
}
#endif

#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
#ifndef SIRKIT_NTT_SIMD_DETAIL_DEFINED
#define SIRKIT_NTT_SIMD_DETAIL_DEFINED
namespace reinsirk::detail {
inline bool ntt_avx2_available() noexcept {
    static const bool available=__builtin_cpu_supports("avx2");
    return available;
}
}
#endif
#endif

#ifndef SIRKIT_NTT_DEFINED
#define SIRKIT_NTT_DEFINED
template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3>
class NTT {
    static_assert(Mod >= 3 && Mod < (std::uint32_t{1} << 31) && reinsirk::detail::ntt_prime(Mod),
                  "NTT modulus must be an odd prime below 2^31");
    static_assert(Root > 0 && Root < Mod, "NTT root outside modulus");
    static constexpr int rank_ = std::countr_zero(Mod - 1);
    static constexpr std::uint32_t highest_root_ = reinsirk::detail::ntt_power(Root, (Mod - 1) >> rank_, Mod);
    static_assert(reinsirk::detail::ntt_power(highest_root_, std::uint32_t{1} << (rank_ - 1), Mod) == Mod - 1,
                  "NTT root has insufficient power-of-two order");
    std::vector<std::uint32_t> roots_;

    static constexpr std::uint32_t multiply(std::uint32_t a, std::uint32_t b) noexcept {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    }
    static constexpr std::uint32_t add(std::uint32_t a, std::uint32_t b) noexcept {
        const auto sum = a + b;
        return sum >= Mod ? sum - Mod : sum;
    }
    static constexpr std::uint32_t subtract(std::uint32_t a, std::uint32_t b) noexcept {
        return a >= b ? a - b : a + Mod - b;
    }
    static constexpr bool montgomery_ = Mod < (std::uint32_t{1} << 30);
    static constexpr std::uint32_t negative_inverse_ = [] {
        std::uint32_t x=Mod;for(int i=0;i<5;++i)x*=2u-Mod*x;return 0u-x;
    }();
    static constexpr std::uint32_t montgomery_one_ = static_cast<std::uint32_t>((std::uint64_t{1} << 32)%Mod);
    static constexpr std::uint32_t r_squared_ = static_cast<std::uint32_t>(std::uint64_t{montgomery_one_}*montgomery_one_%Mod);
    static constexpr std::array<std::uint32_t, rank_ + 1> stage_roots_ = [] {
        std::array<std::uint32_t, rank_ + 1> table{};
        table[rank_] = highest_root_;
        for (int k = rank_; k >= 2; --k)
            table[k - 1] = static_cast<std::uint32_t>(std::uint64_t{table[k]} * table[k] % Mod);
        if constexpr (montgomery_) {
            for (auto& value : table) {
                const auto product = std::uint64_t{value} * r_squared_;
                const auto q = static_cast<std::uint32_t>(std::uint64_t{static_cast<std::uint32_t>(product)} * negative_inverse_);
                value = static_cast<std::uint32_t>((product + std::uint64_t{q} * Mod) >> 32);
            }
        }
        return table;
    }();
    static constexpr std::uint32_t working_multiply(std::uint32_t a,std::uint32_t b) noexcept {
        if constexpr(montgomery_) {
            // 内部値は2Mod未満。t+qMod<2^63で、還元後も2Mod未満。
            const auto product=std::uint64_t{a}*b;
            const auto q=static_cast<std::uint32_t>(product)*negative_inverse_;
            const auto value=static_cast<std::uint32_t>((product+std::uint64_t{q}*Mod)>>32);
            return value;
        } else return multiply(a,b);
    }
    static constexpr std::uint32_t working_normalize(std::uint32_t value) noexcept {
        if constexpr(montgomery_)return value>=Mod?value-Mod:value;
        else return value;
    }
    static constexpr std::uint32_t working_add(std::uint32_t a,std::uint32_t b) noexcept {
        if constexpr(montgomery_){const auto value=a+b;return value>=2*Mod?value-2*Mod:value;}
        else return add(a,b);
    }
    static constexpr std::uint32_t working_subtract(std::uint32_t a,std::uint32_t b) noexcept {
        if constexpr(montgomery_)return a>=b?a-b:a+2*Mod-b;
        else return subtract(a,b);
    }
    static constexpr std::uint32_t encode(std::uint32_t value) noexcept {
        if constexpr(montgomery_)return working_multiply(value,r_squared_);
        else return value;
    }
    template<bool Encode,bool Check>
    static void bit_reverse_convert(std::vector<std::uint32_t>& a) {
        const auto convert=[](std::uint32_t value) {
#ifndef SKIP_RUNTIME_CHECKS
            if constexpr(Check)if(value>=Mod)throw std::invalid_argument("NTT: coefficient not normalized");
#endif
            if constexpr(Encode)return encode(value);
            else return working_normalize(working_multiply(value,1));
        };
        a[0]=convert(a[0]);
        for(std::size_t i=1,reverse=0;i<a.size();++i) {
            std::size_t bit=a.size()>>1;
            for(;reverse&bit;bit>>=1)reverse^=bit;
            reverse^=bit;
            if(i<reverse){const auto lo=convert(a[i]),hi=convert(a[reverse]);a[i]=hi;a[reverse]=lo;}
            else if(i==reverse)a[i]=convert(a[i]);
        }
    }
    template <std::integral T>
    static std::uint32_t normalize(T value) noexcept {
        static_assert(sizeof(T) <= sizeof(std::uint64_t));
        if constexpr (std::is_signed_v<T>) {
            const auto remainder = static_cast<std::int64_t>(value) % static_cast<std::int64_t>(Mod);
            return static_cast<std::uint32_t>(remainder < 0 ? remainder + Mod : remainder);
        } else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value) % Mod);
    }
    static std::size_t result_size(std::size_t a, std::size_t b) {
        if (a == 0 || b == 0) return 0;
        if (a - 1 > std::numeric_limits<std::size_t>::max() - b)
            throw std::length_error("NTT: convolution size overflow");
        return (a - 1) + b;
    }
    static std::size_t transform_size(std::size_t output) {
        if (output > max_length()) throw std::length_error("NTT: transform too long");
        return std::bit_ceil(output);
    }
    static void bit_reverse(std::vector<std::uint32_t>& a) noexcept {
        const std::size_t n = a.size();
        for (std::size_t i = 1, reverse = 0; i < n; ++i) {
            std::size_t bit = n >> 1;
            for (; reverse & bit; bit >>= 1) reverse ^= bit;
            reverse ^= bit;
            if (i < reverse) std::swap(a[i], a[reverse]);
        }
    }

#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
    static bool use_avx2(std::size_t n) noexcept {
        if constexpr(montgomery_)return n>=16 && reinsirk::detail::ntt_avx2_available();
        else return false;
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_reduce(__m256i value) noexcept {
        const auto mod=_mm256_set1_epi32(static_cast<int>(Mod));
        return _mm256_min_epu32(value,_mm256_sub_epi32(value,mod));
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_add(__m256i a,__m256i b) noexcept {
        const auto sum=_mm256_add_epi32(a,b);return _mm256_min_epu32(sum,_mm256_sub_epi32(sum,_mm256_set1_epi32(static_cast<int>(2*Mod))));
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_subtract(__m256i a,__m256i b) noexcept {
        const auto difference=_mm256_sub_epi32(a,b);return _mm256_min_epu32(difference,_mm256_add_epi32(difference,_mm256_set1_epi32(static_cast<int>(2*Mod))));
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_multiply(__m256i a,__m256i b) noexcept {
        const auto mod=_mm256_set1_epi32(static_cast<int>(Mod));
        const auto inverse=_mm256_set1_epi32(static_cast<int>(negative_inverse_));
        const auto even=_mm256_mul_epu32(a,b);
        const auto odd=_mm256_mul_epu32(_mm256_srli_epi64(a,32),_mm256_srli_epi64(b,32));
        const auto q_even=_mm256_mul_epu32(even,inverse),q_odd=_mm256_mul_epu32(odd,inverse);
        const auto reduced_even=_mm256_srli_epi64(_mm256_add_epi64(even,_mm256_mul_epu32(q_even,mod)),32);
        const auto reduced_odd=_mm256_add_epi64(odd,_mm256_mul_epu32(q_odd,mod));
        return _mm256_blend_epi32(reduced_even,reduced_odd,0xaa);
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    // 4点段の偶数レーンの根は1。奇数レーンだけ虚数単位の根を掛ける。
    static inline __m256i vector_multiply_quarter_root(__m256i value, __m256i imaginary) noexcept {
        const auto odd = _mm256_mul_epu32(_mm256_srli_epi64(value, 32), imaginary);
        const auto q = _mm256_mul_epu32(odd, _mm256_set1_epi32(static_cast<int>(negative_inverse_)));
        const auto reduced = _mm256_add_epi64(odd, _mm256_mul_epu32(q, _mm256_set1_epi32(static_cast<int>(Mod))));
        return _mm256_blend_epi32(value, reduced, 0xaa);
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_multiply_prepared(__m256i a, __m256i b,
                                                  __m256i b_times_negative_inverse) noexcept {
        const auto mod = _mm256_set1_epi32(static_cast<int>(Mod));
        const auto even = _mm256_mul_epu32(a, b);
        const auto odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
        const auto q_even = _mm256_mul_epu32(a, b_times_negative_inverse);
        const auto q_odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b_times_negative_inverse, 32));
        const auto reduced_even = _mm256_srli_epi64(_mm256_add_epi64(even, _mm256_mul_epu32(q_even, mod)), 32);
        const auto reduced_odd = _mm256_add_epi64(odd, _mm256_mul_epu32(q_odd, mod));
        return _mm256_blend_epi32(reduced_even, reduced_odd, 0xaa);
    }
#ifndef SKIP_RUNTIME_CHECKS
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline void vector_check(__m256i value) {
        const auto maximum=_mm256_set1_epi32(static_cast<int>(Mod-1));
        if(_mm256_movemask_epi8(_mm256_cmpeq_epi32(value,_mm256_min_epu32(value,maximum)))!=-1)
            throw std::invalid_argument("NTT: coefficient not normalized");
    }
#endif
    [[gnu::target("avx2")]]
    static void pointwise_avx2(std::vector<std::uint32_t>& a,const std::vector<std::uint32_t>& b) noexcept {
        for(std::size_t i=0;i<a.size();i+=8) {
            const auto x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+i));
            const auto y=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b.data()+i));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+i),vector_multiply(x,y));
        }
    }

    [[gnu::target("avx2")]]
    void prepare_avx2(std::size_t previous, std::size_t length) {
        for (std::size_t half = previous; half < length; half <<= 1) {
            const auto step = stage_roots_[std::countr_zero(half) + 1];
            const auto scale = _mm256_set1_epi32(static_cast<int>(step));
            std::size_t j = 0;
            for (; j + 8 <= half / 2; j += 8) {
                const auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data() + half / 2 + j));
                const auto product = vector_multiply(value, scale);
                // 偶数べきと奇数べきを交互に並べる。
                const auto low = _mm256_unpacklo_epi32(value, product);
                const auto high = _mm256_unpackhi_epi32(value, product);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(roots_.data() + half + 2 * j), _mm256_permute2x128_si256(low, high, 0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(roots_.data() + half + 2 * j + 8), _mm256_permute2x128_si256(low, high, 0x31));
            }
            for (; j < half / 2; ++j) {
                const auto value = roots_[half / 2 + j];
                roots_[half + 2 * j] = value;
                roots_[half + 2 * j + 1] = working_multiply(value, step);
            }
        }
    }
    template<bool Check>
    [[gnu::target("avx2")]]
    static std::vector<std::uint32_t> naive_avx2(std::vector<std::uint32_t>& a,std::vector<std::uint32_t>& b,std::size_t output) {
        if(a.size()>b.size())a.swap(b);
        std::vector<std::uint32_t> result(output);
        for(std::size_t i=0;i<a.size();++i) {
#ifndef SKIP_RUNTIME_CHECKS
            if constexpr(Check)if(a[i]>=Mod)throw std::invalid_argument("NTT: coefficient not normalized");
#endif
            const auto encoded_coefficient = encode(a[i]);
            const auto coefficient=_mm256_set1_epi32(static_cast<int>(encoded_coefficient));
            const auto coefficient_times_inverse = static_cast<std::uint32_t>(std::uint64_t{encoded_coefficient} * negative_inverse_);
            const auto qcoefficient = _mm256_set1_epi32(static_cast<int>(coefficient_times_inverse));
            std::size_t j=0;
            for(;j+8<=b.size();j+=8) {
                const auto input=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b.data()+j));
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr(Check)if(i==0)vector_check(input);
#endif
                const auto old=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(result.data()+i+j));
                const auto product=vector_reduce(vector_multiply_prepared(input,coefficient,qcoefficient));
                const auto sum=vector_reduce(_mm256_add_epi32(old,product));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(result.data()+i+j),sum);
            }
            for(;j<b.size();++j) {
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr(Check)if(i==0&&b[j]>=Mod)throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                result[i+j]=add(result[i+j],multiply(a[i],b[j]));
            }
        }
        return result;
    }
    template<bool Check, bool SkipFirst = false>
    [[gnu::target("avx2")]]
    void forward_avx2(std::vector<std::uint32_t>& a, [[maybe_unused]] std::size_t remaining_length=0) const {
        const auto n=a.size();

        std::size_t length=SkipFirst ? remaining_length : n;
        for(;length>=32;length>>=2) {
            const auto quarter=length/4;
            for(std::size_t block=0;block<n;block+=length)for(std::size_t j=0;j<quarter;j+=8) {
                auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                auto x1=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+quarter));
                auto x2=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+2*quarter));
                auto x3=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+3*quarter));
                if constexpr (!SkipFirst) if(length==n) {
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr(Check){vector_check(x0);vector_check(x1);vector_check(x2);vector_check(x3);}
#endif
                    const auto r2=_mm256_set1_epi32(static_cast<int>(r_squared_));
                    x0=vector_multiply(x0,r2);x1=vector_multiply(x1,r2);x2=vector_multiply(x2,r2);x3=vector_multiply(x3,r2);
                }
                const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
                const auto second=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+3*quarter+j));
                const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
                const auto v0=vector_add(x0,x2),v1=vector_add(x1,x3);
                const auto v2=vector_multiply(vector_subtract(x0,x2),first),v3=vector_multiply(vector_subtract(x1,x3),second);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(v0,v1));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+quarter),vector_multiply(vector_subtract(v0,v1),third));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+2*quarter),vector_add(v2,v3));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+3*quarter),vector_multiply(vector_subtract(v2,v3),third));
            }
        }
        if(length==16) {
            const auto half=length>>1;
            for(std::size_t block=0;block<n;block+=length) {
                std::size_t j=0;
                for(;j+8<=half;j+=8) {
                    auto u=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                    auto v=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+half));
                    if constexpr (!SkipFirst) if(length==n) {
#ifndef SKIP_RUNTIME_CHECKS
                        if constexpr(Check){vector_check(u);vector_check(v);}
#endif
                        const auto r2=_mm256_set1_epi32(static_cast<int>(r_squared_));
                        u=vector_multiply(u,r2);v=vector_multiply(v,r2);
                    }
                    const auto roots=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+half+j));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(u,v));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+half),vector_multiply(vector_subtract(u,v),roots));
                }
                for(;j<half;++j) {
                    auto u=a[block+j],v=a[block+j+half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr(Check)if(length==n&&(u>=Mod||v>=Mod))throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    if constexpr (!SkipFirst) if(length==n){u=encode(u);v=encode(v);}
                    a[block+j]=working_add(u,v);a[block+j+half]=working_multiply(working_subtract(u,v),roots_[half+j]);
                }
            }
        }
        const auto u4=_mm256_setr_epi32(0,1,0,1,4,5,4,5),v4=_mm256_setr_epi32(2,3,2,3,6,7,6,7);
        const auto r4=_mm256_set1_epi32(static_cast<int>(roots_[3]));
        const auto r8=_mm256_setr_epi32(static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]),static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]));
        for(std::size_t block=0;block<n;block+=8) {
            auto x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block));
            auto u=_mm256_permute2x128_si256(x,x,0),v=_mm256_permute2x128_si256(x,x,0x11);
            x=_mm256_blend_epi32(vector_add(u,v),vector_multiply(vector_subtract(u,v),r8),0xf0);
            u=_mm256_permutevar8x32_epi32(x,u4);v=_mm256_permutevar8x32_epi32(x,v4);
            x=_mm256_blend_epi32(vector_add(u,v),vector_multiply_quarter_root(vector_subtract(u,v),r4),0xcc);
            u=_mm256_shuffle_epi32(x,_MM_SHUFFLE(2,2,0,0));v=_mm256_shuffle_epi32(x,_MM_SHUFFLE(3,3,1,1));
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xaa);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block),x);
        }
    }
    template<bool Check>
    [[gnu::target("avx2")]]
    void forward_zero_padded_avx2(std::vector<std::uint32_t>& a) const {
        const auto n=a.size(), quarter=n/4;
        const auto r2=_mm256_set1_epi32(static_cast<int>(r_squared_));
        // 後半はresizeが補ったゼロ。初段のゼロの符号化と積を省く。
        for(std::size_t j=0;j<quarter;j+=8) {
            auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+j));
            auto x1=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+quarter+j));
#ifndef SKIP_RUNTIME_CHECKS
            if constexpr(Check){vector_check(x0);vector_check(x1);}
#endif
            x0=vector_multiply(x0,r2);x1=vector_multiply(x1,r2);
            const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
            const auto second=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+3*quarter+j));
            const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
            const auto v2=vector_multiply(x0,first),v3=vector_multiply(x1,second);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+j),vector_add(x0,x1));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+quarter+j),vector_multiply(vector_subtract(x0,x1),third));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+2*quarter+j),vector_add(v2,v3));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+3*quarter+j),vector_multiply(vector_subtract(v2,v3),third));
        }
        forward_avx2<false,true>(a,n/4);
    }
    template<bool Check, bool Encode>
    [[gnu::target("avx2")]]
    void forward_zero_quarters_avx2(std::vector<std::uint32_t>& a,
                                    std::size_t length, std::size_t active) const {
        const auto quarter=length/4;
        // 各区間の先頭active要素以外は、以前の段でもゼロのまま。
        for(std::size_t block=0;block<a.size();block+=length)for(std::size_t j=0;j<active;j+=8) {
            auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
            if constexpr(Encode) {
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr(Check)vector_check(x0);
#endif
                x0=vector_multiply(x0,_mm256_set1_epi32(static_cast<int>(r_squared_)));
            }
            const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
            const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
            const auto v2=vector_multiply(x0,first);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),x0);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+quarter+j),vector_multiply(x0,third));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+2*quarter+j),v2);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+3*quarter+j),vector_multiply(v2,third));
        }
    }
    template<bool Check>
    [[gnu::target("avx2")]]
    void backward_avx2(std::vector<std::uint32_t>& a) const {
        const auto n=a.size();

        const auto u4=_mm256_setr_epi32(0,1,0,1,4,5,4,5),v4=_mm256_setr_epi32(2,3,2,3,6,7,6,7);
        const auto r4=_mm256_set1_epi32(static_cast<int>(roots_[3]));
        const auto r8=_mm256_setr_epi32(static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]),static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]));
        for(std::size_t block=0;block<n;block+=8) {
            auto x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block));
            auto u=_mm256_shuffle_epi32(x,_MM_SHUFFLE(2,2,0,0)),v=_mm256_shuffle_epi32(x,_MM_SHUFFLE(3,3,1,1));
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xaa);
            u=_mm256_permutevar8x32_epi32(x,u4);v=vector_multiply_quarter_root(_mm256_permutevar8x32_epi32(x,v4),r4);
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xcc);
            u=_mm256_permute2x128_si256(x,x,0);v=vector_multiply(_mm256_permute2x128_si256(x,x,0x11),r8);
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xf0);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block),x);
        }

        std::size_t completed=8;
        for(std::size_t length=32;length<=n;) {
            const auto quarter=length/4;
            for(std::size_t block=0;block<n;block+=length)for(std::size_t j=0;j<quarter;j+=8) {
                const auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                const auto raw1=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+quarter));
                const auto x2=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+2*quarter));
                const auto raw3=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+3*quarter));
                const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
                const auto second=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
                const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+3*quarter+j));
                const auto x1=vector_multiply(raw1,first),x3=vector_multiply(raw3,first);
                const auto v0=vector_add(x0,x1),v1=vector_subtract(x0,x1);
                const auto v2=vector_multiply(vector_add(x2,x3),second),v3=vector_multiply(vector_subtract(x2,x3),third);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(v0,v2));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+quarter),vector_add(v1,v3));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+2*quarter),vector_subtract(v0,v2));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+3*quarter),vector_subtract(v1,v3));
            }
            completed=length;
            if(length>n/4)break;
            length*=4;
        }
        if(completed<n) {
            const auto length=n;
            const auto half=length>>1;
            for(std::size_t block=0;block<n;block+=length) {
                std::size_t j=0;
                for(;j+8<=half;j+=8) {
                    const auto u=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                    const auto raw=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+half));
                    const auto roots=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+half+j));
                    const auto v=vector_multiply(raw,roots);
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(u,v));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+half),vector_subtract(u,v));
                }
                for(;j<half;++j) {
                    const auto u=a[block+j],raw=a[block+j+half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr(Check)if(length==2&&(u>=Mod||raw>=Mod))throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    const auto v=working_multiply(raw,roots_[half+j]);
                    a[block+j]=working_add(u,v);a[block+j+half]=working_subtract(u,v);
                }
            }
        }
        // nはMod-1の約数（変換長は対応する2冪）。
        // d=(Mod-1)/nなら、n*(Mod-d)=(n-1)*Mod+1。
        // よってMod-dがnの逆元になる。
        const auto inverse_n=static_cast<std::uint32_t>(Mod - (Mod - 1) / static_cast<std::uint32_t>(n));
        const auto scale=_mm256_set1_epi32(static_cast<int>(inverse_n));
        const auto reverse=_mm256_setr_epi32(7,6,5,4,3,2,1,0);
        a[0]=working_normalize(working_multiply(a[0],inverse_n));
        std::size_t i=1;
        for(;i+8<=n/2;i+=8) {
            const auto lo=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+i));
            const auto hi=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+n-i-7));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+i),_mm256_permutevar8x32_epi32(vector_reduce(vector_multiply(hi,scale)),reverse));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+n-i-7),_mm256_permutevar8x32_epi32(vector_reduce(vector_multiply(lo,scale)),reverse));
        }
        for(;i<n-i;++i) {
            const auto lo=working_normalize(working_multiply(a[i],inverse_n)),hi=working_normalize(working_multiply(a[n-i],inverse_n));
            a[i]=hi;a[n-i]=lo;
        }
        a[n/2]=working_normalize(working_multiply(a[n/2],inverse_n));
    }
#endif

    template <bool Check>
    void forward(std::vector<std::uint32_t>& a) const {
        const std::size_t n = a.size();
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if constexpr(montgomery_)if(use_avx2(n)){forward_avx2<Check>(a);return;}
#endif
        for (std::size_t length = n; length >= 2; length >>= 1) {
            const std::size_t half = length >> 1;
            for (std::size_t block = 0; block < n; block += length)
                for (std::size_t j = 0; j < half; ++j) {
                    auto u = a[block + j], v = a[block + j + half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr (Check) if (length == n && (u >= Mod || v >= Mod))
                        throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    if constexpr(montgomery_)if(length==n){u=encode(u);v=encode(v);}
                    a[block + j] = working_add(u, v);
                    a[block + j + half] = working_multiply(working_subtract(u, v), roots_[half + j]);
                }
        }
    }
    template<bool Check>
    void forward_padded(std::vector<std::uint32_t>& a, [[maybe_unused]] std::size_t input_size) const {
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if constexpr(montgomery_) if(a.size()>=32 && input_size<=a.size()/2 && use_avx2(a.size())) {
            if(input_size<=a.size()/4) {
                // 長さは上限検査済み。8要素へ丸めても各区間の1/4以内に収まる。
                const auto active=(input_size+7)&~std::size_t{7};
                auto length=a.size();
                forward_zero_quarters_avx2<Check,true>(a,length,active);
                length/=4;
                while(length>=32 && active<=length/4) {
                    forward_zero_quarters_avx2<false,false>(a,length,active);
                    length/=4;
                }
                forward_avx2<false,true>(a,length);return;
            }
            forward_zero_padded_avx2<Check>(a);return;
        }
#endif
        forward<Check>(a);
    }
    template <bool Check>
    void backward(std::vector<std::uint32_t>& a) const {
        const std::size_t n = a.size();
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if constexpr(montgomery_)if(use_avx2(n)){backward_avx2<Check>(a);return;}
#endif
        for (std::size_t length = 2; length <= n; length <<= 1) {
            const std::size_t half = length >> 1;
            for (std::size_t block = 0; block < n; block += length)
                for (std::size_t j = 0; j < half; ++j) {
                    const auto u = a[block + j], raw = a[block + j + half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr (Check) if (length == 2 && (u >= Mod || raw >= Mod))
                        throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    const auto v = working_multiply(raw, roots_[half + j]);
                    a[block + j] = working_add(u, v);
                    a[block + j + half] = working_subtract(u, v);
                }
        }
        // nはMod-1の約数（変換長は対応する2冪）。
        // d=(Mod-1)/nなら、n*(Mod-d)=(n-1)*Mod+1。
        // よってMod-dがnの逆元になる。
        const auto inverse_n = static_cast<std::uint32_t>(Mod - (Mod - 1) / static_cast<std::uint32_t>(n));
        a[0] = working_normalize(working_multiply(a[0], inverse_n));
        for (std::size_t i = 1; i < n - i; ++i) {
            const auto lo = working_normalize(working_multiply(a[i], inverse_n)), hi = working_normalize(working_multiply(a[n - i], inverse_n));
            a[i] = hi; a[n - i] = lo;
        }
        if (n > 1) a[n / 2] = working_normalize(working_multiply(a[n / 2], inverse_n));
    }
    template <bool Check>
    std::vector<std::uint32_t> compute(std::vector<std::uint32_t> a, std::vector<std::uint32_t> b) {
        const std::size_t output = result_size(a.size(), b.size());
        if (output == 0) return {};
        if (std::min(a.size(), b.size()) <= 32) {
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
            if(use_avx2(output))return naive_avx2<Check>(a,b,output);
#endif
            std::vector<std::uint32_t> result(output);
            for (std::size_t i = 0; i < a.size(); ++i) {
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr (Check) if (a[i] >= Mod) throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                for (std::size_t j = 0; j < b.size(); ++j) {
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr (Check) if (i == 0 && b[j] >= Mod)
                        throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    result[i + j] = add(result[i + j], multiply(a[i], b[j]));
                }
            }
            return result;
        }
        const auto length = transform_size(output);
        prepare(length);
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        const auto input_a=a.size(), input_b=b.size();
#endif
        a.resize(length); b.resize(length);
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        forward_padded<Check>(a,input_a); forward_padded<Check>(b,input_b);
#else
        forward<Check>(a); forward<Check>(b);
#endif
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if(use_avx2(length))pointwise_avx2(a,b);
        else
#endif
        for (std::size_t i = 0; i < length; ++i) a[i] = working_multiply(a[i], b[i]);
        backward<false>(a); a.resize(output);
        return a;
    }

public:
    static constexpr std::uint32_t modulus() noexcept { return Mod; }
    static constexpr std::size_t max_length() noexcept { return std::size_t{1} << rank_; }
    [[nodiscard]] std::size_t roots_capacity() const noexcept { return roots_.size(); }
    void clear_cache() noexcept { std::vector<std::uint32_t>{}.swap(roots_); }
    void prepare(std::size_t length) {
        if (length == 0) return;
        if (!std::has_single_bit(length) || length > max_length())
            throw std::length_error("NTT: length must be a supported power of two");
        if (length <= 1 || length <= roots_.size()) return;
        const std::size_t previous = std::max<std::size_t>(2, roots_.size());
        roots_.resize(length); roots_[1] = encode(1);
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if constexpr (montgomery_) if (length >= 256 && use_avx2(length)) {
            prepare_avx2(previous, length);
            return;
        }
#endif
        for (std::size_t half = previous; half < length; half <<= 1) {
            const auto step = stage_roots_[std::countr_zero(half) + 1];
            for (std::size_t j = 0; j < half/2; ++j) {
                const auto value=roots_[half/2+j];
                roots_[half+2*j]=value;
                roots_[half+2*j+1]=working_multiply(value,step);
            }
        }
    }
    void transform(std::vector<std::uint32_t>& a, bool inverse = false) {
        if (a.empty()) return;
        prepare(a.size());
        if (a.size() == 1) {
#ifndef SKIP_RUNTIME_CHECKS
            if (a[0] >= Mod) throw std::invalid_argument("NTT: coefficient not normalized");
#endif
            return;
        }
        if constexpr(montgomery_) {
            if(inverse){bit_reverse_convert<true,true>(a);backward<false>(a);}
            else{forward<true>(a);bit_reverse_convert<false,false>(a);}
        }else{
        if (inverse) { bit_reverse(a); backward<true>(a); }
        else { forward<true>(a); bit_reverse(a); }
        }
    }
    template <std::integral T, std::integral U>
    std::vector<std::uint32_t> convolution(const std::vector<T>& a, const std::vector<U>& b) {
        if (a.empty() || b.empty()) return {};
        const auto output = result_size(a.size(), b.size());
        if (std::min(a.size(), b.size()) > 32) (void)transform_size(output);
        std::vector<std::uint32_t> x(a.size()), y(b.size());
        for (std::size_t i = 0; i < a.size(); ++i) x[i] = normalize<T>(a[i]);
        for (std::size_t i = 0; i < b.size(); ++i) y[i] = normalize<U>(b[i]);
        return compute<false>(std::move(x), std::move(y));
    }
    std::vector<std::uint32_t> convolution_normalized(std::vector<std::uint32_t> a,
                                                      std::vector<std::uint32_t> b) {
        return compute<true>(std::move(a), std::move(b));
    }
    template <std::integral T>
    std::vector<std::uint32_t> square(const std::vector<T>& a) {
        if (a.size() <= 32) return convolution(a, a);
        const auto output = result_size(a.size(), a.size()), length = transform_size(output);
        prepare(length);
        std::vector<std::uint32_t> x(length);
        for (std::size_t i = 0; i < a.size(); ++i) x[i] = normalize<T>(a[i]);
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        forward_padded<false>(x,a.size());
#else
        forward<false>(x);
#endif
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if(use_avx2(length))pointwise_avx2(x,x);
        else
#endif
        for (auto& value : x) value = working_multiply(value, value);
        backward<false>(x); x.resize(output);
        return x;
    }
};
#endif

#ifndef SIRKIT_CONVOLUTION_MOD_DEFINED
#define SIRKIT_CONVOLUTION_MOD_DEFINED
template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3, std::integral T, std::integral U>
std::vector<std::uint32_t> ConvolutionMod(const std::vector<T>& a, const std::vector<U>& b) {
    return NTT<Mod, Root>{}.convolution(a, b);
}
#endif

#include <initializer_list>
#include <optional>

#ifndef SIRKIT_FORMAL_POWER_SERIES_DETAIL_DEFINED
#define SIRKIT_FORMAL_POWER_SERIES_DETAIL_DEFINED
namespace reinsirk::detail {
inline constexpr std::uint32_t fps_modulus = 998244353;
inline std::uint32_t fps_add(std::uint32_t a, std::uint32_t b) noexcept {
    const auto value = a + b; return value >= fps_modulus ? value - fps_modulus : value;
}
inline std::uint32_t fps_subtract(std::uint32_t a, std::uint32_t b) noexcept {
    return a >= b ? a - b : a + fps_modulus - b;
}
inline std::uint32_t fps_multiply(std::uint32_t a, std::uint32_t b) noexcept {
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % fps_modulus);
}
inline std::uint32_t fps_power(std::uint32_t value, std::uint64_t exponent) noexcept {
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1) {
        if (exponent & 1) result = fps_multiply(result, value);
        value = fps_multiply(value, value);
    }
    return result;
}
inline NTT<>& fps_engine() {
    static thread_local NTT<> engine;
    return engine;
}
inline std::vector<std::uint32_t>& fps_integer_inverses() {
    static thread_local std::vector<std::uint32_t> values;
    return values;
}
inline void fps_prepare_inverses(std::size_t n) {
    if (n >= fps_modulus) throw std::length_error("FormalPowerSeries: noninvertible integration index");
    auto& table = fps_integer_inverses();
    if (table.size() < 2) { table.resize(2); table[1] = 1; }
    if (n < table.size()) return;
    const auto first = table.size(); table.resize(n + 1);
    for (std::size_t i = first; i <= n; ++i)
        table[i] = fps_modulus - static_cast<std::uint32_t>(
            std::uint64_t{fps_modulus / static_cast<std::uint32_t>(i)} * table[fps_modulus % i] % fps_modulus);
}
inline void fps_require_precision(std::size_t n) {
    if (n > NTT<>::max_length() / 2) throw std::length_error("FormalPowerSeries: precision exceeds NTT budget");
}
inline std::optional<std::uint32_t> fps_scalar_sqrt(std::uint32_t value) noexcept {
    if (value == 0) return 0;
    if (fps_power(value, (fps_modulus - 1) / 2) != 1) return std::nullopt;
    constexpr std::uint32_t odd = 119;
    auto root = fps_power(value, (odd + 1) / 2), remainder = fps_power(value, odd);
    auto correction = fps_power(3, odd);
    unsigned remaining = 23;
    while (remainder != 1) {
        unsigned first = 0; auto probe = remainder;
        while (probe != 1 && first < remaining) { probe = fps_multiply(probe, probe); ++first; }
        if (first == remaining) return std::nullopt;
        const auto change = fps_power(correction, std::uint32_t{1} << (remaining - first - 1));
        root = fps_multiply(root, change); correction = fps_multiply(change, change);
        remainder = fps_multiply(remainder, correction); remaining = first;
    }
    return std::min(root, fps_modulus - root);
}
}
#endif

#ifndef SIRKIT_FORMAL_POWER_SERIES_DEFINED
#define SIRKIT_FORMAL_POWER_SERIES_DEFINED
class FormalPowerSeries {
    std::vector<std::uint32_t> coefficients_;
    struct Normalized {};
    FormalPowerSeries(std::vector<std::uint32_t> values, Normalized) noexcept : coefficients_(std::move(values)) {}
    template <std::integral T>
    static std::uint32_t normalize(T value) noexcept {
        static_assert(sizeof(T) <= sizeof(std::uint64_t));
        if constexpr (std::is_signed_v<T>) {
            const auto remainder = static_cast<std::int64_t>(value) % static_cast<std::int64_t>(modulus());
            return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus() : remainder);
        } else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value) % modulus());
    }
    std::vector<std::uint32_t> short_prefix(std::size_t n) const {
        return {coefficients_.begin(), coefficients_.begin() + std::min(n, size())};
    }
    static FormalPowerSeries normalized(std::vector<std::uint32_t> values) noexcept {
        return FormalPowerSeries(std::move(values), Normalized{});
    }
public:
    static constexpr std::uint32_t modulus() noexcept { return reinsirk::detail::fps_modulus; }
    static constexpr std::size_t max_precision() noexcept { return NTT<>::max_length() / 2; }
    FormalPowerSeries() = default;
    explicit FormalPowerSeries(std::size_t n) : coefficients_(n) {}
    explicit FormalPowerSeries(std::vector<std::uint32_t> values) : coefficients_(std::move(values)) {
        for (auto& value : coefficients_) value %= modulus();
    }
    template <std::integral T>
    explicit FormalPowerSeries(const std::vector<T>& values) : coefficients_(values.size()) {
        for (std::size_t i = 0; i < values.size(); ++i) coefficients_[i] = normalize<T>(values[i]);
    }
    FormalPowerSeries(std::initializer_list<std::int64_t> values) : coefficients_(values.size()) {
        std::size_t at = 0; for (auto value : values) coefficients_[at++] = normalize(value);
    }
    [[nodiscard]] std::size_t size() const noexcept { return coefficients_.size(); }
    [[nodiscard]] bool empty() const noexcept { return coefficients_.empty(); }
    [[nodiscard]] const std::vector<std::uint32_t>& coefficients() const noexcept { return coefficients_; }
    [[nodiscard]] std::uint32_t operator[](std::size_t i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i >= size()) throw std::out_of_range("FormalPowerSeries: coefficient");
#endif
        return coefficients_[i];
    }
    [[nodiscard]] std::uint32_t get(std::size_t i) const noexcept { return i < size() ? coefficients_[i] : 0; }
    template <std::integral T>
    void set(std::size_t i, T value) {
#ifndef SKIP_RUNTIME_CHECKS
        if (i >= size()) throw std::out_of_range("FormalPowerSeries: coefficient");
#endif
        coefficients_[i] = normalize(value);
    }
    void resize(std::size_t n) { coefficients_.resize(n); }
    void trim() noexcept { while (!empty() && coefficients_.back() == 0) coefficients_.pop_back(); }
    [[nodiscard]] FormalPowerSeries prefix(std::size_t n) const {
        std::vector<std::uint32_t> result(n);
        std::copy_n(coefficients_.begin(), std::min(n, size()), result.begin());
        return normalized(std::move(result));
    }
    FormalPowerSeries& operator+=(const FormalPowerSeries& other) {
        if (other.size() > size()) coefficients_.resize(other.size());
        for (std::size_t i = 0; i < other.size(); ++i) coefficients_[i] = reinsirk::detail::fps_add(coefficients_[i], other.coefficients_[i]);
        return *this;
    }
    FormalPowerSeries& operator-=(const FormalPowerSeries& other) {
        if (other.size() > size()) coefficients_.resize(other.size());
        for (std::size_t i = 0; i < other.size(); ++i) coefficients_[i] = reinsirk::detail::fps_subtract(coefficients_[i], other.coefficients_[i]);
        return *this;
    }
    FormalPowerSeries& operator*=(const FormalPowerSeries& other) {
        auto result = reinsirk::detail::fps_engine().convolution_normalized(coefficients_, other.coefficients_);
        coefficients_.swap(result); return *this;
    }
    FormalPowerSeries& operator/=(const FormalPowerSeries& other) { *this = divmod(other).first; return *this; }
    FormalPowerSeries& operator%=(const FormalPowerSeries& other) { *this = divmod(other).second; return *this; }
    friend FormalPowerSeries operator+(FormalPowerSeries a, const FormalPowerSeries& b) { a += b; return a; }
    friend FormalPowerSeries operator-(FormalPowerSeries a, const FormalPowerSeries& b) { a -= b; return a; }
    friend FormalPowerSeries operator*(const FormalPowerSeries& a, const FormalPowerSeries& b) {
        return normalized(reinsirk::detail::fps_engine().convolution_normalized(a.coefficients_, b.coefficients_));
    }
    friend FormalPowerSeries operator/(const FormalPowerSeries& a, const FormalPowerSeries& b) { return a.divmod(b).first; }
    friend FormalPowerSeries operator%(const FormalPowerSeries& a, const FormalPowerSeries& b) { return a.divmod(b).second; }
    friend bool operator==(const FormalPowerSeries&, const FormalPowerSeries&) = default;
    template <std::integral T>
    [[nodiscard]] FormalPowerSeries scale(T scalar) const {
        auto result = coefficients_; const auto value = normalize(scalar);
        for (auto& coefficient : result) coefficient = reinsirk::detail::fps_multiply(coefficient, value);
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries derivative() const {
        if (empty()) return {};
        std::vector<std::uint32_t> result(size() - 1);
        for (std::size_t i = 1; i < size(); ++i)
            result[i - 1] = reinsirk::detail::fps_multiply(coefficients_[i], static_cast<std::uint32_t>(i % modulus()));
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries integral() const {
        if (empty()) return FormalPowerSeries{0};
        reinsirk::detail::fps_prepare_inverses(size());
        const auto& inverses = reinsirk::detail::fps_integer_inverses();
        std::vector<std::uint32_t> result(size() + 1);
        for (std::size_t i = 0; i < size(); ++i) result[i + 1] = reinsirk::detail::fps_multiply(coefficients_[i], inverses[i + 1]);
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries inverse(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
        if (get(0) == 0) throw std::domain_error("FormalPowerSeries: inverse of zero constant");
        std::vector<std::uint32_t> result{reinsirk::detail::fps_power(get(0), modulus() - 2)};
        auto& engine = reinsirk::detail::fps_engine();
        for (std::size_t known = 1; known < n;) {
            const auto target = std::min(n, known * 2);
            auto error = engine.convolution_normalized(short_prefix(target), result);
            error.resize(target);
            for (auto& coefficient : error) coefficient = coefficient == 0 ? 0 : modulus() - coefficient;
            error[0] = reinsirk::detail::fps_add(error[0], 2);
            auto next = engine.convolution_normalized(std::move(result), std::move(error)); next.resize(target);
            result.swap(next); known = target;
        }
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries inverse() const { return inverse(size()); }
    [[nodiscard]] FormalPowerSeries log(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
#ifndef SKIP_RUNTIME_CHECKS
        if (get(0) != 1) throw std::domain_error("FormalPowerSeries: logarithm constant must be one");
#endif
        auto differentiated = normalized(short_prefix(n)).derivative();
        auto result = reinsirk::detail::fps_engine().convolution_normalized(
            std::move(differentiated.coefficients_), inverse(n).coefficients_);
        result.resize(n - 1);
        return normalized(std::move(result)).integral();
    }
    [[nodiscard]] FormalPowerSeries log() const { return log(size()); }
    [[nodiscard]] FormalPowerSeries exp(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
#ifndef SKIP_RUNTIME_CHECKS
        if (get(0) != 0) throw std::domain_error("FormalPowerSeries: exponential constant must be zero");
#endif
        FormalPowerSeries result{1};
        for (std::size_t known = 1; known < n;) {
            const auto target = std::min(n, known * 2);
            auto correction = result.log(target).coefficients_;
            for (std::size_t i = 0; i < target; ++i) correction[i] = reinsirk::detail::fps_subtract(get(i), correction[i]);
            correction[0] = reinsirk::detail::fps_add(correction[0], 1);
            auto next = reinsirk::detail::fps_engine().convolution_normalized(std::move(result.coefficients_), std::move(correction));
            next.resize(target); result.coefficients_.swap(next); known = target;
        }
        return result;
    }
    [[nodiscard]] FormalPowerSeries exp() const { return exp(size()); }
    [[nodiscard]] FormalPowerSeries pow(std::uint64_t exponent, std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return {};
        if (exponent == 0) { auto result = FormalPowerSeries(n); result.coefficients_[0] = 1; return result; }
        std::size_t leading = 0;
        while (leading < std::min(n, size()) && coefficients_[leading] == 0) ++leading;
        if (leading == std::min(n, size()) || (leading != 0 && exponent > (n - 1) / leading)) return FormalPowerSeries(n);
        const auto shift = leading == 0 ? 0 : leading * static_cast<std::size_t>(exponent);
        const auto remaining = n - shift;
        const auto first = coefficients_[leading], inverse_first = reinsirk::detail::fps_power(first, modulus() - 2);
        std::vector<std::uint32_t> unit(std::min(remaining, size() - leading));
        for (std::size_t i = 0; i < unit.size(); ++i) unit[i] = reinsirk::detail::fps_multiply(coefficients_[leading + i], inverse_first);
        auto logarithm = normalized(std::move(unit)).log(remaining);
        const auto reduced_exponent = static_cast<std::uint32_t>(exponent % modulus());
        for (auto& coefficient : logarithm.coefficients_) coefficient = reinsirk::detail::fps_multiply(coefficient, reduced_exponent);
        auto powered = logarithm.exp(remaining);
        const auto multiplier = reinsirk::detail::fps_power(first, exponent);
        std::vector<std::uint32_t> result(n);
        for (std::size_t i = 0; i < remaining; ++i) result[shift + i] = reinsirk::detail::fps_multiply(powered.coefficients_[i], multiplier);
        return normalized(std::move(result));
    }
    [[nodiscard]] FormalPowerSeries pow(std::uint64_t exponent) const { return pow(exponent, size()); }
    [[nodiscard]] std::optional<FormalPowerSeries> sqrt(std::size_t n) const {
        reinsirk::detail::fps_require_precision(n);
        if (n == 0) return FormalPowerSeries{};
        std::size_t leading = 0;
        while (leading < std::min(n, size()) && coefficients_[leading] == 0) ++leading;
        if (leading == std::min(n, size())) return FormalPowerSeries(n);
        if (leading & 1) return std::nullopt;
        const auto root = reinsirk::detail::fps_scalar_sqrt(coefficients_[leading]);
        if (!root) return std::nullopt;
        const auto needed = n - leading;
        FormalPowerSeries result{static_cast<std::int64_t>(*root)};
        for (std::size_t known = 1; known < needed;) {
            const auto target = std::min(needed, known * 2);
            std::vector<std::uint32_t> part(std::min(target, size() - leading));
            std::copy_n(coefficients_.begin() + leading, part.size(), part.begin());
            auto ratio = reinsirk::detail::fps_engine().convolution_normalized(std::move(part), result.inverse(target).coefficients_);
            ratio.resize(target);
            for (std::size_t i = 0; i < target; ++i)
                ratio[i] = reinsirk::detail::fps_multiply(reinsirk::detail::fps_add(ratio[i], result.get(i)), (modulus() + 1) / 2);
            result.coefficients_.swap(ratio); known = target;
        }
        std::vector<std::uint32_t> shifted(n);
        std::copy(result.coefficients_.begin(), result.coefficients_.end(), shifted.begin() + leading / 2);
        return normalized(std::move(shifted));
    }
    [[nodiscard]] std::optional<FormalPowerSeries> sqrt() const { return sqrt(size()); }
    [[nodiscard]] std::pair<FormalPowerSeries, FormalPowerSeries> divmod(const FormalPowerSeries& divisor) const {
        auto a = *this, b = divisor; a.trim(); b.trim();
        if (b.empty()) throw std::domain_error("FormalPowerSeries: zero polynomial divisor");
        if (a.size() < b.size()) return {FormalPowerSeries{}, std::move(a)};
        const auto quotient_size = a.size() - b.size() + 1;
        FormalPowerSeries quotient;
        if (std::min(quotient_size, b.size()) <= 32) {
            quotient.resize(quotient_size);
            const auto inverse_leading = reinsirk::detail::fps_power(b.coefficients_.back(), modulus() - 2);
            for (std::size_t at = quotient_size; at != 0; --at) {
                const auto index = at - 1;
                const auto coefficient = reinsirk::detail::fps_multiply(a.coefficients_[index + b.size() - 1], inverse_leading);
                quotient.coefficients_[index] = coefficient;
                for (std::size_t j = 0; j < b.size(); ++j)
                    a.coefficients_[index + j] = reinsirk::detail::fps_subtract(a.coefficients_[index + j],
                        reinsirk::detail::fps_multiply(coefficient, b.coefficients_[j]));
            }
            a.resize(b.size() - 1); a.trim(); quotient.trim();
            return {std::move(quotient), std::move(a)};
        }
        reinsirk::detail::fps_require_precision(quotient_size);
        std::vector<std::uint32_t> reversed_a(quotient_size), reversed_b(std::min(quotient_size, b.size()));
        std::copy_n(a.coefficients_.rbegin(), quotient_size, reversed_a.begin());
        std::copy_n(b.coefficients_.rbegin(), reversed_b.size(), reversed_b.begin());
        auto reversed_quotient = reinsirk::detail::fps_engine().convolution_normalized(
            std::move(reversed_a), normalized(std::move(reversed_b)).inverse(quotient_size).coefficients_);
        reversed_quotient.resize(quotient_size); std::reverse(reversed_quotient.begin(), reversed_quotient.end());
        quotient = normalized(std::move(reversed_quotient));
        auto remainder = std::move(a) - b * quotient;
        remainder.resize(b.size() - 1); remainder.trim(); quotient.trim();
        return {std::move(quotient), std::move(remainder)};
    }
    static void clear_cache() noexcept {
        reinsirk::detail::fps_engine().clear_cache();
        std::vector<std::uint32_t>{}.swap(reinsirk::detail::fps_integer_inverses());
    }
};
#endif

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#ifndef SIRKIT_POLYNOMIAL_DETAIL_DEFINED
#define SIRKIT_POLYNOMIAL_DETAIL_DEFINED
namespace reinsirk::detail {
inline constexpr std::uint32_t polynomial_modulus=998244353;
inline std::uint32_t polynomial_multiply(std::uint32_t a,std::uint32_t b) noexcept{return static_cast<std::uint32_t>(std::uint64_t{a}*b%polynomial_modulus);}
inline std::uint32_t polynomial_power(std::uint32_t a,std::uint32_t e) noexcept {std::uint32_t r=1;for(;e;e>>=1,a=polynomial_multiply(a,a))if(e&1)r=polynomial_multiply(r,a);return r;}
template<std::integral T>
std::uint32_t polynomial_normalize(T value) noexcept {
    static_assert(sizeof(T)<=sizeof(std::uint64_t));
    if constexpr(std::is_signed_v<T>){const auto r=static_cast<std::int64_t>(value)%polynomial_modulus;return static_cast<std::uint32_t>(r<0?r+polynomial_modulus:r);}
    else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value)%polynomial_modulus);
}
inline std::uint32_t polynomial_horner(const FormalPowerSeries& f,std::uint32_t x) noexcept {
    std::uint32_t value=0;
    for(auto i=f.size();i;--i){value=polynomial_multiply(value,x);const auto sum=value+f.get(i-1);value=sum>=polynomial_modulus?sum-polynomial_modulus:sum;}
    return value;
}
}
#endif
#ifndef SIRKIT_PRODUCT_TREE_DEFINED
#define SIRKIT_PRODUCT_TREE_DEFINED
class ProductTree {
    std::size_t base_=0;
    std::vector<std::uint32_t> points_;
    std::vector<FormalPowerSeries> products_;
    void evaluate_node(const FormalPowerSeries& f,std::size_t node,std::size_t begin,std::size_t width,std::vector<std::uint32_t>& out) const {
        if(begin>=size())return;
        const auto count=std::min(width,size()-begin);
        if(count<=32||f.size()<=32){for(std::size_t i=begin;i<begin+count;++i)out[i]=reinsirk::detail::polynomial_horner(f,points_[i]);return;}
        const auto half=width/2;
        {const auto remainder=f%products_[node*2];evaluate_node(remainder,node*2,begin,half,out);}
        if(begin+half<size()){const auto remainder=f%products_[node*2+1];evaluate_node(remainder,node*2+1,begin+half,half,out);}
    }
    template<std::integral T>
    void build(const std::vector<T>& points) {
        if(points.size()>max_points())throw std::length_error("ProductTree: too many points");
        if(points.empty())return;
        base_=std::bit_ceil(points.size());
        points_.reserve(points.size());for(const auto value:points)points_.push_back(reinsirk::detail::polynomial_normalize(value));
        products_.resize(base_*2);
        for(std::size_t i=0;i<base_;++i){
            if(i<size())products_[base_+i]=FormalPowerSeries{points_[i]?std::int64_t{998244353}-points_[i]:0,1};
            else products_[base_+i]=FormalPowerSeries{1};
        }
        for(std::size_t node=base_-1;node;--node){
            if(products_[node*2+1].size()==1)products_[node]=products_[node*2];
            else products_[node]=products_[node*2]*products_[node*2+1];
        }
    }
public:
    static constexpr std::size_t max_points() noexcept{return FormalPowerSeries::max_precision()-1;}
    ProductTree()=default;
    template<std::integral T>explicit ProductTree(const std::vector<T>& points){build(points);}
    ProductTree(const ProductTree&)=default;
    ProductTree(ProductTree&& other) noexcept:base_(std::exchange(other.base_,0)),points_(std::move(other.points_)),products_(std::move(other.products_)){other.points_.clear();other.products_.clear();}
    ProductTree& operator=(const ProductTree& other){if(this!=&other){ProductTree copy(other);swap(copy);}return *this;}
    ProductTree& operator=(ProductTree&& other) noexcept {if(this!=&other){ProductTree moved(std::move(other));swap(moved);}return *this;}
    void swap(ProductTree& other) noexcept {std::swap(base_,other.base_);points_.swap(other.points_);products_.swap(other.products_);}
    friend void swap(ProductTree& a,ProductTree& b) noexcept {a.swap(b);}
    template<std::integral T>void assign(const std::vector<T>& points){ProductTree next(points);swap(next);}
    std::size_t size()const noexcept{return points_.size();}
    bool empty()const noexcept{return points_.empty();}
    const std::vector<std::uint32_t>& points()const noexcept{return points_;}
    FormalPowerSeries polynomial()const{return empty()?FormalPowerSeries{1}:products_[1];}
    std::vector<std::uint32_t> evaluate(const FormalPowerSeries& f)const {
        if(f.size()>FormalPowerSeries::max_precision())throw std::length_error("ProductTree: polynomial too long");
        std::vector<std::uint32_t> result(size());
        if(empty())return result;
        if(size()>32&&f.size()>32&&f.size()>size()){const auto remainder=f%products_[1];evaluate_node(remainder,1,0,base_,result);}
        else evaluate_node(f,1,0,base_,result);
        return result;
    }
    template<std::integral T>std::vector<std::uint32_t> evaluate(const std::vector<T>& coefficients)const{if(coefficients.size()>FormalPowerSeries::max_precision())throw std::length_error("ProductTree: polynomial too long");return evaluate(FormalPowerSeries(coefficients));}
    template<std::integral T>FormalPowerSeries interpolate(const std::vector<T>& values)const {
#ifndef SKIP_RUNTIME_CHECKS
        if(values.size()!=size())throw std::invalid_argument("ProductTree: values size mismatch");
#endif
        if(empty())return {};
        const auto denominators=evaluate(products_[1].derivative());
        std::vector<std::uint32_t> prefix(size()+1,1);
        for(std::size_t i=0;i<size();++i){
            if(denominators[i]==0)throw std::domain_error("ProductTree: repeated point");
            prefix[i+1]=reinsirk::detail::polynomial_multiply(prefix[i],denominators[i]);
        }
        auto inverse=reinsirk::detail::polynomial_power(prefix.back(),998244351);
        std::vector<FormalPowerSeries> values_tree(base_*2);
        for(std::size_t i=size();i;--i){
            const auto at=i-1;
            const auto weight=reinsirk::detail::polynomial_multiply(reinsirk::detail::polynomial_normalize(values[at]),reinsirk::detail::polynomial_multiply(prefix[at],inverse));
            inverse=reinsirk::detail::polynomial_multiply(inverse,denominators[at]);
            if(weight)values_tree[base_+at]=FormalPowerSeries{weight};
        }
        for(std::size_t node=base_-1;node;--node){
            const auto left=node*2,right=left+1;
            if(!values_tree[left].empty())values_tree[node]=values_tree[left]*products_[right];
            if(!values_tree[right].empty())values_tree[node]+=values_tree[right]*products_[left];
        }
        values_tree[1].resize(size());return std::move(values_tree[1]);
    }
};
#endif
#ifndef SIRKIT_MULTIPOINT_EVALUATE_DEFINED
#define SIRKIT_MULTIPOINT_EVALUATE_DEFINED
template<std::integral T>
std::vector<std::uint32_t> MultipointEvaluate(const FormalPowerSeries& f,const std::vector<T>& points){return ProductTree(points).evaluate(f);}
template<std::integral T,std::integral U>
std::vector<std::uint32_t> MultipointEvaluate(const std::vector<T>& f,const std::vector<U>& points){return ProductTree(points).evaluate(f);}
#endif
#ifndef SIRKIT_POLYNOMIAL_INTERPOLATE_DEFINED
#define SIRKIT_POLYNOMIAL_INTERPOLATE_DEFINED
template<std::integral T,std::integral U>
FormalPowerSeries PolynomialInterpolate(const std::vector<T>& points,const std::vector<U>& values){return ProductTree(points).interpolate(values);}
#endif
#ifndef SIRKIT_TAYLOR_SHIFT_DEFINED
#define SIRKIT_TAYLOR_SHIFT_DEFINED
template<std::integral T>
FormalPowerSeries TaylorShift(const FormalPowerSeries& f,T shift) {
    const auto n=f.size();if(n>FormalPowerSeries::max_precision())throw std::length_error("TaylorShift: polynomial too long");
    if(!n)return {};
    std::vector<std::uint32_t> factorial(n,1),inverse_factorial(n,1),left(n),right(n);
    for(std::size_t i=1;i<n;++i)factorial[i]=reinsirk::detail::polynomial_multiply(factorial[i-1],static_cast<std::uint32_t>(i));
    inverse_factorial[n-1]=reinsirk::detail::polynomial_power(factorial[n-1],998244351);
    for(std::size_t i=n-1;i;--i)inverse_factorial[i-1]=reinsirk::detail::polynomial_multiply(inverse_factorial[i],static_cast<std::uint32_t>(i));
    const auto c=reinsirk::detail::polynomial_normalize(shift);std::uint32_t power=1;
    for(std::size_t i=0;i<n;++i){left[n-1-i]=reinsirk::detail::polynomial_multiply(f.get(i),factorial[i]);right[i]=reinsirk::detail::polynomial_multiply(power,inverse_factorial[i]);power=reinsirk::detail::polynomial_multiply(power,c);}
    const auto product=FormalPowerSeries(std::move(left))*FormalPowerSeries(std::move(right));
    std::vector<std::uint32_t> result(n);
    for(std::size_t i=0;i<n;++i)result[i]=reinsirk::detail::polynomial_multiply(product.get(n-1-i),inverse_factorial[i]);
    return FormalPowerSeries(std::move(result));
}
template<std::integral T,std::integral U>
FormalPowerSeries TaylorShift(const std::vector<T>& f,U shift){if(f.size()>FormalPowerSeries::max_precision())throw std::length_error("TaylorShift: polynomial too long");return TaylorShift(FormalPowerSeries(f),shift);}
#endif


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
#include <vector>
int main(){
    std::size_t n{};if(!(verify_in>>n))return 0;
    std::vector<std::uint32_t> points(n),values(n);
    for(auto& value:points)verify_in>>value;
    for(auto& value:values)verify_in>>value;
    const auto result=PolynomialInterpolate(points,values);
    for(std::size_t i=0;i<result.size();++i)verify_out<<result.get(i)<<(i+1==result.size()?'\n':' ');
}
