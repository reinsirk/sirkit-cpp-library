#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence

// Library: Math/Linear_Recurrence.cpp
// SPDX-License-Identifier: CC0-1.0
// 数学的参考（係数抽出）: https://specfun.inria.fr/bostan/BoMo20.pdf
// 数学的参考（講座）: https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_kth_term
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/fps/nth-term.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_recurrence.md
//
// API一覧
// 以下のcは長さd、aは最初のd項。a[n]=sum(c[j]*a[n-j-1])をn>=dで用いる。
// LinearRecurrenceTerm<T>(a,c,k): 0始まりk項目。O(d² log(k+2))時間・O(d)領域。d=0ならT{0}。
// LinearRecurrenceWeights<T>(c,k): k項目の初期d項への係数。k<dはO(d)、それ以外は上記と同計算量。d=0なら{}。
// LinearRecurrencePrefix<T>(a,c,count): 先頭count項。O(count*(1+d))時間・O(count)領域。
// Tは正確な可換環の加減乗算と0/1構築を提供する。除算は不要。Mintは利用側が用意する。
// 上記uint32_t vector overload: 法998244353で計算し、入力は自動で正規化する。
// uint32_t版TermはBostan–Mori+同梱NTT。L=bit_ceil(2d+1)としてO(L logL log(k+2))時間・O(L)領域。
// uint32_t版Weights/Prefixは汎用版と同計算量。ClangのWeights(d>=16)は標準整数のMontgomery演算を使う。
// TermのNTT長は2^23以下、短い畳み込みは直接計算する。
// a.size()==c.size()が前提。長さ・容量の内部上限は検査省略時も維持する。全APIは入力不変。
// NTTも独立ガードで同梱。NTT/FPS/Berlekamp_Massey.cppと重ねて貼付できる。
//
// [同梱BerlekampMassey]
// BerlekampMassey<T>(s): 体T上の有限列sに合う最短線形漸化式の係数cを返す。O(n²)時間・O(n)領域。
// 返却長dとし、d<=i<nでs[i]=sum(c[j]*s[i-j-1])。同じ最短次数の解は一意とは限らない。
// Tは正確な体の四則演算・0/1からの構築・等値比較を提供する。Mintは利用側が用意する。
// BerlekampMassey(vector<uint32_t>): 法998244353で同じ係数を返す。入力は自動で正規化する。
// 空列・全零列は{}。有限列の適合だけを保証し、未知の後続項や本来の漸化式の特定は保証しない。
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
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#if defined(__clang__)
#ifndef SIRKIT_RECURRENCE_MONTGOMERY_DEFINED
#define SIRKIT_RECURRENCE_MONTGOMERY_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
struct RecurrenceMontgomery998 {
    static constexpr std::uint32_t modulus=998244353;
    static constexpr std::uint32_t negative_inverse=[] {
        std::uint32_t inverse=1;
        for(unsigned i=0;i<5;++i)inverse*=2-modulus*inverse;
        return std::uint32_t{0}-inverse;
    }();
    static constexpr std::uint64_t r=(std::uint64_t{1}<<32)%modulus;
    static constexpr std::uint32_t r_squared=static_cast<std::uint32_t>(r*r%modulus);
    std::uint32_t raw=0;
    static constexpr std::uint32_t reduce(std::uint64_t product)noexcept {
        const auto factor=static_cast<std::uint32_t>(product)*negative_inverse;
        // product<p²かつp<2^30なので、この和はuint64_tの範囲内。
        const auto result=static_cast<std::uint32_t>((product+std::uint64_t{factor}*modulus)>>32);
        return result>=modulus?result-modulus:result;
    }
    RecurrenceMontgomery998()=default;
    RecurrenceMontgomery998(std::uint64_t x):raw(reduce((x%modulus)*r_squared)){}
    std::uint32_t get()const noexcept{return reduce(raw);}
    RecurrenceMontgomery998& operator+=(RecurrenceMontgomery998 b)noexcept {
        raw+=b.raw;if(raw>=modulus)raw-=modulus;return *this;
    }
    RecurrenceMontgomery998& operator-=(RecurrenceMontgomery998 b)noexcept {
        raw=raw>=b.raw?raw-b.raw:raw+modulus-b.raw;return *this;
    }
    RecurrenceMontgomery998& operator*=(RecurrenceMontgomery998 b)noexcept {
        raw=reduce(std::uint64_t{raw}*b.raw);return *this;
    }
    friend RecurrenceMontgomery998 operator+(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept{return a+=b;}
    friend RecurrenceMontgomery998 operator-(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept{return a-=b;}
    friend RecurrenceMontgomery998 operator*(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept{return a*=b;}
    friend bool operator==(RecurrenceMontgomery998,RecurrenceMontgomery998)=default;
    friend RecurrenceMontgomery998 operator/(RecurrenceMontgomery998 a,RecurrenceMontgomery998 b)noexcept {
        RecurrenceMontgomery998 inverse{1};std::uint32_t power=modulus-2;
        while(power){if(power&1)inverse*=b;b*=b;power>>=1;}return a*inverse;
    }
};
}
#endif
#endif


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
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <immintrin.h>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#endif

#ifndef SIRKIT_NTT_DETAIL_DEFINED
#define SIRKIT_NTT_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
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

#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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

#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
        #if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        const auto input_a=a.size(), input_b=b.size();
#endif
        a.resize(length); b.resize(length);
        #if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        forward_padded<Check>(a,input_a); forward_padded<Check>(b,input_b);
#else
        forward<Check>(a); forward<Check>(b);
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
        #if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        forward_padded<false>(x,a.size());
#else
        forward<false>(x);
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
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
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
#define SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
struct RecurrenceMod998 {
    static constexpr std::uint32_t modulus=998244353;
    std::uint32_t value=0;
    RecurrenceMod998()=default;
    RecurrenceMod998(std::uint64_t x):value(static_cast<std::uint32_t>(x%modulus)){}
    RecurrenceMod998& operator+=(RecurrenceMod998 b)noexcept{
        value+=b.value;if(value>=modulus)value-=modulus;return *this;
    }
    RecurrenceMod998& operator-=(RecurrenceMod998 b)noexcept{
        value=value>=b.value?value-b.value:value+modulus-b.value;return *this;
    }
    RecurrenceMod998& operator*=(RecurrenceMod998 b)noexcept{
        value=static_cast<std::uint32_t>(std::uint64_t{value}*b.value%modulus);return *this;
    }
    friend RecurrenceMod998 operator+(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a+=b;}
    friend RecurrenceMod998 operator-(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a-=b;}
    friend RecurrenceMod998 operator*(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a*=b;}
    friend bool operator==(RecurrenceMod998,RecurrenceMod998)=default;
    friend RecurrenceMod998 operator/(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{
        RecurrenceMod998 inverse{1};std::uint32_t power=modulus-2;
        while(power){if(power&1)inverse*=b;b*=b;power>>=1;}return a*inverse;
    }
};
}
#endif


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
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BERLEKAMP_MASSEY_DEFINED
#define SIRKIT_BERLEKAMP_MASSEY_DEFINED
template<class T>
std::vector<T> BerlekampMassey(const std::vector<T>& sequence) {
    static_assert(!std::is_integral_v<T>,"BerlekampMassey<T>: use an exact field type, or the uint32_t overload");
    if(sequence.empty())return {};
    const auto n=sequence.size();
    if(n>=std::vector<T>{}.max_size())throw std::length_error("BerlekampMassey: size overflow");
    std::vector<T> connection{T{1}},previous{T{1}};
    std::size_t order=0,shift=1;
    T previous_inverse{1};
    for(std::size_t position=0;position<n;++position) {
        T discrepancy=sequence[position];
        for(std::size_t j=1;j<=order;++j)discrepancy+=connection[j]*sequence[position-j];
        if(discrepancy==T{0}){++shift;continue;}
        const T ratio=discrepancy*previous_inverse;
        const bool extend=order<=position/2;
        std::vector<T> saved;
        if(extend)saved=connection;
        if(previous.size()>connection.max_size()-shift)
            throw std::length_error("BerlekampMassey: size overflow");
        if(connection.size()<previous.size()+shift)connection.resize(previous.size()+shift,T{0});
        for(std::size_t j=0;j<previous.size();++j)connection[j+shift]-=ratio*previous[j];
        if(extend) {
            order=position+1-order;
            previous=std::move(saved);
            previous_inverse=T{1}/discrepancy;
            shift=1;
        } else ++shift;
    }
    std::vector<T> result(order,T{0});
    for(std::size_t j=0;j<order;++j)result[j]=T{0}-connection[j+1];
    return result;
}
inline std::vector<std::uint32_t> BerlekampMassey(const std::vector<std::uint32_t>& sequence) {
    using Field=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RecurrenceMod998;
    std::vector<Field> input;input.reserve(sequence.size());
    for(const auto value:sequence)input.emplace_back(value);
    const auto coefficients=BerlekampMassey(input);
    std::vector<std::uint32_t> result;result.reserve(coefficients.size());
    for(const auto value:coefficients)result.push_back(value.value);
    return result;
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

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
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LINEAR_RECURRENCE_DETAIL_DEFINED
#define SIRKIT_LINEAR_RECURRENCE_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T>
std::size_t recurrence_product_size(std::size_t d) {
    const auto maximum=std::vector<T>{}.max_size();
    if(d>maximum/2+maximum%2)throw std::length_error("LinearRecurrence: size overflow");
    return d?d*2-1:0;
}
template<class T>
void recurrence_product(const std::vector<T>& left,const std::vector<T>& right,
                        const std::vector<T>& coefficients,std::vector<T>& workspace) {
    const auto d=coefficients.size();
    workspace.assign(recurrence_product_size<T>(d),T{0});
    for(std::size_t i=0;i<d;++i)
        for(std::size_t j=0;j<d;++j)workspace[i+j]+=left[i]*right[j];
    for(std::size_t i=workspace.size();i-->d;) {
        const T highest=workspace[i];
        for(std::size_t j=0;j<d;++j)workspace[i-j-1]+=highest*coefficients[j];
    }
    workspace.resize(d);
}
inline std::vector<RecurrenceMod998> recurrence_field_vector(const std::vector<std::uint32_t>& input) {
    std::vector<RecurrenceMod998> result;result.reserve(input.size());
    for(auto x:input)result.emplace_back(x);
    return result;
}
inline std::vector<std::uint32_t> recurrence_residues(const std::vector<RecurrenceMod998>& input) {
    std::vector<std::uint32_t> result;result.reserve(input.size());
    for(auto x:input)result.push_back(x.value);
    return result;
}
}
#endif
#ifndef SIRKIT_LINEAR_RECURRENCE_DEFINED
#define SIRKIT_LINEAR_RECURRENCE_DEFINED
template<class T>
std::vector<T> LinearRecurrenceWeights(const std::vector<T>& coefficients,std::uint64_t k) {
    static_assert(!std::is_integral_v<T>,"LinearRecurrenceWeights<T>: use a coefficient type, or the uint32_t overload");
    const auto d=coefficients.size();
    if(d==0)return {};
    const auto capacity=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_product_size<T>(d);
    if(k<d) {
        std::vector<T> result(d,T{0});result[static_cast<std::size_t>(k)]=T{1};return result;
    }
    std::vector<T> result(d,T{0}),power(d,T{0}),workspace;
    result.reserve(capacity);power.reserve(capacity);workspace.reserve(capacity);
    result[0]=T{1};
    if(d==1)power[0]=coefficients[0];else power[1]=T{1};
    while(k) {
        if(k&1){/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_product(result,power,coefficients,workspace);result.swap(workspace);}
        k>>=1;
        if(k){/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_product(power,power,coefficients,workspace);power.swap(workspace);}
    }
    return result;
}
template<class T>
T LinearRecurrenceTerm(const std::vector<T>& initial,const std::vector<T>& coefficients,std::uint64_t k) {
    static_assert(!std::is_integral_v<T>,"LinearRecurrenceTerm<T>: use a coefficient type, or the uint32_t overload");
#ifndef SKIP_RUNTIME_CHECKS
    if(initial.size()!=coefficients.size())throw std::invalid_argument("LinearRecurrenceTerm: mismatched sizes");
#endif
    if(k<initial.size())return initial[static_cast<std::size_t>(k)];
    const auto weights=LinearRecurrenceWeights(coefficients,k);
    T result{0};
    for(std::size_t j=0;j<weights.size();++j)result+=initial[j]*weights[j];
    return result;
}
template<class T>
std::vector<T> LinearRecurrencePrefix(const std::vector<T>& initial,const std::vector<T>& coefficients,std::size_t count) {
    static_assert(!std::is_integral_v<T>,"LinearRecurrencePrefix<T>: use a coefficient type, or the uint32_t overload");
#ifndef SKIP_RUNTIME_CHECKS
    if(initial.size()!=coefficients.size())throw std::invalid_argument("LinearRecurrencePrefix: mismatched sizes");
#endif
    std::vector<T> result(count,T{0});
    const auto d=coefficients.size();
    std::copy_n(initial.begin(),std::min(d,count),result.begin());
    for(std::size_t i=d;i<count;++i)
        for(std::size_t j=0;j<d;++j)result[i]+=coefficients[j]*result[i-j-1];
    return result;
}
inline std::vector<std::uint32_t> LinearRecurrenceWeights(const std::vector<std::uint32_t>& coefficients,std::uint64_t k) {
    if(k<coefficients.size()) {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_product_size</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RecurrenceMod998>(coefficients.size());
        std::vector<std::uint32_t> result(coefficients.size(),0);
        result[static_cast<std::size_t>(k)]=1;return result;
    }
#if defined(__clang__)
    if(coefficients.size()>=16) {
        using Field=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RecurrenceMontgomery998;
        std::vector<Field> normalized;normalized.reserve(coefficients.size());
        for(auto value:coefficients)normalized.emplace_back(value);
        const auto weights=LinearRecurrenceWeights(normalized,k);
        std::vector<std::uint32_t> result;result.reserve(weights.size());
        for(auto value:weights)result.push_back(value.get());
        return result;
    }
#endif
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_residues(LinearRecurrenceWeights(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_field_vector(coefficients),k));
}
inline std::vector<std::uint32_t> LinearRecurrencePrefix(const std::vector<std::uint32_t>& initial,
                                                      const std::vector<std::uint32_t>& coefficients,std::size_t count) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_residues(LinearRecurrencePrefix(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_field_vector(initial),/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_field_vector(coefficients),count));
}
inline std::uint32_t LinearRecurrenceTerm(const std::vector<std::uint32_t>& initial,
                                        const std::vector<std::uint32_t>& coefficients,std::uint64_t k) {
#ifndef SKIP_RUNTIME_CHECKS
    if(initial.size()!=coefficients.size())throw std::invalid_argument("LinearRecurrenceTerm: mismatched sizes");
#endif
    constexpr std::uint32_t mod=998244353;
    if(k<initial.size())return initial[static_cast<std::size_t>(k)]%mod;
    const auto d=coefficients.size();
    if(d==0)return 0;
    if(d>=std::vector<std::uint32_t>{}.max_size())throw std::length_error("LinearRecurrenceTerm: size overflow");
    std::vector<std::uint32_t> denominator(d+1);denominator[0]=1;
    for(std::size_t j=0;j<d;++j){const auto x=coefficients[j]%mod;denominator[j+1]=x?mod-x:0;}
    while(denominator.size()>1&&denominator.back()==0)denominator.pop_back();
    NTT<> ntt;
    auto numerator=ntt.convolution(initial,denominator);numerator.resize(d);
    while(!numerator.empty()&&numerator.back()==0)numerator.pop_back();
    while(k&&!numerator.empty()) {
        auto reflected=denominator;
        for(std::size_t j=1;j<reflected.size();j+=2)if(reflected[j])reflected[j]=mod-reflected[j];
        auto product=ntt.convolution_normalized(std::move(numerator),reflected);
        auto norm=ntt.convolution_normalized(denominator,std::move(reflected));
        numerator.clear();numerator.reserve((product.size()+1)/2);
        for(std::size_t j=static_cast<std::size_t>(k&1);j<product.size();j+=2)numerator.push_back(product[j]);
        denominator.clear();denominator.reserve((norm.size()+1)/2);
        for(std::size_t j=0;j<norm.size();j+=2)denominator.push_back(norm[j]);
        while(!numerator.empty()&&numerator.back()==0)numerator.pop_back();
        while(denominator.size()>1&&denominator.back()==0)denominator.pop_back();
        k>>=1;
    }
    // 定数項1の分母から始め、Q(x)Q(-x)の偶数次項だけを取るので定数項は常に1。
    return numerator.empty()?0:numerator[0];
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
int main() {
    std::size_t d{};std::uint64_t k{};if(!(verify_in>>d>>k))return 0;
    std::vector<std::uint32_t> initial(d),coefficients(d);
    for(auto&value:initial)verify_in>>value;
    for(auto&value:coefficients)verify_in>>value;
    verify_out<<LinearRecurrenceTerm(initial,coefficients,k)<<'\n';
}
