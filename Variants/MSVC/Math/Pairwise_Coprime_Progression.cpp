// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Math/Pairwise_Coprime_Progression.cpp
// 参考（公差を指定した等差数列の存在判定）: https://yukicoder.me/problems/no/3832
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pairwise_coprime_progression.md
//
// API一覧
// max_pairwise_coprime_progression_length(step): 正整数でどの2項も互いに素な等差数列の最大長。
// pairwise_coprime_progression_start(length, step): その長さを実現する正の初項をBigInteger<10>で返す。不能ならnullopt。
// 公差stepは正のuint64_t。step=0はinvalid_argument。空列に対しても正の初項を返す。
// 64ビット公差の最大長は105。固定個数の素数と短い整数だけを使い、時間・領域ともにO(1)。
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
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <string>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_PAIRWISE_COPRIME_PROGRESSION
#define SIRKIT_PAIRWISE_COPRIME_PROGRESSION

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
constexpr std::array<std::uint32_t, 27> kCoprimeProgressionPrimes{
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103};
// 53以下の全素数がstepを割ることは64ビットの範囲では起こらない。
static_assert(std::numeric_limits<std::uint64_t>::max() / 53 < 614889782588491410ULL);
} // namespace reinsirk::detail

[[nodiscard]] inline std::uint64_t max_pairwise_coprime_progression_length(std::uint64_t step)
{
    if (step == 0)
    {
        throw std::invalid_argument("pairwise coprime progression: step must be positive");
    }
    const auto missing = std::ranges::find_if(detail::kCoprimeProgressionPrimes,
                                              [step](std::uint32_t prime)
                                              {
                                                  return step % prime != 0;
                                              });
    // 2p項並べると、pで割り切れる位置が少なくとも2回現れる。
    return 2 * *missing - 1;
}

// productはstepと互いに素。各p | productでは、初項はstepと合同だからpの倍数はp項目だけ。
[[nodiscard]] inline std::optional<BigInteger<10>> pairwise_coprime_progression_start(std::uint64_t length, std::uint64_t step)
{
    if (length > max_pairwise_coprime_progression_length(step))
    {
        return std::nullopt;
    }
    BigInteger<10> product{1};
    for (const auto prime : detail::kCoprimeProgressionPrimes)
    {
        if (prime >= length)
        {
            break;
        }
        if (step % prime != 0)
        {
            product *= BigInteger<10>(prime);
        }
    }
    return BigInteger<10>(std::to_string(step)) + product;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
