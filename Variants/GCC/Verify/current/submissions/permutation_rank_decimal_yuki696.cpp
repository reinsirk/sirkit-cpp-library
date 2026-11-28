#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/696

// Library: Math/Mixed_Radix.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Mixed_Radix.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/mixed_radix.md
//
// API一覧
// [MixedRadix<Radix=10>]
// MixedRadix(radices): 正のuint32_tを上位桁から渡す。1も可。桁数n、全基数の積の桁数D。
// capacity(): 表現可能な整数の個数、基数の積のconst参照。O(1)。
// encode(digits): 同じ長さ・0<=digits[i]<radices[i]の桁をBigIntegerへ。decode(value): 桁へ戻す。
// encodeと構築はO(n + D log(D+1)log(n+1))時間。decodeはO(n + D log²(D+1)log(n+1))。
// 積を保持する領域はO(n+D log(n+1))。整数の範囲は0..capacity()-1。RadixはBigIntegerの基数。
//
// [同梱の多倍長整数]
// [BigInteger<Radix=10>]
// Radixは10か16。BigInteger(value=0): int64_tから構築。BigInteger(text): 符号付きの文字列から構築。
// to_string(): 先頭ゼロのない文字列。16進の英字は大文字。sign(): -1、0、1。
// +,-,*,+=,-=,*=、比較、単項-: 符号付きの整数演算。桁数Dについて比較・加減・変換はO(D)時間。
// 乗算は内部の4桁ブロック数A,BについてO(A*B)またはO((A+B)log(A+B))時間、O(A+B)領域。
// NTTが必要な場合の変換長は2^23以下。整数係数を2種類のNTTから復元する。
// divmod(divisor): 正のuint32_tで割った商とint64_tの余り。商は0へ丸める。O(D)時間・領域。
// divmod(BigInteger)、/,%,/=,%=: 非零のBigIntegerで割る。商は0へ丸め、余りは元の整数と同符号か0。
// 4桁ブロック数を被除数A、除数Bとして、除算はO(A log^2(B+1))時間・O(A+B)領域。NTTの制約は乗算と共通。
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
#include <compare>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <string>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <string_view>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_BIG_INTEGER_DEFINED
#define SIRKIT_BIG_INTEGER_DEFINED
template <unsigned Radix = 10>
class BigInteger
{
    static_assert(Radix == 10 || Radix == 16);
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using i64 = std::int64_t;

    static constexpr u32 kBlockSize = 4;
    static constexpr u32 kBase = Radix * Radix * Radix * Radix;
    static constexpr u32 kFirstPrime = 998244353;
    static constexpr u32 kSecondPrime = 469762049;
    static constexpr u32 kFirstInverseModuloSecond = 208783132;
    static constexpr std::string_view kDigits = "0123456789ABCDEF";
    static constexpr std::size_t kDivisionLeafSize = 64;

    std::vector<u32> limbs_;
    bool negative_ = false;

    void trim() noexcept
    {
        while (!limbs_.empty() && limbs_.back() == 0)
        {
            limbs_.pop_back();
        }
        if (limbs_.empty())
        {
            negative_ = false;
        }
    }

    static unsigned parse_digit(char character) noexcept
    {
        if ('0' <= character && character <= '9')
        {
            return static_cast<unsigned>(character - '0');
        }
        if constexpr (Radix == 16)
        {
            if ('A' <= character && character <= 'F')
            {
                return 10u + static_cast<unsigned>(character - 'A');
            }
            if ('a' <= character && character <= 'f')
            {
                return 10u + static_cast<unsigned>(character - 'a');
            }
        }
        return Radix;
    }

    static int magnitude_order(const BigInteger &left, const BigInteger &right) noexcept
    {
        if (left.limbs_.size() != right.limbs_.size())
        {
            return left.limbs_.size() < right.limbs_.size() ? -1 : 1;
        }
        for (std::size_t i = left.limbs_.size(); i != 0; --i)
        {
            const auto a = left.limbs_[i - 1];
            const auto b = right.limbs_[i - 1];
            if (a != b)
            {
                return a < b ? -1 : 1;
            }
        }
        return 0;
    }

    static BigInteger add_or_subtract(const BigInteger &left, const BigInteger &right, bool subtract)
    {
        const bool right_negative = right.negative_ != subtract;
        if (left.limbs_.empty())
        {
            auto out = right;
            out.negative_ = !out.limbs_.empty() && right_negative;
            return out;
        }
        if (right.limbs_.empty())
        {
            return left;
        }
        BigInteger out;
        if (left.negative_ == right_negative)
        {
            const auto count = std::max(left.limbs_.size(), right.limbs_.size());
            out.limbs_.resize(count);
            out.negative_ = left.negative_;
            u32 carry = 0;
            for (std::size_t i = 0; i < count; ++i)
            {
                const u32 a = i < left.limbs_.size() ? left.limbs_[i] : 0;
                const u32 b = i < right.limbs_.size() ? right.limbs_[i] : 0;
                const u32 sum = a + b + carry;
                carry = sum >= kBase;
                out.limbs_[i] = sum - carry * kBase;
            }
            if (carry != 0)
            {
                out.limbs_.push_back(carry);
            }
            return out;
        }

        const auto order = magnitude_order(left, right);
        if (order == 0)
        {
            return {};
        }
        const auto &bigger = order > 0 ? left.limbs_ : right.limbs_;
        const auto &smaller = order > 0 ? right.limbs_ : left.limbs_;
        out.limbs_.resize(bigger.size());
        out.negative_ = order > 0 ? left.negative_ : right_negative;
        std::int32_t carry = 0;
        for (std::size_t i = 0; i < bigger.size(); ++i)
        {
            const auto sub = i < smaller.size() ? smaller[i] : 0;
            const auto value = static_cast<std::int32_t>(bigger[i]) - static_cast<std::int32_t>(sub) - carry;
            carry = value < 0;
            out.limbs_[i] = static_cast<u32>(value + carry * static_cast<std::int32_t>(kBase));
        }
        out.trim();
        return out;
    }

    static u64 restore_coefficient(u32 first, u32 second) noexcept
    {
        const u32 reduced = first % kSecondPrime;
        const u32 delta = second >= reduced ? second - reduced : second + kSecondPrime - reduced;
        const u64 lifted = static_cast<u64>(delta) * kFirstInverseModuloSecond % kSecondPrime;
        return first + static_cast<u64>(kFirstPrime) * lifted;
    }

    template <class Coefficient>
    static std::vector<u32> carry_digits(std::size_t count, const Coefficient &coefficient)
    {
        std::vector<u32> result(count);
        u64 carry = 0;
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto value = coefficient(i) + carry;
            result[i] = static_cast<u32>(value % kBase);
            carry = value / kBase;
        }
        while (carry != 0)
        {
            result.push_back(static_cast<u32>(carry % kBase));
            carry /= kBase;
        }
        return result;
    }

    static std::vector<u32> multiply_magnitudes(const std::vector<u32> &left, const std::vector<u32> &right)
    {
        const auto size = left.size() + right.size() - 1;
        if (std::min(left.size(), right.size()) <= 32)
        {
            std::vector<u64> coefficients(size);
            for (std::size_t i = 0; i < left.size(); ++i)
            {
                for (std::size_t j = 0; j < right.size(); ++j)
                {
                    coefficients[i + j] += static_cast<u64>(left[i]) * right[j];
                }
            }
            return carry_digits(size, [&](std::size_t i)
                                {
                                    return coefficients[i];
                                });
        }

        // 係数の上限は min(A,B)*(65536-1)^2。変換長の上限内なら
        // 2法の積より小さく、CRTで整数として一意に復元できる。
        const auto first = NTT<kFirstPrime, 3>{}.convolution_normalized(left, right);
        const auto second = NTT<kSecondPrime, 3>{}.convolution_normalized(left, right);
        return carry_digits(size, [&](std::size_t i)
                            {
                                return restore_coefficient(first[i], second[i]);
                            });
    }

    static BigInteger slice_limbs(const BigInteger &value, std::size_t first, std::size_t count)
    {
        BigInteger result;
        if (first < value.limbs_.size())
        {
            const auto length = std::min(count, value.limbs_.size() - first);
            const auto begin = value.limbs_.begin() + static_cast<std::ptrdiff_t>(first);
            result.limbs_.assign(begin, begin + static_cast<std::ptrdiff_t>(length));
            result.trim();
        }
        return result;
    }

    static BigInteger join_limbs(const BigInteger &high, const BigInteger &low, std::size_t offset)
    {
        if (high.limbs_.empty())
        {
            return low;
        }
        BigInteger result;
        result.limbs_.resize(offset + high.limbs_.size());
        std::copy(low.limbs_.begin(), low.limbs_.end(), result.limbs_.begin());
        std::copy(high.limbs_.begin(), high.limbs_.end(), result.limbs_.begin() + static_cast<std::ptrdiff_t>(offset));
        return result;
    }

    static void scale_limbs(BigInteger &value, u32 factor)
    {
        u64 carry = 0;
        for (auto &limb : value.limbs_)
        {
            const auto scaled = static_cast<u64>(limb) * factor + carry;
            limb = static_cast<u32>(scaled % kBase);
            carry = scaled / kBase;
        }
        if (carry != 0)
        {
            value.limbs_.push_back(static_cast<u32>(carry));
        }
    }

    // 除数の最上位桁はkBase/2以上。上位2桁で商を見積もり、引き過ぎを補正する。
    static std::pair<BigInteger, BigInteger> divide_normalized(const BigInteger &dividend, const BigInteger &divisor)
    {
        BigInteger quotient;
        BigInteger remainder = dividend;
        const auto m = divisor.limbs_.size();
        const auto &b = divisor.limbs_;
        auto &a = remainder.limbs_;
        const auto size = a.size() - m + 1;
        a.push_back(0);
        quotient.limbs_.resize(size);
        for (auto next = size; next != 0; --next)
        {
            const auto offset = next - 1;
            const auto upper = static_cast<u64>(a[offset + m]) * kBase + a[offset + m - 1];
            auto digit = std::min(upper / b[m - 1], u64{kBase - 1});
            auto estimate_remainder = upper - digit * b[m - 1];
            while (digit * b[m - 2] > estimate_remainder * kBase + a[offset + m - 2])
            {
                --digit;
                estimate_remainder += b[m - 1];
            }

            u64 carry = 0;
            for (std::size_t j = 0; j < m; ++j)
            {
                const auto product = digit * b[j] + carry;
                const auto unit = static_cast<u32>(product % kBase);
                const auto borrowed = a[offset + j] < unit;
                a[offset + j] = a[offset + j] + static_cast<u32>(borrowed) * kBase - unit;
                carry = product / kBase + static_cast<u32>(borrowed);
            }
            if (a[offset + m] < carry)
            {
                --digit;
                u32 restored_carry = 0;
                for (std::size_t j = 0; j < m; ++j)
                {
                    const auto sum = a[offset + j] + b[j] + restored_carry;
                    restored_carry = sum >= kBase;
                    a[offset + j] = sum - restored_carry * kBase;
                }
            }
            a[offset + m] = 0;
            quotient.limbs_[offset] = static_cast<u32>(digit);
        }
        quotient.trim();
        remainder.trim();
        return {std::move(quotient), std::move(remainder)};
    }

    // Burnikel–Ziegler: 正規化済みの除数2ブロックで、被除数3ブロックを割る。
    static std::pair<BigInteger, BigInteger> divide_three_by_two(
        const BigInteger &dividend, const BigInteger &divisor,
        const BigInteger &high, const BigInteger &low, std::size_t block)
    {
        BigInteger quotient;
        BigInteger remainder;
        const auto high_dividend = slice_limbs(dividend, 2 * block, block);
        if (magnitude_order(high_dividend, high) < 0)
        {
            auto [q, r] = divide_two_by_one(slice_limbs(dividend, block, 2 * block), high);
            quotient = std::move(q);
            remainder = std::move(r);
        }
        else
        {
            quotient.limbs_.assign(block, kBase - 1);
            remainder = slice_limbs(dividend, block, block) + high;
        }
        remainder = join_limbs(remainder, slice_limbs(dividend, 0, block), block) - quotient * low;
        while (remainder.negative_)
        {
            quotient -= BigInteger(1);
            remainder += divisor;
        }
        return {std::move(quotient), std::move(remainder)};
    }

    static std::pair<BigInteger, BigInteger> divide_two_by_one(const BigInteger &dividend, const BigInteger &divisor)
    {
        if (dividend.limbs_.size() < divisor.limbs_.size())
        {
            return {BigInteger{}, dividend};
        }
        if (divisor.limbs_.size() <= kDivisionLeafSize)
        {
            return divide_normalized(dividend, divisor);
        }

        const auto block = divisor.limbs_.size() / 2;
        const auto high = slice_limbs(divisor, block, block);
        const auto low = slice_limbs(divisor, 0, block);
        const auto [leading, kept] = divide_three_by_two(
            slice_limbs(dividend, block, 3 * block), divisor, high, low, block);
        auto [trailing, remainder] = divide_three_by_two(
            join_limbs(kept, slice_limbs(dividend, 0, block), block), divisor, high, low, block);
        return {join_limbs(leading, trailing, block), std::move(remainder)};
    }

    static std::pair<BigInteger, BigInteger> divide_blocks(const BigInteger &dividend, const BigInteger &divisor)
    {
        const auto block = divisor.limbs_.size();
        const auto chunks = (dividend.limbs_.size() - 1) / block + 1;
        BigInteger quotient;
        quotient.limbs_.resize(dividend.limbs_.size());
        BigInteger remainder;
        for (auto index = chunks; index != 0; --index)
        {
            const auto first = (index - 1) * block;
            const auto active = join_limbs(remainder, slice_limbs(dividend, first, block), block);
            auto [digits, kept] = divide_two_by_one(active, divisor);
            std::copy(digits.limbs_.begin(), digits.limbs_.end(),
                      quotient.limbs_.begin() + static_cast<std::ptrdiff_t>(first));
            remainder = std::move(kept);
        }
        quotient.trim();
        return {std::move(quotient), std::move(remainder)};
    }

public:
    BigInteger(i64 value = 0) : negative_(value < 0)
    {
        const auto bits = static_cast<u64>(value);
        for (auto magnitude = negative_ ? u64{0} - bits : bits; magnitude > 0; magnitude /= kBase)
        {
            limbs_.push_back(static_cast<u32>(magnitude % kBase));
        }
    }

    explicit BigInteger(std::string_view text)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (text.empty())
        {
            throw std::invalid_argument("BigInteger: empty string");
        }
#endif
        negative_ = text.front() == '-';
        const std::size_t first = negative_ || text.front() == '+' ? 1 : 0;
#ifndef SKIP_RUNTIME_CHECKS
        if (first == text.size())
        {
            throw std::invalid_argument("BigInteger: digits required");
        }
#endif
        limbs_.reserve((text.size() - first) / kBlockSize + 1);
        for (auto end = text.size(); end > first;)
        {
            const auto start = end - first < kBlockSize ? first : end - kBlockSize;
            u32 value = 0;
            for (auto i = start; i < end; ++i)
            {
                const auto digit = parse_digit(text[i]);
#ifndef SKIP_RUNTIME_CHECKS
                if (digit >= Radix)
                {
                    throw std::invalid_argument("BigInteger: invalid digit");
                }
#endif
                value = value * Radix + digit;
            }
            limbs_.push_back(value);
            end = start;
        }
        trim();
    }

    [[nodiscard]] int sign() const noexcept
    {
        return limbs_.empty() ? 0 : (negative_ ? -1 : 1);
    }

    [[nodiscard]] std::string to_string() const
    {
        if (limbs_.empty())
        {
            return "0";
        }
        std::string text;
        text.reserve(kBlockSize * limbs_.size() + static_cast<std::size_t>(negative_));
        if (negative_)
        {
            text.push_back('-');
        }
        for (std::size_t i = limbs_.size(); i != 0; --i)
        {
            std::array<char, kBlockSize> block{};
            auto value = limbs_[i - 1];
            for (auto it = block.rbegin(); it != block.rend(); ++it)
            {
                *it = kDigits[value % Radix];
                value /= Radix;
            }
            auto first = block.begin();
            if (i == limbs_.size())
            {
                while (*first == '0')
                {
                    ++first;
                }
            }
            text.append(first, block.end());
        }
        return text;
    }

    [[nodiscard]] BigInteger operator-() const
    {
        auto result = *this;
        result.negative_ = !result.limbs_.empty() && !result.negative_;
        return result;
    }

    friend std::strong_ordering operator<=>(const BigInteger &left, const BigInteger &right) noexcept
    {
        const auto left_sign = left.sign();
        const auto right_sign = right.sign();
        if (left_sign != right_sign)
        {
            return left_sign <=> right_sign;
        }
        return (left_sign * magnitude_order(left, right)) <=> 0;
    }

    friend bool operator==(const BigInteger &left, const BigInteger &right) noexcept
    {
        return (left <=> right) == std::strong_ordering::equal;
    }

    friend BigInteger operator+(const BigInteger &left, const BigInteger &right)
    {
        return add_or_subtract(left, right, false);
    }

    friend BigInteger operator-(const BigInteger &left, const BigInteger &right)
    {
        return add_or_subtract(left, right, true);
    }

    friend BigInteger operator*(const BigInteger &left, const BigInteger &right)
    {
        BigInteger result;
        if (left.limbs_.empty() || right.limbs_.empty())
        {
            return result;
        }
        result.limbs_ = multiply_magnitudes(left.limbs_, right.limbs_);
        result.negative_ = left.negative_ != right.negative_;
        result.trim();
        return result;
    }

    BigInteger &operator+=(const BigInteger &other)
    {
        return *this = *this + other;
    }

    BigInteger &operator-=(const BigInteger &other)
    {
        return *this = *this - other;
    }

    BigInteger &operator*=(const BigInteger &other)
    {
        return *this = *this * other;
    }

    [[nodiscard]] std::pair<BigInteger, i64> divmod(u32 divisor) const
    {
        if (divisor == 0)
        {
            throw std::domain_error("BigInteger: division by zero");
        }
        auto result = *this;
        u64 remainder = 0;
        for (std::size_t i = limbs_.size(); i != 0; --i)
        {
            const auto value = remainder * kBase + limbs_[i - 1];
            result.limbs_[i - 1] = static_cast<u32>(value / divisor);
            remainder = value % divisor;
        }
        result.trim();
        return {std::move(result), static_cast<i64>(remainder) * sign()};
    }

    [[nodiscard]] std::pair<BigInteger, BigInteger> divmod(const BigInteger &divisor) const
    {
        if (divisor.limbs_.empty())
        {
            throw std::domain_error("BigInteger: division by zero");
        }
        if (magnitude_order(*this, divisor) < 0)
        {
            return {BigInteger{}, *this};
        }
        const auto size = divisor.limbs_.size();
        if (size == 1)
        {
            auto [quotient, remainder] = divmod(divisor.limbs_.front());
            quotient.negative_ = negative_ != divisor.negative_;
            return {std::move(quotient), BigInteger(remainder)};
        }

        const bool recursive = size > kDivisionLeafSize && limbs_.size() - size >= 16;
        const auto padding = recursive ? std::bit_ceil(size) - size : 0;
        auto dividend = join_limbs(*this, BigInteger{}, padding);
        auto normalized = join_limbs(divisor, BigInteger{}, padding);
        const auto factor = kBase / (normalized.limbs_.back() + 1);
        if (factor != 1)
        {
            scale_limbs(dividend, factor);
            scale_limbs(normalized, factor);
        }
        auto [quotient, remainder] = recursive ? divide_blocks(dividend, normalized)
                                               : divide_normalized(dividend, normalized);
        remainder = slice_limbs(remainder, padding, remainder.limbs_.size());
        if (factor != 1)
        {
            remainder = remainder.divmod(factor).first;
        }
        quotient.negative_ = negative_ != divisor.negative_;
        remainder.negative_ = !remainder.limbs_.empty() && negative_;
        return {std::move(quotient), std::move(remainder)};
    }

    friend BigInteger operator/(const BigInteger &left, const BigInteger &right)
    {
        return left.divmod(right).first;
    }

    friend BigInteger operator%(const BigInteger &left, const BigInteger &right)
    {
        return left.divmod(right).second;
    }

    BigInteger &operator/=(const BigInteger &other)
    {
        return *this = *this / other;
    }

    BigInteger &operator%=(const BigInteger &other)
    {
        return *this = *this % other;
    }
};
#endif

// SPDX-License-Identifier: CC0-1.0

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

#ifndef SIRKIT_MIXED_RADIX_DEFINED
#define SIRKIT_MIXED_RADIX_DEFINED
template <unsigned Radix = 10>
class MixedRadix
{
    using Int = BigInteger<Radix>;
    static constexpr std::size_t kLeafSize = 24;

    std::vector<std::uint32_t> radices_;
    std::vector<Int> products_;

    void build(std::size_t node, std::size_t begin, std::size_t end)
    {
        if (end - begin <= kLeafSize)
        {
            Int product{1};
            for (auto i = begin; i < end; ++i)
            {
                product *= Int{radices_[i]};
            }
            products_[node] = std::move(product);
            return;
        }
        const auto middle = begin + (end - begin) / 2;
        build(2 * node, begin, middle);
        build(2 * node + 1, middle, end);
        products_[node] = products_[2 * node] * products_[2 * node + 1];
    }

    [[nodiscard]] Int encode_tree(std::size_t node, std::size_t begin, std::size_t end,
                                  std::span<const std::uint32_t> digits) const
    {
        if (end - begin <= kLeafSize)
        {
            Int value;
            for (auto i = begin; i < end; ++i)
            {
                value = value * Int{radices_[i]} + Int{digits[i]};
            }
            return value;
        }
        const auto middle = begin + (end - begin) / 2;
        auto high = encode_tree(2 * node, begin, middle, digits);
        auto low = encode_tree(2 * node + 1, middle, end, digits);
        return high * products_[2 * node + 1] + low;
    }

    void decode_tree(std::size_t node, std::size_t begin, std::size_t end, Int value,
                     std::span<std::uint32_t> digits) const
    {
        if (value.sign() == 0)
        {
            return;
        }
        if (end - begin <= kLeafSize)
        {
            for (auto i = end; i > begin; --i)
            {
                auto [quotient, remainder] = value.divmod(radices_[i - 1]);
                digits[i - 1] = static_cast<std::uint32_t>(remainder);
                value = std::move(quotient);
            }
            return;
        }
        const auto middle = begin + (end - begin) / 2;
        auto [high, low] = value.divmod(products_[2 * node + 1]);
        decode_tree(2 * node, begin, middle, std::move(high), digits);
        decode_tree(2 * node + 1, middle, end, std::move(low), digits);
    }

public:
    explicit MixedRadix(std::span<const std::uint32_t> radices)
        : radices_(radices.begin(), radices.end()),
          products_(2 * std::bit_ceil(radices.size() / kLeafSize + static_cast<std::size_t>(radices.size() % kLeafSize != 0)))
    {
        if (std::ranges::find(radices_, 0u) != radices_.end())
        {
            throw std::invalid_argument("MixedRadix: positive digit radices required");
        }
        build(1, 0, radices_.size());
    }

    [[nodiscard]] const Int &capacity() const noexcept
    {
        return products_[1];
    }

    [[nodiscard]] Int encode(std::span<const std::uint32_t> digits) const
    {
        if (digits.size() != radices_.size())
        {
            throw std::invalid_argument("MixedRadix: digit count differs");
        }
        for (std::size_t i = 0; i < digits.size(); ++i)
        {
            if (digits[i] >= radices_[i])
            {
                throw std::invalid_argument("MixedRadix: digit exceeds radix");
            }
        }
        return encode_tree(1, 0, radices_.size(), digits);
    }

    [[nodiscard]] std::vector<std::uint32_t> decode(Int value) const
    {
        if (value.sign() < 0 || value >= capacity())
        {
            throw std::out_of_range("MixedRadix: integer exceeds representable range");
        }
        std::vector<std::uint32_t> digits(radices_.size());
        decode_tree(1, 0, digits.size(), std::move(value), digits);
        return digits;
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


// Library: Algorithm/Permutation_Order.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Permutation_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/permutation_order.md
//
// API一覧
// permutation_lehmer_code(p): p[i]より小さく、iより後ろにある要素の個数を各桁として返す。
// permutation_from_lehmer_code(code): 各桁0<=code[i]<n-iから順列を復元する。
// permutation_rank(p): 0始まりの辞書順の順位。uint64_tに収まらなければnullopt。
// permutation_unrank(n,rank): 長さnで指定順位の順列。rank>=n!ならnullopt。
// 順列は0..n-1を一度ずつ含む。空順列の順位は0。各O(n log(n+1)+1)時間・O(n+1)領域。
//
// [同梱のFenwick木]
// [fenwick_tree<T>]
// fenwick_tree()/fenwick_tree(int n)/fenwick_tree(vector<T>): 空列/n要素の0/列から構築。O(n+1)時間・領域。
// add(int p,T x): 点加算。sum(int l,int r): 半開区間[l,r)の和をT値で返す。O(log(n+1))、空区間はT{}。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()。
// ACL形式の公開APIをsirkit内に定義する。bool以外の整数は内部を符号なしにして型幅のmod 2^bitで計算する。
// その他のTはT{}が単位元の可換加法群とし、加減算・コピーをO(1)とする。要素数はintの範囲内。
// size()/empty(): 要素数/空か、O(1)。get(int p)/set(int p,T value): 点取得/代入、O(log(n+1))。
// lower_bound(target)/upper_bound(target): sum(0,i+1)>=target/>targetとなる最小int添字、なければsize()。O(log(n+1))。
// 境界探索は全要素が非負で、途中の和がTに収まり、加算と順序が整合すること。全要素の検査はしない。
// values(): 列を復元して返す。O(n+1)時間・追加O(n)。reset(int n)/reset(): n要素/現在の長さを0にする。O(旧n+n+1)。
// clear()/swap(other): 空にして容量保持/交換。O(旧n+1)/O(1)。コピーは独立、移動元は空として使える。
// 添字・区間は既定で検査しSKIP_RUNTIME_CHECKSで除く。容量・添字のオーバーフロー対策は残す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <cstdint>
#include <optional>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldFenwickAccess; }
#endif

#ifndef SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
#define SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T, bool>>
struct FenwickStorage { using type = T; };
template<class T>
struct FenwickStorage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct FenwickStorage<__int128> { using type = unsigned __int128; };
template<> struct FenwickStorage<unsigned __int128> { using type = unsigned __int128; };
#endif
}
#endif

#ifndef SIRKIT_FENWICK_TREE_DEFINED
#define SIRKIT_FENWICK_TREE_DEFINED
namespace detail {
inline std::size_t CheckedFenwickStorageSize(std::size_t n, std::size_t limit) {
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()) || n >= limit)
        throw std::length_error("fenwick_tree: size");
    return n == 0 ? 0 : n + 1;
}
}

template<class T>
class fenwick_tree {
    // HLD所有時の読取り専用接続。通常のFenwick操作には使わない。
    using HldFenwickTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldFenwickAccess;
    using Storage = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FenwickStorage<T>::type;
    std::vector<Storage> tree_;
    static decltype(auto) to_storage(const T& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<Storage>(value);
    }
    static decltype(auto) from_storage(Storage& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<T>(value);
    }
    static std::size_t checked_size(int n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("fenwick_tree: negative size");
#endif
        return static_cast<std::size_t>(n);
    }
    static std::size_t storage_size(std::size_t n) {
        return detail::CheckedFenwickStorageSize(n, std::vector<Storage>{}.max_size());
    }
    static std::size_t lowbit(std::size_t x) noexcept { return x & (~x + 1); }
    void check_index(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i < 0 || i >= size()) throw std::out_of_range("fenwick_tree: index");
#else
        (void)i;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size()) throw std::out_of_range("fenwick_tree: range");
#else
        (void)l; (void)r;
#endif
    }
    Storage prefix_unchecked(std::size_t r) const {
        Storage result{};
        for (; r; r -= lowbit(r)) result += tree_[r];
        return result;
    }
    Storage get_unchecked(std::size_t i) const {
        auto node = i + 1;
        Storage result = tree_[node];
        const auto begin = node - lowbit(node);
        for (--node; node > begin; node -= lowbit(node)) result -= tree_[node];
        return result;
    }
    void add_unchecked(std::size_t i, const Storage& delta) {
        const auto n = static_cast<std::size_t>(size());
        for (++i;;) {
            tree_[i] += delta;
            const auto step = lowbit(i);
            if (step > n - i) break;
            i += step;
        }
    }
public:
    fenwick_tree() = default;
    explicit fenwick_tree(int n) : tree_(storage_size(checked_size(n)), Storage{}) {}
    template<class Values> requires std::is_same_v<Values, std::vector<T>>
    explicit fenwick_tree(const Values& values) : tree_(storage_size(values.size()), Storage{}) {
        const auto n = values.size();
        for (std::size_t i = 1; i <= n; ++i) {
            tree_[i] += to_storage(values[i - 1]);
            const auto step = lowbit(i);
            if (step <= n - i) tree_[i + step] += tree_[i];
        }
    }
    fenwick_tree(const fenwick_tree&) = default;
    fenwick_tree& operator=(const fenwick_tree&) = default;
    fenwick_tree(fenwick_tree&& other) noexcept : tree_(std::move(other.tree_)) { other.tree_.clear(); }
    fenwick_tree& operator=(fenwick_tree&& other) noexcept {
        if (this != &other) { tree_ = std::move(other.tree_); other.tree_.clear(); }
        return *this;
    }
    int size() const noexcept { return tree_.empty() ? 0 : static_cast<int>(tree_.size() - 1); }
    bool empty() const noexcept { return size() == 0; }
    void add(int i, T delta) {
        check_index(i);
        add_unchecked(static_cast<std::size_t>(i), to_storage(delta));
    }
    T get(int i) const { check_index(i); return static_cast<T>(get_unchecked(static_cast<std::size_t>(i))); }
    void set(int i, T value) {
        check_index(i);
        const Storage delta = to_storage(value) - get_unchecked(static_cast<std::size_t>(i));
        add_unchecked(static_cast<std::size_t>(i), delta);
    }
    T sum(int l, int r) {
        check_range(l, r);
        return static_cast<T>(prefix_unchecked(static_cast<std::size_t>(r)) - prefix_unchecked(static_cast<std::size_t>(l)));
    }
    int lower_bound(const T& target) const {
        if (!(T{} < target)) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (from_storage(candidate) < target) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    int upper_bound(const T& target) const {
        if (target < T{}) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (!(target < from_storage(candidate))) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    std::vector<T> values() const {
        if (tree_.empty()) return {};
        std::vector<T> result(tree_.begin() + 1, tree_.end());
        const auto n = static_cast<std::size_t>(size());
        for (auto i = n; i; --i) {
            const auto step = lowbit(i);
            if (step <= n - i) {
                if constexpr (std::is_same_v<T, Storage>) result[i + step - 1] -= result[i - 1];
                else {
                    const Storage difference = static_cast<Storage>(result[i + step - 1]) - static_cast<Storage>(result[i - 1]);
                    result[i + step - 1] = static_cast<T>(difference);
                }
            }
        }
        return result;
    }
    void reset(int n) {
        std::vector<Storage> replacement(storage_size(checked_size(n)), Storage{});
        tree_.swap(replacement);
    }
    void reset() { std::fill(tree_.begin(), tree_.end(), Storage{}); }
    void clear() noexcept { tree_.clear(); }
    void swap(fenwick_tree& other) noexcept { tree_.swap(other.tree_); }
};
#endif

#ifndef SIRKIT_PERMUTATION_ORDER_DEFINED
#define SIRKIT_PERMUTATION_ORDER_DEFINED

namespace detail
{

inline fenwick_tree<int> AvailablePermutationValues(std::size_t size)
{
    CheckedFenwickStorageSize(size, std::vector<unsigned>{}.max_size());
    return fenwick_tree<int>(std::vector<int>(size, 1));
}

}

inline std::vector<int> permutation_lehmer_code(std::span<const int> values)
{
    auto unused = detail::AvailablePermutationValues(values.size());
    const int n = unused.size();
    std::vector<int> code(n);
    for (int i = 0; i < n; ++i)
    {
        const int value = values[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (value < 0 || value >= n || unused.get(value) == 0)
        {
            throw std::invalid_argument("permutation_lehmer_code: expected a permutation of 0..n-1");
        }
#endif
        code[i] = unused.sum(0, value);
        unused.add(value, -1);
    }
    return code;
}

inline std::vector<int> permutation_from_lehmer_code(std::span<const int> code)
{
    auto unused = detail::AvailablePermutationValues(code.size());
    const int n = unused.size();
    std::vector<int> values(n);
    for (int i = 0; i < n; ++i)
    {
        const int digit = code[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (digit < 0 || digit >= n - i)
        {
            throw std::invalid_argument("permutation_from_lehmer_code: digit out of range");
        }
#endif
        const int value = unused.lower_bound(digit + 1);
        values[i] = value;
        unused.add(value, -1);
    }
    return values;
}

inline std::optional<std::uint64_t> permutation_rank(std::span<const int> values)
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const auto code = permutation_lehmer_code(values);
    std::uint64_t rank = 0;
    for (std::size_t i = 0; i < code.size(); ++i)
    {
        const std::uint64_t radix = code.size() - i;
        const auto digit = static_cast<std::uint64_t>(code[i]);
        if (rank > (kMax - digit) / radix)
        {
            return std::nullopt;
        }
        rank = rank * radix + digit;
    }
    return rank;
}

inline std::optional<std::vector<int>> permutation_unrank(int n, std::uint64_t rank)
{
    if (n < 0)
    {
        throw std::invalid_argument("permutation_unrank: negative length");
    }
    std::vector<int> code(n);
    for (int i = n; i > 0; --i)
    {
        const auto radix = static_cast<std::uint64_t>(n - i + 1);
        code[i - 1] = static_cast<int>(rank % radix);
        rank /= radix;
    }
    if (rank != 0)
    {
        return std::nullopt;
    }
    return permutation_from_lehmer_code(code);
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
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<int> permutation(n);
    for (auto &element : permutation)
    {
        verify_in >> element;
        --element;
    }
    const auto code = sirkit::permutation_lehmer_code(permutation);
    std::vector<std::uint32_t> radices(n), digits(code.begin(), code.end());
    for (std::size_t i = 0; i < n; ++i)
    {
        radices[i] = static_cast<std::uint32_t>(n - i);
    }
    const sirkit::MixedRadix format(radices);
    verify_out << (format.encode(digits) + 1).to_string() << '\n';
}
