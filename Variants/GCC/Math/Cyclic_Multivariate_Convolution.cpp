// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Cyclic_Multivariate_Convolution.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_multivariate_convolution.md
// 参考（問題設定）: https://judge.yosupo.jp/problem/multivariate_convolution_cyclic
// 参考（任意長変換）: https://doi.org/10.1109/TAU.1970.1162132
//
// API一覧
// cyclic_multivariate_convolution(dimensions,a,b,prime): 素数法での多次元巡回畳み込み。
// 各方向の長さがprime-1を割り切ること。最初の方向が最下位桁、全長Nは各方向の長さの積。
// a,bは長さNのuint32_t列で、入力値はprimeで正規化。N<=2^21。
// 原始根の計算を除きK方向でO(K+N log(N+1))時間、O(N)作業領域。長さ1の方向は無視する。
// dimensionsが空なら1要素同士の積を返す。
//
// [同梱のCRT畳み込み]
// CRTConvolution: 3素数NTTを所有する畳み込み。コピーの根cacheは独立、移動可。Mint不要。
// modular(a,b,mod): 整数vectorを1<=mod<=UINT32_MAXへ正規化し、modでの畳み込みvector<uint32_t>を返す。
// modular_square(a,mod): 同列の畳み込み。大規模では各NTTの順変換を1回にする。
// integer(a,b)/integer_square(a): int64_t列の厳密な畳み込みvector<int64_t>。全ての最終係数がint64_t内であることが前提。
// 中間の整数係数が大きくても相殺を扱う。前提違反を全て検出する保証はない。入力は変更しない。空を含めば{}。
// 結果長はmax_length()=2^23以下。短辺32以下ではO(nm)、それ以外はL=bit_ceil(n+m-1)としてO(L log L)。追加O(L)。
// prepare(length): 0または上限以下の2冪長の根を準備。新規O(length)、保持O(length)。長さ/容量の制限は検査省略時も維持。
// roots_capacity()/clear_cache(): 3表の保持根数の合計を返す/根を解放する。O(1)/O(保持数)。
// ConvolutionAnyMod(a,b,mod)/ConvolutionInt64(a,b): 一時オブジェクトを使う簡易版。反復時はCRTConvolutionを再利用する。
// GCC/Clangの128ビット整数拡張があれば厳密復元に使用。SIRKIT_CONVOLUTION_DISABLE_INT128または
// SIRKIT_FORCE_STANDARD_ARITHMETICで標準2語復元を選べる。
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
//
// [同梱の原始根]
// PrimeMultiplicativeGroup(p): 素数pを法とする乗法群。p-1をPollard–Rho法で一度だけ素因数分解する。
// modulus(): 法pを返す。
// order(a): aの位数を返す。aがpの倍数ならnullopt。O(log^2 p)回の剰余乗算。
// is_generator(a): aが原始根ならtrue。異なる素因数の数をdとしてO(d log p)回の剰余乗算。
// primitive_root(): 最小の原始根。候補をg個試すとO(g d log p)回の剰余乗算。
// primitive_root(p): 法pの乗法群を構築して最小の原始根を返す。
//
// [同梱64ビット因数分解]
// 参考（数学的手法。実装は独自構成）:
// https://doi.org/10.1007/BF01933190
// https://miller-rabin.appspot.com/
//
// PollardRho64(seed=定数): 乱数状態だけを持つ64bit因数分解器。コピーは独立、各操作は入力整数を変更しない。
// is_prime(n): uint64_t全域の素数判定。底7個の決定的Miller-Rabin、O(log n)回の剰余乗算。
// prime_factors(n): 素因数を重複込みの昇順で返す。factorize(n): {素数,指数}を昇順で返す。
// divisors(n,sorted=true): 正の約数。sorted=falseは列挙順、trueは昇順。divisor_count(n): 約数の個数。
// totient(n)/mobius(n): Eulerのφ/メビウス関数を返す。n=1では因数{}・約数{1}・φ=1・μ=1。
// 因数分解・約数・φ・μはn>=1が必要。n=0は検査省略時もdomain_error。
// 因数分解は乱択Brent法で、素因数pを分離する期待O(sqrt(p))回の剰余乗算。確定的時間上界は保証しない。
// 分解後、約数列挙はD個にO(D)、整列ありはO(D log D)。他は素因数の個数に比例。
// PrimeFactors64/Factorize64/Divisors64/DivisorCount64/EulerPhi64/Mobius64(n): 一時分解器を使う同名操作。
// 128bit拡張を使えるGCC/Clangでは高速演算、それ以外は標準C++の32bit limb演算。
// SIRKIT_FORCE_STANDARD_ARITHMETICで拡張側を除外できる。既存is_prime/MillerRabinとは部品・guardが独立。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
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


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NTT_DETAIL_DEFINED
#define SIRKIT_NTT_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
consteval bool ntt_prime(std::uint32_t n) noexcept {
    if (n < 2 || n % 2 == 0) return n == 2;
    for (std::uint32_t d = 3; d <= n / d; d += 2) if (n % d == 0) return false;
    return true;
}
consteval std::uint32_t ntt_power(std::uint32_t a, std::uint32_t exponent, std::uint32_t mod) noexcept {
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1) {
        if (exponent & 1) result = static_cast<std::uint32_t>(std::uint64_t{result} * a % mod);
        a = static_cast<std::uint32_t>(std::uint64_t{a} * a % mod);
    }
    return result;
}

inline std::size_t convolution_result_size(std::size_t a, std::size_t b, const char* overflow_message) {
    if (a == 0 || b == 0) return 0;
    if (a - 1 > std::numeric_limits<std::size_t>::max() - b)
        throw std::length_error(overflow_message);
    return a - 1 + b;
}
}
#endif

#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
#ifndef SIRKIT_NTT_SIMD_DETAIL_DEFINED
#define SIRKIT_NTT_SIMD_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
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
    static_assert(Mod >= 3 && Mod < (std::uint32_t{1} << 31) && /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_prime(Mod),
                  "NTT modulus must be an odd prime below 2^31");
    static_assert(Root > 0 && Root < Mod, "NTT root outside modulus");
    static constexpr int rank_ = std::countr_zero(Mod - 1);
    static constexpr std::uint32_t highest_root_ = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_power(Root, (Mod - 1) >> rank_, Mod);
    static_assert(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_power(highest_root_, std::uint32_t{1} << (rank_ - 1), Mod) == Mod - 1,
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
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_result_size(a, b, "NTT: convolution size overflow");
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
        if constexpr(montgomery_)return n>=16 && /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_avx2_available();
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


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <concepts>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <type_traits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_CRT_CONVOLUTION_DETAIL_DEFINED
#define SIRKIT_CRT_CONVOLUTION_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline constexpr std::uint32_t convolution_prime0=167772161,convolution_prime1=469762049,convolution_prime2=998244353;
inline constexpr std::uint64_t convolution_product01=std::uint64_t{convolution_prime0}*convolution_prime1;
inline constexpr auto convolution_inverse0=ntt_power(convolution_prime0,convolution_prime1-2,convolution_prime1);
inline constexpr auto convolution_inverse01=ntt_power(static_cast<std::uint32_t>(convolution_product01%convolution_prime2),convolution_prime2-2,convolution_prime2);
struct ConvolutionDigits {std::uint64_t low;std::uint32_t high;};
inline ConvolutionDigits convolution_crt_digits(std::uint32_t a,std::uint32_t b,std::uint32_t c) noexcept {
    const auto delta1=b>=a?b-a:b+convolution_prime1-a;
    const auto digit1=std::uint64_t{delta1}*convolution_inverse0%convolution_prime1;
    const auto low=std::uint64_t{a}+std::uint64_t{convolution_prime0}*digit1;
    const auto residue=static_cast<std::uint32_t>(low%convolution_prime2);
    const auto delta2=c>=residue?c-residue:c+convolution_prime2-residue;
    return {low,static_cast<std::uint32_t>(std::uint64_t{delta2}*convolution_inverse01%convolution_prime2)};
}
struct ConvolutionWide {std::uint64_t low,high;};
constexpr ConvolutionWide convolution_wide_product(std::uint64_t a,std::uint32_t b,std::uint64_t add=0) noexcept {
    const auto lower=(a&0xffffffffu)*b;
    const auto upper=(a>>32)*b;
    const auto low=lower+(upper<<32);
    const auto result=low+add;
    return {result,(upper>>32)+(low<lower)+(result<low)};
}
inline std::int64_t convolution_decode_integer(ConvolutionDigits digits) {
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_CONVOLUTION_DISABLE_INT128)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    __extension__ using Wide=unsigned __int128;
    constexpr Wide product=Wide{convolution_product01}*convolution_prime2;
    const Wide value=Wide{convolution_product01}*digits.high+digits.low;
    const bool negative=value>product/2;
    const Wide magnitude=negative?product-value:value;
    const Wide limit=Wide{std::numeric_limits<std::int64_t>::max()}+(negative?1u:0u);
    if(magnitude>limit)throw std::overflow_error("CRTConvolution: coefficient outside int64_t");
    const auto small=static_cast<std::uint64_t>(magnitude);
#else
    constexpr auto product=convolution_wide_product(convolution_product01,convolution_prime2);
    const auto value=convolution_wide_product(convolution_product01,digits.high,digits.low);
    constexpr ConvolutionWide half{(product.low>>1)|(product.high<<63),product.high>>1};
    const bool negative=value.high>half.high||(value.high==half.high&&value.low>half.low);
    const ConvolutionWide magnitude=negative?ConvolutionWide{product.low-value.low,product.high-value.high-(product.low<value.low)}:value;
    const auto limit=std::uint64_t{std::numeric_limits<std::int64_t>::max()}+(negative?1u:0u);
    if(magnitude.high||magnitude.low>limit)throw std::overflow_error("CRTConvolution: coefficient outside int64_t");
    const auto small=magnitude.low;
#endif
    if(!negative)return static_cast<std::int64_t>(small);
    if(small==(std::uint64_t{1}<<63))return std::numeric_limits<std::int64_t>::min();
    return -static_cast<std::int64_t>(small);
}
template<std::integral T>
std::uint32_t convolution_normalize(T value,std::uint32_t mod) noexcept {
    static_assert(sizeof(T)<=sizeof(std::uint64_t));
    if constexpr(std::is_signed_v<T>){const auto r=static_cast<std::int64_t>(value)%static_cast<std::int64_t>(mod);return static_cast<std::uint32_t>(r<0?r+mod:r);}
    else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value)%mod);
}
}
#endif

#ifndef SIRKIT_CRT_CONVOLUTION_DEFINED
#define SIRKIT_CRT_CONVOLUTION_DEFINED
class CRTConvolution {
    NTT<167772161> first_;
    NTT<469762049> second_;
    NTT<998244353> third_;
    static std::size_t output_size(std::size_t a,std::size_t b) {
        const auto n=/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_result_size(a,b,"CRTConvolution: size overflow");
        if(n>max_length())throw std::length_error("CRTConvolution: result too long");
        return n;
    }
    template<bool Square,std::integral T,std::integral U>
    std::vector<std::uint32_t> modular_impl(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod) {
        if(mod==0)throw std::invalid_argument("CRTConvolution: zero modulus");
        const auto n=output_size(a.size(),b.size());
        if(!n)return {};
        if(mod==1)return std::vector<std::uint32_t>(n);
        std::vector<std::uint32_t> x; x.reserve(a.size());
        for(const auto v:a)x.push_back(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_normalize(v,mod));
        std::vector<std::uint32_t> y;
        if constexpr(!Square){y.reserve(b.size());for(const auto v:b)y.push_back(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_normalize(v,mod));}
        const auto& right=Square?x:y;
        if(std::min(x.size(),right.size())<=32){
            std::vector<std::uint32_t> out(n);
            for(std::size_t i=0;i<x.size();++i)for(std::size_t j=0;j<right.size();++j)
                out[i+j]=static_cast<std::uint32_t>((std::uint64_t{x[i]}*right[j]+out[i+j])%mod);
            return out;
        }
        auto compute=[&](auto& ntt){if constexpr(Square)return ntt.square(x);else return ntt.convolution(x,y);};
        if(mod==167772161)return compute(first_);
        if(mod==469762049)return compute(second_);
        if(mod==998244353)return compute(third_);
        auto r0=compute(first_),r1=compute(second_),r2=compute(third_);
        const auto factor=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_product01%mod;
        for(std::size_t i=0;i<n;++i){
            const auto digits=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_crt_digits(r0[i],r1[i],r2[i]);
            r0[i]=static_cast<std::uint32_t>((digits.low%mod+factor*(digits.high%mod))%mod);
        }
        return r0;
    }
    template<bool Square>
    std::vector<std::int64_t> integer_impl(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b) {
        const auto n=output_size(a.size(),b.size());
        if(!n)return {};
        auto compute=[&](auto& ntt){if constexpr(Square)return ntt.square(a);else return ntt.convolution(a,b);};
        const auto r0=compute(first_),r1=compute(second_),r2=compute(third_);
        std::vector<std::int64_t> out(n);
        for(std::size_t i=0;i<n;++i)out[i]=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_decode_integer(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_crt_digits(r0[i],r1[i],r2[i]));
        return out;
    }
public:
    static constexpr std::size_t max_length() noexcept {return std::size_t{1}<<23;}
    void prepare(std::size_t length) {
        if(length>max_length()||(length&&!std::has_single_bit(length)))throw std::length_error("CRTConvolution: invalid transform length");
        first_.prepare(length);second_.prepare(length);third_.prepare(length);
    }
    std::size_t roots_capacity() const noexcept {return first_.roots_capacity()+second_.roots_capacity()+third_.roots_capacity();}
    void clear_cache() noexcept {first_.clear_cache();second_.clear_cache();third_.clear_cache();}
    template<std::integral T,std::integral U>
    std::vector<std::uint32_t> modular(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod){return modular_impl<false>(a,b,mod);}
    template<std::integral T>
    std::vector<std::uint32_t> modular_square(const std::vector<T>& a,std::uint32_t mod){return modular_impl<true>(a,a,mod);}
    std::vector<std::int64_t> integer(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b){return integer_impl<false>(a,b);}
    std::vector<std::int64_t> integer_square(const std::vector<std::int64_t>& a){return integer_impl<true>(a,a);}
};
#endif

#ifndef SIRKIT_CONVOLUTION_ANY_MOD_DEFINED
#define SIRKIT_CONVOLUTION_ANY_MOD_DEFINED
template<std::integral T,std::integral U>
std::vector<std::uint32_t> ConvolutionAnyMod(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod){return CRTConvolution{}.modular(a,b,mod);}
#endif
#ifndef SIRKIT_CONVOLUTION_INT64_DEFINED
#define SIRKIT_CONVOLUTION_INT64_DEFINED
inline std::vector<std::int64_t> ConvolutionInt64(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b){return CRTConvolution{}.integer(a,b);}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <array>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cmath>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <numeric>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_POLLARD_RHO_DETAIL_DEFINED
#define SIRKIT_POLLARD_RHO_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
class FactorizationArithmetic {
    std::uint64_t inverse_, r_squared_;
public:
    std::uint64_t modulus, one;
    explicit FactorizationArithmetic(std::uint64_t n) noexcept : modulus(n) {
        std::uint64_t inverse = n;
        for (int i = 0; i < 6; ++i) inverse *= 2 - n * inverse;
        inverse_ = std::uint64_t{0} - inverse;
        const auto residue = std::numeric_limits<std::uint64_t>::max() % n;
        one = residue == n - 1 ? 0 : residue + 1;
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
        using Wide = __uint128_t; // GCC/Clang拡張。標準C++の型ではない。
        r_squared_ = static_cast<std::uint64_t>(static_cast<Wide>(one) * one % n);
#else
        r_squared_ = one;
        for (int i = 0; i < 64; ++i) r_squared_ = add(r_squared_, r_squared_);
#endif
    }
    [[nodiscard]] std::uint64_t add(std::uint64_t a, std::uint64_t b) const noexcept {
        return a >= modulus - b ? a - (modulus - b) : a + b;
    }
    [[nodiscard]] std::uint64_t multiply(std::uint64_t a, std::uint64_t b) const noexcept {
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
        using Wide = __uint128_t;
        const Wide product = static_cast<Wide>(a) * b;
        const auto q = static_cast<std::uint64_t>(product) * inverse_;
        const Wide correction = static_cast<Wide>(q) * modulus;
        // 128bit全体を加算せず、65bitの上位和で129bit目の繰上がりを保持する。
        const Wide upper = (product >> 64) + (correction >> 64) + (static_cast<std::uint64_t>(product) != 0);
        return static_cast<std::uint64_t>(upper >= modulus ? upper - modulus : upper);
#else
        constexpr std::uint64_t mask = (std::uint64_t{1} << 32) - 1;
        const std::uint32_t a0 = static_cast<std::uint32_t>(a), a1 = static_cast<std::uint32_t>(a >> 32);
        const std::uint32_t b0 = static_cast<std::uint32_t>(b), b1 = static_cast<std::uint32_t>(b >> 32);
        const auto p00 = std::uint64_t{a0} * b0;
        auto middle = std::uint64_t{a0} * b1 + (p00 >> 32);
        const auto upper_carry = middle >> 32;
        middle = std::uint64_t{a1} * b0 + (middle & mask);
        const auto high = std::uint64_t{a1} * b1 + upper_carry + (middle >> 32);
        std::array<std::uint32_t, 5> words{static_cast<std::uint32_t>(p00),
            static_cast<std::uint32_t>(middle), static_cast<std::uint32_t>(high),
            static_cast<std::uint32_t>(high >> 32), 0};
        const std::uint32_t mod0 = static_cast<std::uint32_t>(modulus), mod1 = static_cast<std::uint32_t>(modulus >> 32);
        for (std::size_t i = 0; i < 2; ++i) {
            const auto q = static_cast<std::uint32_t>(std::uint64_t{words[i]} * static_cast<std::uint32_t>(inverse_));
            auto sum = std::uint64_t{q} * mod0 + words[i];
            words[i] = static_cast<std::uint32_t>(sum);
            sum = std::uint64_t{q} * mod1 + words[i + 1] + (sum >> 32);
            words[i + 1] = static_cast<std::uint32_t>(sum);
            std::uint64_t carry = sum >> 32;
            for (std::size_t at = i + 2; carry != 0; ++at) {
                const auto combined = std::uint64_t{words[at]} + carry;
                words[at] = static_cast<std::uint32_t>(combined); carry = combined >> 32;
            }
        }
        const auto result = (std::uint64_t{words[3]} << 32) | words[2];
        return words[4] != 0 || result >= modulus ? result - modulus : result;
#endif
    }
    [[nodiscard]] std::uint64_t encode(std::uint64_t value) const noexcept {
        return multiply(value % modulus, r_squared_);
    }
    [[nodiscard]] std::uint64_t power(std::uint64_t value, std::uint64_t exponent) const noexcept {
        auto result = one;
        for (; exponent != 0; exponent >>= 1) {
            if (exponent & 1) result = multiply(result, value);
            value = multiply(value, value);
        }
        return result;
    }
    [[nodiscard]] bool prime() const noexcept {
        const int shifts = std::countr_zero(modulus - 1);
        const auto exponent = (modulus - 1) >> shifts;
        for (std::uint64_t base : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
            if (base % modulus == 0) continue;
            auto value = power(encode(base), exponent);
            if (value == one || value == modulus - one) continue;
            int step = 1;
            for (; step < shifts; ++step) {
                value = multiply(value, value);
                if (value == modulus - one) break;
                if (value == one) return false;
            }
            if (step == shifts) return false;
        }
        return true;
    }
};
inline constexpr std::array<std::uint32_t, 12> factor_small_primes{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
inline std::uint64_t factor_exact_square_root(std::uint64_t value, std::uint64_t root) noexcept {
    while (root != 0 && root > value / root) --root;
    while (root + 1 <= value / (root + 1)) ++root;
    return root;
}
inline std::uint64_t factor_square_root(std::uint64_t value) noexcept {
    return factor_exact_square_root(value, static_cast<std::uint64_t>(std::sqrt(static_cast<long double>(value))));
}
inline std::size_t factor_divisor_capacity(std::size_t count, std::size_t terms, std::size_t limit) {
    if (count > limit / terms) throw std::length_error("PollardRho64: too many divisors");
    return count * terms;
}
}
#endif

#ifndef SIRKIT_POLLARD_RHO64_DEFINED
#define SIRKIT_POLLARD_RHO64_DEFINED
class PollardRho64 {
    std::uint64_t state_;
    std::uint64_t random_below(std::uint64_t upper) noexcept {
        state_ = state_ * std::uint64_t{6364136223846793005} + std::uint64_t{1442695040888963407};
        return state_ % upper;
    }
    std::uint64_t split(const /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FactorizationArithmetic& arithmetic) noexcept {
        const auto n = arithmetic.modulus;
        constexpr std::size_t batch = 128, attempt_budget = std::size_t{1} << 22;
        for (;;) {
            const auto constant = arithmetic.encode(random_below(n - 1) + 1);
            auto moving = arithmetic.encode(random_below(n - 2) + 2);
            const auto advance = [&](std::uint64_t value) { return arithmetic.add(arithmetic.multiply(value, value), constant); };
            std::uint64_t fixed = moving, saved = moving, divisor = 1;
            std::size_t span = 1, spent = 0;
            while (divisor == 1 && span <= (attempt_budget - spent) / 2) {
                fixed = moving;
                for (std::size_t i = 0; i < span; ++i) moving = advance(moving);
                spent += span;
                for (std::size_t offset = 0; offset < span && divisor == 1; offset += batch) {
                    saved = moving;
                    auto product = arithmetic.one;
                    const auto count = std::min(batch, span - offset);
                    for (std::size_t j = 0; j < count; ++j) {
                        moving = advance(moving);
                        const auto difference = fixed >= moving ? fixed - moving : moving - fixed;
                        product = arithmetic.multiply(product, difference);
                    }
                    spent += count;
                    divisor = std::gcd(product, n);
                }
                span <<= 1;
            }
            if (divisor == n) {
                for (std::size_t i = 0; i < batch; ++i) {
                    saved = advance(saved);
                    const auto difference = fixed >= saved ? fixed - saved : saved - fixed;
                    divisor = std::gcd(difference, n);
                    if (divisor > 1) break;
                }
            }
            if (divisor > 1 && divisor < n) return divisor;
        }
    }
    static void require_positive(std::uint64_t n) {
        if (n == 0) throw std::domain_error("PollardRho64: zero has no finite factorization");
    }
public:
    explicit PollardRho64(std::uint64_t seed = 0x8b54e7163d20a9c1ULL) noexcept : state_(seed) {}
    [[nodiscard]] static bool is_prime(std::uint64_t n) noexcept {
        if (n < 2) return false;
        for (auto prime : /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::factor_small_primes)
            if (n % prime == 0) return n == prime;
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FactorizationArithmetic(n).prime();
    }
    std::vector<std::uint64_t> prime_factors(std::uint64_t n) {
        require_positive(n);
        if (n == 1) return {};
        std::vector<std::uint64_t> result; result.reserve(64);
        for (auto prime : /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::factor_small_primes)
            while (n % prime == 0) { result.push_back(prime); n /= prime; }
        if (n == 1) return result;
        std::vector<std::pair<std::uint64_t, int>> pending;
        pending.reserve(64);
        if (n != 1) pending.emplace_back(n, 1);
        while (!pending.empty()) {
            const auto [value, multiplicity] = pending.back(); pending.pop_back();
            const /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FactorizationArithmetic arithmetic(value);
            if (arithmetic.prime()) {
                for (int i = 0; i < multiplicity; ++i) result.push_back(value);
                continue;
            }
            const auto root = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::factor_square_root(value);
            if (root > 1 && root == value / root && value % root == 0) {
                pending.emplace_back(root, multiplicity * 2); continue;
            }
            const auto divisor = split(arithmetic);
            pending.emplace_back(divisor, multiplicity);
            pending.emplace_back(value / divisor, multiplicity);
        }
        std::sort(result.begin(), result.end());
        return result;
    }
    std::vector<std::pair<std::uint64_t, int>> factorize(std::uint64_t n) {
        const auto primes = prime_factors(n);
        std::vector<std::pair<std::uint64_t, int>> result;
        for (auto prime : primes) {
            if (result.empty() || result.back().first != prime) result.emplace_back(prime, 1);
            else ++result.back().second;
        }
        return result;
    }
    std::vector<std::uint64_t> divisors(std::uint64_t n, bool sorted = true) {
        const auto powers = factorize(n);
        std::vector<std::uint64_t> result{1};
        std::size_t count = 1;
        for (const auto& [prime, exponent] : powers) {
            (void)prime;
            count = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::factor_divisor_capacity(
                count, static_cast<std::size_t>(exponent + 1), result.max_size());
        }
        result.reserve(count);
        for (const auto& [prime, exponent] : powers) {
            const auto previous = result.size();
            std::uint64_t power = 1;
            for (int i = 0; i < exponent; ++i) {
                power *= prime;
                for (std::size_t j = 0; j < previous; ++j) result.push_back(result[j] * power);
            }
        }
        if (sorted) std::sort(result.begin(), result.end());
        return result;
    }
    std::uint64_t divisor_count(std::uint64_t n) {
        std::uint64_t result = 1;
        for (const auto& [prime, exponent] : factorize(n)) {
            (void)prime; result *= static_cast<std::uint64_t>(exponent + 1);
        }
        return result;
    }
    std::uint64_t totient(std::uint64_t n) {
        auto result = n;
        for (const auto& [prime, exponent] : factorize(n)) {
            (void)exponent; result = result / prime * (prime - 1);
        }
        return result;
    }
    int mobius(std::uint64_t n) {
        int result = 1;
        for (const auto& [prime, exponent] : factorize(n)) {
            (void)prime;
            if (exponent > 1) return 0;
            result = -result;
        }
        return result;
    }
};
#endif

#ifndef SIRKIT_FACTORIZATION64_FUNCTIONS_DEFINED
#define SIRKIT_FACTORIZATION64_FUNCTIONS_DEFINED
inline std::vector<std::uint64_t> PrimeFactors64(std::uint64_t n) { return PollardRho64{}.prime_factors(n); }
inline std::vector<std::pair<std::uint64_t, int>> Factorize64(std::uint64_t n) { return PollardRho64{}.factorize(n); }
inline std::vector<std::uint64_t> Divisors64(std::uint64_t n, bool sorted = true) { return PollardRho64{}.divisors(n, sorted); }
inline std::uint64_t DivisorCount64(std::uint64_t n) { return PollardRho64{}.divisor_count(n); }
inline std::uint64_t EulerPhi64(std::uint64_t n) { return PollardRho64{}.totient(n); }
inline int Mobius64(std::uint64_t n) { return PollardRho64{}.mobius(n); }
#endif

#ifndef SIRKIT_PRIME_MULTIPLICATIVE_GROUP_DEFINED
#define SIRKIT_PRIME_MULTIPLICATIVE_GROUP_DEFINED
class PrimeMultiplicativeGroup
{
public:
    explicit PrimeMultiplicativeGroup(std::uint64_t prime) : modulus_(prime), factors_(factor_group_size(prime))
    {
    }

    [[nodiscard]] std::uint64_t modulus() const noexcept
    {
        return modulus_;
    }

    [[nodiscard]] std::optional<std::uint64_t> order(std::uint64_t a) const noexcept
    {
        a %= modulus_;
        if (a == 0)
        {
            return std::nullopt;
        }
        auto result = modulus_ - 1;
        if (modulus_ == 2)
        {
            return result;
        }
        const detail::FactorizationArithmetic arithmetic(modulus_);
        const auto encoded = arithmetic.encode(a);
        for (const auto &[factor, exponent] : factors_)
        {
            for (int degree = 0; degree < exponent; ++degree)
            {
                if (arithmetic.power(encoded, result / factor) != arithmetic.one)
                {
                    break;
                }
                result /= factor;
            }
        }
        return result;
    }

    [[nodiscard]] bool is_generator(std::uint64_t a) const noexcept
    {
        a %= modulus_;
        if (a == 0)
        {
            return false;
        }
        if (modulus_ == 2)
        {
            return true;
        }
        return generates(detail::FactorizationArithmetic(modulus_), a);
    }

    [[nodiscard]] std::uint64_t primitive_root() const noexcept
    {
        if (modulus_ == 2)
        {
            return 1;
        }
        const detail::FactorizationArithmetic arithmetic(modulus_);
        std::uint64_t candidate = 2;
        while (!generates(arithmetic, candidate))
        {
            ++candidate;
        }
        return candidate;
    }

private:
    std::uint64_t modulus_;
    std::vector<std::pair<std::uint64_t, int>> factors_;

    static std::vector<std::pair<std::uint64_t, int>> factor_group_size(std::uint64_t prime)
    {
        if (!PollardRho64::is_prime(prime))
        {
            throw std::domain_error("PrimeMultiplicativeGroup: modulus must be prime");
        }
        return Factorize64(prime - 1);
    }

    [[nodiscard]] bool generates(const detail::FactorizationArithmetic &arithmetic, std::uint64_t a) const noexcept
    {
        const auto encoded = arithmetic.encode(a);
        for (const auto &[factor, exponent] : factors_)
        {
            if (arithmetic.power(encoded, (modulus_ - 1) / factor) == arithmetic.one)
            {
                return false;
            }
        }
        return true;
    }
};

[[nodiscard]] inline std::uint64_t primitive_root(std::uint64_t prime)
{
    return PrimeMultiplicativeGroup(prime).primitive_root();
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <span>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_CYCLIC_MULTIVARIATE_CONVOLUTION_DEFINED
#define SIRKIT_CYCLIC_MULTIVARIATE_CONVOLUTION_DEFINED
namespace detail
{
class CyclicPrimeField32
{
public:
    explicit CyclicPrimeField32(std::uint32_t modulus) noexcept : modulus_(modulus)
    {
    }

    [[nodiscard]] std::uint32_t modulus() const noexcept
    {
        return modulus_;
    }

    [[nodiscard]] std::uint32_t add(std::uint32_t a, std::uint32_t b) const noexcept
    {
        const auto sum = std::uint64_t{a} + b;
        return static_cast<std::uint32_t>(sum >= modulus_ ? sum - modulus_ : sum);
    }

    [[nodiscard]] std::uint32_t subtract(std::uint32_t a, std::uint32_t b) const noexcept
    {
        return a >= b ? a - b : static_cast<std::uint32_t>(std::uint64_t{a} + modulus_ - b);
    }

    [[nodiscard]] std::uint32_t multiply(std::uint32_t a, std::uint32_t b) const noexcept
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % modulus_);
    }

    [[nodiscard]] std::uint32_t power(std::uint32_t a, std::uint32_t exponent) const noexcept
    {
        std::uint32_t result = 1;
        for (; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1U) != 0)
            {
                result = multiply(result, a);
            }
            a = multiply(a, a);
        }
        return result;
    }

private:
    std::uint32_t modulus_;
};

class CyclicAxisTransform
{
    using Coefficients = std::vector<std::uint32_t>;
    static constexpr std::size_t kNaiveLength = 16;

    struct Chirp
    {
        Coefficients factors;
        Coefficients kernel;
    };

public:
    CyclicAxisTransform(std::size_t length, std::uint32_t generator, CyclicPrimeField32 field)
        : field_(field), length_(length)
    {
        const auto modulus = field_.modulus();
        const auto root = field_.power(generator, static_cast<std::uint32_t>((modulus - 1) / length));
        const auto inverse = field_.power(root, modulus - 2);
        if (length > kNaiveLength && !std::has_single_bit(length))
        {
            forward_chirp_ = MakeChirp(root, inverse);
            inverse_chirp_ = MakeChirp(inverse, root);
        }
        else
        {
            roots_ = MakeRoots(root);
            inverse_roots_ = MakeRoots(inverse);
        }
    }

    void apply(Coefficients &values, std::size_t stride, bool inverse)
    {
        if (length_ == 2)
        {
            for (std::size_t base = 0; base < values.size(); base += 2 * stride)
            {
                for (std::size_t i = base; i < base + stride; ++i)
                {
                    const auto a = values[i], b = values[i + stride];
                    values[i] = field_.add(a, b);
                    values[i + stride] = field_.subtract(a, b);
                }
            }
            return;
        }
        Coefficients line(length_), scratch(length_);
        for (std::size_t base = 0; base < values.size(); base += stride * length_)
        {
            for (std::size_t offset = 0; offset < stride; ++offset)
            {
                const auto start = base + offset;
                for (std::size_t i = 0; i < length_; ++i)
                {
                    line[i] = values[start + i * stride];
                }
                if (length_ <= kNaiveLength)
                {
                    Naive(line, scratch, inverse ? inverse_roots_ : roots_);
                    line.swap(scratch);
                }
                else if (std::has_single_bit(length_))
                {
                    RadixTwo(line, inverse ? inverse_roots_ : roots_);
                }
                else
                {
                    Bluestein(line, inverse ? inverse_chirp_ : forward_chirp_);
                }
                for (std::size_t i = 0; i < length_; ++i)
                {
                    values[start + i * stride] = line[i];
                }
            }
        }
    }

private:
    CyclicPrimeField32 field_;
    std::size_t length_;
    Coefficients roots_, inverse_roots_;
    Chirp forward_chirp_, inverse_chirp_;
    CRTConvolution convolution_;

    [[nodiscard]] Coefficients MakeRoots(std::uint32_t root) const
    {
        Coefficients roots(length_, 1);
        for (std::size_t i = 1; i < length_; ++i)
        {
            roots[i] = field_.multiply(roots[i - 1], root);
        }
        return roots;
    }

    [[nodiscard]] Chirp MakeChirp(std::uint32_t root, std::uint32_t inverse) const
    {
        Chirp result{Coefficients(length_), Coefficients(2 * length_ - 1)};
        const auto center = length_ - 1;
        std::uint32_t positive = 1, negative = 1, step = 1, inverse_step = 1;
        for (std::size_t i = 0; i < length_; ++i)
        {
            result.kernel[center - i] = negative;
            positive = field_.multiply(positive, step);
            negative = field_.multiply(negative, inverse_step);
            result.kernel[center + i] = negative;
            result.factors[i] = positive;
            step = field_.multiply(step, root);
            inverse_step = field_.multiply(inverse_step, inverse);
        }
        return result;
    }

    void Naive(const Coefficients &input, Coefficients &output, const Coefficients &roots) const noexcept
    {
        for (std::size_t k = 0; k < length_; ++k)
        {
            std::uint32_t sum = 0;
            for (std::size_t j = 0; j < length_; ++j)
            {
                sum = field_.add(sum, field_.multiply(input[j], roots[j * k % length_]));
            }
            output[k] = sum;
        }
    }

    void RadixTwo(Coefficients &values, const Coefficients &roots) const noexcept
    {
        for (std::size_t i = 1, j = 0; i < length_; ++i)
        {
            auto step = length_ / 2;
            for (; (j & step) != 0; step /= 2)
            {
                j ^= step;
            }
            j ^= step;
            if (i < j)
            {
                std::swap(values[i], values[j]);
            }
        }
        for (std::size_t block = 2; block <= length_; block *= 2)
        {
            const auto half = block / 2;
            const auto step = length_ / block;
            for (std::size_t base = 0; base < length_; base += block)
            {
                for (std::size_t j = 0; j < half; ++j)
                {
                    const auto a = values[base + j];
                    const auto b = field_.multiply(values[base + half + j], roots[j * step]);
                    values[base + j] = field_.add(a, b);
                    values[base + half + j] = field_.subtract(a, b);
                }
            }
        }
    }

    void Bluestein(Coefficients &line, const Chirp &chirp)
    {
        // jk = j(j-1)/2 + k(k+1)/2 - (j-k)(j-k-1)/2。
        for (std::size_t i = 1; i < length_; ++i)
        {
            line[i] = field_.multiply(line[i], chirp.factors[i - 1]);
        }
        const auto linear = convolution_.modular(line, chirp.kernel, field_.modulus());
        for (std::size_t i = 0; i < length_; ++i)
        {
            line[i] = field_.multiply(linear[length_ - 1 + i], chirp.factors[i]);
        }
    }
};

inline std::vector<std::uint32_t> CyclicMultivariateConvolution(std::span<const std::size_t> dimensions,
                                                                std::span<const std::uint32_t> a,
                                                                std::span<const std::uint32_t> b,
                                                                std::uint32_t modulus)
{
    const auto generator = static_cast<std::uint32_t>(primitive_root(modulus));
    const CyclicPrimeField32 field(modulus);
    constexpr std::size_t kMaxSize = std::size_t{1} << 21;
    std::size_t size = 1;
    std::vector<std::size_t> strides;
    std::vector<CyclicAxisTransform> transforms;
    for (const auto length : dimensions)
    {
        if (length == 0)
        {
            throw std::invalid_argument("cyclic_multivariate_convolution: zero dimension");
        }
        if (length > kMaxSize / size)
        {
            throw std::length_error("cyclic_multivariate_convolution: total size exceeds 2^21");
        }
#ifndef SKIP_RUNTIME_CHECKS
        if ((modulus - 1) % length != 0)
        {
            throw std::invalid_argument("cyclic_multivariate_convolution: axis must divide prime-1");
        }
#endif
        if (length > 1)
        {
            strides.push_back(size);
            transforms.emplace_back(length, generator, field);
            size *= length;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (a.size() != size || b.size() != size)
    {
        throw std::invalid_argument("cyclic_multivariate_convolution: input counts do not match");
    }
#endif
    std::vector<std::uint32_t> left(size), right(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        left[i] = a[i] % modulus;
        right[i] = b[i] % modulus;
    }
    for (std::size_t axis = 0; axis < transforms.size(); ++axis)
    {
        transforms[axis].apply(left, strides[axis], false);
        transforms[axis].apply(right, strides[axis], false);
    }
    for (std::size_t i = 0; i < size; ++i)
    {
        left[i] = field.multiply(left[i], right[i]);
    }
    for (std::size_t axis = 0; axis < transforms.size(); ++axis)
    {
        transforms[axis].apply(left, strides[axis], true);
    }
    const auto inverse_size = field.power(static_cast<std::uint32_t>(size % modulus), modulus - 2);
    for (auto &entry : left)
    {
        entry = field.multiply(entry, inverse_size);
    }
    return left;
}
}

inline std::vector<std::uint32_t> cyclic_multivariate_convolution(std::span<const std::size_t> dimensions,
                                                                  std::span<const std::uint32_t> a,
                                                                  std::span<const std::uint32_t> b,
                                                                  std::uint32_t prime)
{
    return detail::CyclicMultivariateConvolution(dimensions, a, b, prime);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
