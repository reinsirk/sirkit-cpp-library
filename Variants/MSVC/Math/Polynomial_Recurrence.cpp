// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Math/Polynomial_Recurrence.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_recurrence.md
// 参考（多項式係数の漸化式）: https://atcoder.jp/contests/abc222/editorial/2742
//
// API一覧
// polynomial_recurrence_term(initial,coefficients,index): 法998244353で漸化式の第index項を求める。
// d=initial.size()>=1、Σ_{j=0}^d P_j(t)*a_(t+j)=0。coefficients[j]はP_jの定数項順のint64_t係数。
// initialはa_0..a_(d-1)。indexはuint64_t。更新が要る場合、使う全tでP_d(t)!=0。違反はdomain_error。
// 形状不正はinvalid_argument。正規化後の多項式次数は256以下（項更新を行う場合）。
// 更新長L=index-d+1、行列次元s=d+1でpolynomial_matrix_interval_productと同じ計算量。
// index<dは初期値をO(1)で返す。係数列と初期値は変更しない。
//
// [同梱の多項式行列積]
// polynomial_matrix_interval_product(matrix,first,count): A(first)*A(first+1)*...*A(first+count-1)を法998244353で返す。
// 正方行列の各要素は定数項順のint64_tの係数列。空は零多項式。法で正規化した最大次数は256以下。
// firstはint64_t、countはuint64_t。法の周期を使うので巨大なcountも可。count=0は単位行列、空行列は空。
// 行列の一辺をs、最大次数をd、P=998244353、H=1+sqrt((d+1)*min(count,P))とする。
// 入力係数C個でO(C+s²(d+1)²+s²H log(H+1)+s³(H+log(count+1)))時間・O(s²(d+H+1))領域。
// 形状不正はinvalid_argument、次数上限超過はlength_error。
//
// [同梱の等間隔多項式評価]
// evaluate_consecutive_polynomial(samples,first,count): f(0)..f(n-1)からf(first)..f(first+count-1)を返す。
// 次数n未満・法998244353。n=0は零多項式、firstはint64_tで負も可。n,count≤2^20。
// nはsamples.size()、m=count。O(1+(n+m)log(n+m+1))時間・O(1+n+m)領域。
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


    template <bool Check>
    void forward(std::vector<std::uint32_t>& a) const {
        const std::size_t n = a.size();
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
        forward<Check>(a);
    }
    template <bool Check>
    void backward(std::vector<std::uint32_t>& a) const {
        const std::size_t n = a.size();
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
        a.resize(length); b.resize(length);
        forward<Check>(a); forward<Check>(b);
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
        forward<false>(x);
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
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_CONSECUTIVE_POLYNOMIAL_EVALUATION_DEFINED
#define SIRKIT_CONSECUTIVE_POLYNOMIAL_EVALUATION_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr std::uint32_t kConsecutiveMod = NTT<>::modulus();

inline std::uint32_t ConsecutiveMultiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kConsecutiveMod);
}

inline std::uint32_t ConsecutiveInverse(std::uint32_t a)
{
    std::uint32_t result = 1;
    for (auto exp = kConsecutiveMod - 2; exp != 0; exp >>= 1)
    {
        if ((exp & 1U) != 0)
        {
            result = ConsecutiveMultiply(result, a);
        }
        a = ConsecutiveMultiply(a, a);
    }
    return result;
}

inline std::vector<std::uint32_t> ConsecutiveWeights(std::span<const std::uint32_t> samples)
{
    const auto n = samples.size();
    std::vector<std::uint32_t> inverse_factorial(n, 1);
    for (std::size_t i = 1; i < n; ++i)
    {
        inverse_factorial[i] = ConsecutiveMultiply(inverse_factorial[i - 1], static_cast<std::uint32_t>(i));
    }
    auto inverse = ConsecutiveInverse(inverse_factorial.back());
    for (std::size_t i = n; i > 1; --i)
    {
        inverse_factorial[i - 1] = inverse;
        inverse = ConsecutiveMultiply(inverse, static_cast<std::uint32_t>(i - 1));
    }
    std::vector<std::uint32_t> weighted(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        auto value = ConsecutiveMultiply(samples[i], ConsecutiveMultiply(inverse_factorial[i], inverse_factorial[n - 1 - i]));
        if ((n - 1 - i) % 2 != 0 && value != 0)
        {
            value = kConsecutiveMod - value;
        }
        weighted[i] = value;
    }
    return weighted;
}

inline void EvaluateConsecutiveOutside(NTT<> &ntt, std::uint32_t first, const std::vector<std::uint32_t> &weighted,
                                       std::span<std::uint32_t> output)
{
    const auto n = weighted.size();
    const auto lower = first - static_cast<std::uint32_t>(n) + 1;
    std::vector<std::uint32_t> inverse(n + output.size() - 1);
    std::uint32_t product = 1;
    for (std::size_t i = 0; i < inverse.size(); ++i)
    {
        product = ConsecutiveMultiply(product, lower + static_cast<std::uint32_t>(i));
        inverse[i] = product;
    }
    auto reciprocal = ConsecutiveInverse(product);
    product = inverse[n - 1];
    for (std::size_t i = inverse.size(); i > 0; --i)
    {
        inverse[i - 1] = ConsecutiveMultiply(reciprocal, i == 1 ? 1 : inverse[i - 2]);
        reciprocal = ConsecutiveMultiply(reciprocal, lower + static_cast<std::uint32_t>(i - 1));
    }
    // Lagrange補間の分母列と重みを畳み込み、分子は隣接する評価点へ差分更新する。
    const auto convolution = ntt.convolution_normalized(weighted, inverse);
    for (std::size_t i = 0; i < output.size(); ++i)
    {
        output[i] = ConsecutiveMultiply(convolution[n - 1 + i], product);
        product = ConsecutiveMultiply(product, ConsecutiveMultiply(first + static_cast<std::uint32_t>(i) + 1, inverse[i]));
    }
}
}

inline std::vector<std::uint32_t> evaluate_consecutive_polynomial(std::span<const std::uint32_t> samples,
                                                                  std::int64_t first, std::size_t count)
{
    constexpr auto kMaxSize = std::size_t{1} << 20;
    constexpr auto kMod = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::kConsecutiveMod;
    if (samples.size() > kMaxSize || count > kMaxSize)
    {
        throw std::length_error("evaluate_consecutive_polynomial: expected at most 2^20 points");
    }
    if (samples.size() <= 1)
    {
        return std::vector<std::uint32_t>(count, samples.empty() ? 0 : samples[0] % kMod);
    }
    const auto residue = first % static_cast<std::int64_t>(kMod);
    auto x = static_cast<std::uint32_t>(residue < 0 ? residue + kMod : residue);
    std::vector<std::uint32_t> weighted, result(count);
    NTT<> ntt;
    for (std::size_t done = 0; done < count;)
    {
        if (x < samples.size())
        {
            const auto direct = std::min(count - done, samples.size() - x);
            for (std::size_t i = 0; i < direct; ++i)
            {
                result[done + i] = samples[x + i] % kMod;
            }
            x += static_cast<std::uint32_t>(direct);
            done += direct;
        }
        else
        {
            if (weighted.empty())
            {
                weighted = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ConsecutiveWeights(samples);
            }
            const auto outside = std::min(count - done, static_cast<std::size_t>(kMod - x));
            /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::EvaluateConsecutiveOutside(ntt, x, weighted, std::span<std::uint32_t>(result).subspan(done, outside));
            x += static_cast<std::uint32_t>(outside);
            done += outside;
            if (x == kMod)
            {
                x = 0;
            }
        }
    }
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
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_POLYNOMIAL_MATRIX_PRODUCT_DEFINED
#define SIRKIT_POLYNOMIAL_MATRIX_PRODUCT_DEFINED

namespace detail
{
class PolynomialMatrixProducts
{
    using Polynomial = std::vector<std::uint32_t>;
    using FlatMatrix = std::vector<std::uint32_t>;
    using Samples = std::vector<Polynomial>;
    using InputMatrix = std::vector<std::vector<std::vector<std::int64_t>>>;
    using OutputMatrix = std::vector<std::vector<std::uint32_t>>;
    static constexpr std::size_t kMaxDegree = 256;
    static constexpr auto kMod = kConsecutiveMod;

public:
    explicit PolynomialMatrixProducts(const InputMatrix &matrix) : side_(matrix.size())
    {
        for (const auto &row : matrix)
        {
            if (row.size() != side_)
            {
                throw std::invalid_argument("polynomial_matrix_interval_product: matrix must be square");
            }
            for (const auto &entry : row)
            {
                auto length = entry.size();
                while (length != 0 && Residue(entry[length - 1]) == 0)
                {
                    --length;
                }
                if (length > kMaxDegree + 1)
                {
                    throw std::length_error("polynomial_matrix_interval_product: degree exceeds 256");
                }
                auto &coefficients = entries_.emplace_back();
                coefficients.reserve(length);
                for (std::size_t i = 0; i < length; ++i)
                {
                    coefficients.push_back(Residue(entry[i]));
                }
                degree_ = std::max(degree_, std::max<std::size_t>(1, length) - 1);
            }
        }
    }

    [[nodiscard]] OutputMatrix Product(std::int64_t first, std::uint64_t count) const
    {
        const auto start = Residue(first);
        if (count == 0 || side_ == 0)
        {
            return Expand(Identity());
        }
        if (degree_ == 0)
        {
            return Expand(Power(Evaluate(start), count));
        }
        if (count < kMod)
        {
            return Expand(Interval(start, static_cast<std::uint32_t>(count)));
        }
        auto prefix = Power(Interval(start, kMod), count / kMod);
        const auto suffix = Interval(start, static_cast<std::uint32_t>(count % kMod));
        FlatMatrix workspace(entries_.size());
        Append(prefix, workspace, [&](std::size_t entry)
               {
                   return suffix[entry];
               });
        return Expand(prefix);
    }

private:
    [[nodiscard]] static std::uint32_t Residue(std::int64_t value)
    {
        const auto remainder = value % kMod;
        return static_cast<std::uint32_t>(remainder < 0 ? remainder + kMod : remainder);
    }

    [[nodiscard]] static std::uint32_t Add(std::uint32_t a, std::uint32_t b)
    {
        const auto sum = a + b;
        return sum >= kMod ? sum - kMod : sum;
    }

    [[nodiscard]] static std::uint32_t Evaluate(const Polynomial &coefficients, std::uint32_t point)
    {
        std::uint32_t value = 0;
        for (auto coefficient = coefficients.rbegin(); coefficient != coefficients.rend(); ++coefficient)
        {
            value = Add(ConsecutiveMultiply(value, point), *coefficient);
        }
        return value;
    }

    [[nodiscard]] FlatMatrix Evaluate(std::uint32_t point) const
    {
        FlatMatrix result(entries_.size());
        for (std::size_t i = 0; i < entries_.size(); ++i)
        {
            result[i] = Evaluate(entries_[i], point);
        }
        return result;
    }

    [[nodiscard]] FlatMatrix Identity() const
    {
        FlatMatrix result(entries_.size());
        for (std::size_t i = 0; i < side_; ++i)
        {
            result[i * side_ + i] = 1;
        }
        return result;
    }

    [[nodiscard]] OutputMatrix Expand(const FlatMatrix &matrix) const
    {
        OutputMatrix result(side_, std::vector<std::uint32_t>(side_));
        for (std::size_t i = 0; i < side_; ++i)
        {
            std::copy_n(matrix.data() + i * side_, side_, result[i].begin());
        }
        return result;
    }

    template <class GetRight>
    void Append(FlatMatrix &prefix, FlatMatrix &workspace, GetRight right) const
    {
        std::ranges::fill(workspace, 0);
        for (std::size_t i = 0; i < side_; ++i)
        {
            for (std::size_t k = 0; k < side_; ++k)
            {
                const auto left = prefix[i * side_ + k];
                for (std::size_t j = 0; j < side_; ++j)
                {
                    auto &entry = workspace[i * side_ + j];
                    entry = Add(entry, ConsecutiveMultiply(left, right(k * side_ + j)));
                }
            }
        }
        prefix.swap(workspace);
    }

    [[nodiscard]] FlatMatrix Power(FlatMatrix base, std::uint64_t exponent) const
    {
        auto result = Identity();
        FlatMatrix workspace(entries_.size());
        for (; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                Append(result, workspace, [&](std::size_t entry)
                       {
                           return base[entry];
                       });
            }
            Append(base, workspace, [&](std::size_t entry)
                   {
                       return base[entry];
                   });
        }
        return result;
    }

    void AppendTail(FlatMatrix &prefix, std::uint32_t start, std::uint32_t count, FlatMatrix &workspace) const
    {
        FlatMatrix next(entries_.size());
        for (std::uint32_t i = 0; i < count; ++i)
        {
            const auto point = static_cast<std::uint32_t>((std::uint64_t{start} + i) % kMod);
            for (std::size_t entry = 0; entry < entries_.size(); ++entry)
            {
                next[entry] = Evaluate(entries_[entry], point);
            }
            Append(prefix, workspace, [&](std::size_t entry)
                   {
                       return next[entry];
                   });
        }
    }

    [[nodiscard]] Samples DoubleBlock(const Samples &current, std::size_t points, std::uint32_t shift) const
    {
        Samples left, right;
        left.reserve(current.size());
        right.reserve(current.size());
        for (const auto &entry : current)
        {
            left.push_back(evaluate_consecutive_polynomial(entry, 0, points));
            right.push_back(evaluate_consecutive_polynomial(entry, shift, points));
        }
        Samples next(entries_.size(), Polynomial(points));
        for (std::size_t i = 0; i < side_; ++i)
        {
            for (std::size_t k = 0; k < side_; ++k)
            {
                const auto &a = left[i * side_ + k];
                for (std::size_t j = 0; j < side_; ++j)
                {
                    const auto &b = right[k * side_ + j];
                    auto &out = next[i * side_ + j];
                    for (std::size_t point = 0; point < points; ++point)
                    {
                        out[point] = Add(out[point], ConsecutiveMultiply(a[point], b[point]));
                    }
                }
            }
        }
        return next;
    }

    [[nodiscard]] FlatMatrix Interval(std::uint32_t start, std::uint32_t count) const
    {
        auto result = Identity();
        FlatMatrix workspace(entries_.size());
        std::uint32_t block = 1;
        while (std::uint64_t{block} * block * 4 * degree_ <= count)
        {
            block *= 2;
        }
        if (block == 1)
        {
            AppendTail(result, start, count, workspace);
            return result;
        }

        const auto inverse = ConsecutiveInverse(block);
        Samples samples(entries_.size(), Polynomial(degree_ + 1));
        for (std::size_t i = 0; i <= degree_; ++i)
        {
            const auto point = static_cast<std::uint32_t>((std::uint64_t{start} + std::uint64_t{block} * i) % kMod);
            for (std::size_t entry = 0; entry < entries_.size(); ++entry)
            {
                samples[entry][i] = Evaluate(entries_[entry], point);
            }
        }
        for (std::uint32_t current = 1; current < block; current *= 2)
        {
            const auto points = 2 * current * degree_ + 1;
            const auto shift = ConsecutiveMultiply(current, inverse);
            samples = DoubleBlock(samples, points, shift); // 長さcurrentの積を、開始点をcurrentずらした積と掛ける。
        }

        const auto blocks = count / block;
        for (auto &entry : samples)
        {
            entry = evaluate_consecutive_polynomial(entry, 0, blocks);
        }
        for (std::size_t i = 0; i < blocks; ++i)
        {
            Append(result, workspace, [&](std::size_t entry)
                   {
                       return samples[entry][i];
                   });
        }
        const auto tail_start = static_cast<std::uint32_t>((std::uint64_t{start} + std::uint64_t{blocks} * block) % kMod);
        AppendTail(result, tail_start, count % block, workspace);
        return result;
    }

    std::size_t side_;
    std::size_t degree_ = 0;
    Samples entries_;
};
} // namespace detail

[[nodiscard]] inline std::vector<std::vector<std::uint32_t>> polynomial_matrix_interval_product(
    const std::vector<std::vector<std::vector<std::int64_t>>> &matrix, std::int64_t first, std::uint64_t count)
{
    return detail::PolynomialMatrixProducts(matrix).Product(first, count);
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
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_POLYNOMIAL_RECURRENCE_DEFINED
#define SIRKIT_POLYNOMIAL_RECURRENCE_DEFINED

namespace detail
{
inline std::uint32_t PolynomialRecurrenceResidue(std::int64_t value)
{
    const auto residue = value % kConsecutiveMod;
    return static_cast<std::uint32_t>(residue < 0 ? residue + kConsecutiveMod : residue);
}

inline std::vector<std::int64_t> ReverseRecurrenceArgument(std::span<const std::int64_t> polynomial, bool negate)
{
    std::vector<std::int64_t> coefficients;
    coefficients.reserve(polynomial.size());
    for (std::size_t i = 0; i < polynomial.size(); ++i)
    {
        const auto value = polynomial[i] % kConsecutiveMod;
        const bool change_sign = ((i & 1u) != 0) != negate;
        coefficients.push_back(change_sign ? -value : value);
    }
    while (!coefficients.empty() && coefficients.back() == 0)
    {
        coefficients.pop_back();
    }
    return coefficients;
}
} // namespace detail

[[nodiscard]] inline std::uint32_t polynomial_recurrence_term(
    std::span<const std::int64_t> initial, std::span<const std::vector<std::int64_t>> coefficients,
    std::uint64_t index)
{
    const auto order = initial.size();
    if (order == 0 || coefficients.empty() || coefficients.size() - 1 != order)
    {
        throw std::invalid_argument("polynomial_recurrence_term: expected d initial values and d+1 coefficients");
    }
    if (index < order)
    {
        return detail::PolynomialRecurrenceResidue(initial[static_cast<std::size_t>(index)]);
    }
    const auto scale = detail::ReverseRecurrenceArgument(coefficients.back(), false);
    using Polynomial = std::vector<std::int64_t>;
    std::vector<std::vector<Polynomial>> transition(order + 1, std::vector<Polynomial>(order + 1));
    for (std::size_t i = 0; i < order; ++i)
    {
        transition[0][i] = detail::ReverseRecurrenceArgument(coefficients[order - i - 1], true);
    }
    for (std::size_t i = 1; i < order; ++i)
    {
        transition[i][i - 1] = scale;
    }
    transition[order][order] = scale; // 各更新の分母を別の成分でまとめて掛ける。
    const auto last = index - order;
    const auto first = -static_cast<std::int64_t>(last % detail::kConsecutiveMod);
    const auto product = polynomial_matrix_interval_product(transition, first, last + 1);
    const auto denominator = product[order][order];
    if (denominator == 0)
    {
        throw std::domain_error("polynomial_recurrence_term: a leading coefficient evaluated to zero");
    }
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < order; ++i)
    {
        const auto seed = detail::PolynomialRecurrenceResidue(initial[order - i - 1]);
        value += detail::ConsecutiveMultiply(product[0][i], seed);
        if (value >= detail::kConsecutiveMod)
        {
            value -= detail::kConsecutiveMod;
        }
    }
    return detail::ConsecutiveMultiply(value, detail::ConsecutiveInverse(denominator));
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
