// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Tridiagonal_Matrix_Power.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tridiagonal_matrix_power.md
// 参考（三重対角行列の余因子と転置分割統治）: https://atcoder.jp/contests/arc228/editorial/24856
//
// API一覧
// tridiagonal_power_first_column(exponent,lower,diagonal,upper): 法998244353の三重対角行列の累乗の第0列。
// n=diagonal.size()、下・上対角の長さはmax(0,n-1)。lower[i]=T[i+1][i]、upper[i]=T[i][i+1]。
// 係数はint64_tで負も可。指数はuint64_t。空行列は空の列、指数0は第0単位ベクトル。
// s=min(n,exponent+1)が2^20以下。長さ不一致はinvalid_argument、上限超過はlength_error。
// O(n+s log(s+1)(log(s+1)+log(exponent+2)))時間、O(n+s(log(s+1)+log(exponent+2)))領域。
//
// [同梱の連続線形漸化式]
// consecutive_linear_recurrence(initial, coefficients, first, count): 法998244353で第first項から連続count項を返す。
// 両入力の長さdは等しく、a_i=Σ_j c_j*a_(i−j−1) (0≤j<d、i≥d)。各係数は剰余に直す。
// count=0なら空、d=0なら0列。最後の添字がuint64_tを超える場合はoverflow_error。
// NTT用の上限はd+count≤2^22（count>0のとき）。超過はlength_error、通常版で長さ不一致はinvalid_argument。
// O(d log(d+1) log(first+2)+(d+count) log(d+count+1))時間、O(d log(first+2)+count)領域。
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
                for (std::size_t j = 0; j < b.size(); ++j) {
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
#include <limits>

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

#ifndef SIRKIT_CONSECUTIVE_LINEAR_RECURRENCE_DEFINED
#define SIRKIT_CONSECUTIVE_LINEAR_RECURRENCE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
using ConsecutiveCoefficients = std::vector<std::uint32_t>;
inline constexpr auto kConsecutiveRecurrenceMod = NTT<>::modulus();

inline std::uint32_t ConsecutiveSubtract(std::uint32_t a, std::uint32_t b)
{
    return a >= b ? a - b : a + kConsecutiveRecurrenceMod - b;
}

inline void CheckConsecutiveRecurrenceSize(std::size_t order, std::size_t count)
{
    constexpr auto kLimit = NTT<>::max_length() / 2;
    if (order > kLimit || count > kLimit - order)
    {
        throw std::length_error("consecutive_linear_recurrence: NTT size limit");
    }
}

inline ConsecutiveCoefficients ConsecutiveReciprocalPrefix(std::span<const std::uint32_t> q,
                                                           std::size_t count, NTT<> &ntt)
{
    ConsecutiveCoefficients inverse{1};
    for (std::size_t known = 1; known < count;)
    {
        const auto next = std::min(count, 2 * known);
        const auto prefix = q.first(std::min(q.size(), next));
        auto correction = ntt.convolution_normalized({prefix.begin(), prefix.end()}, inverse);
        correction.resize(next);
        correction.erase(correction.begin(), correction.begin() + known);
        for (auto &value : correction)
        {
            value = ConsecutiveSubtract(0, value);
        }
        // G←G(2−QG)。下位known項は既知なので、補正の高位部分だけとGを畳み込む。
        const auto high = ntt.convolution_normalized(inverse, std::move(correction));
        inverse.resize(next);
        std::copy_n(high.begin(), next - known, inverse.begin() + known);
        known = next;
    }
    return inverse;
}

inline void ConsecutiveFromTransform(ConsecutiveCoefficients &filter, std::span<const std::uint32_t> transformed,
                                     NTT<> &ntt)
{
    filter.resize(transformed.size());
    ntt.transform(filter);
    for (std::size_t i = 0; i < filter.size(); ++i)
    {
        filter[i] = static_cast<std::uint32_t>(std::uint64_t{filter[i]} * transformed[i] % kConsecutiveRecurrenceMod);
    }
    ntt.transform(filter, true);
}

inline ConsecutiveCoefficients ConsecutiveReciprocalSlice(ConsecutiveCoefficients q, std::uint64_t first,
                                                          std::size_t count, NTT<> &ntt)
{
    const auto degree = q.size() - 1;
    if (degree == 0)
    {
        ConsecutiveCoefficients result(count);
        result[0] = static_cast<std::uint32_t>(first == 0);
        return result;
    }
    if (first <= degree)
    {
        const auto start = static_cast<std::size_t>(first);
        auto result = ConsecutiveReciprocalPrefix(q, start + count, ntt);
        result.erase(result.begin(), result.begin() + start);
        return result;
    }

    ConsecutiveCoefficients even((q.size() + 1) / 2), odd(q.size() / 2);
    for (std::size_t j = 0; j < q.size(); j += 2)
    {
        even[j / 2] = q[j];
    }
    for (std::size_t j = 1; j < q.size(); j += 2)
    {
        odd[j / 2] = ConsecutiveSubtract(0, q[j]);
    }
    // Q(x)Q(-x)=R(x²)。R(z)=even²−z*odd² を半分の長さの二乗で求める。
    auto norm = ntt.square(even);
    norm.resize(q.size());
    const auto odd_square = ntt.square(odd);
    for (std::size_t j = 0; j < odd_square.size(); ++j)
    {
        norm[j + 1] = ConsecutiveSubtract(norm[j + 1], odd_square[j]);
    }
    ConsecutiveCoefficients{}.swap(q);

    const auto distance = first - degree;
    const auto first_half = distance / 2 + distance % 2;
    const auto last_half = (first + (count - 1)) / 2;
    const auto count_half = static_cast<std::size_t>(last_half - first_half + 1);
    auto center = ConsecutiveReciprocalSlice(std::move(norm), first_half, count_half, ntt);

    // 1/Q=Q(-x)/R(x²) を偶数・奇数に分けて復元する。
    // 各抽出位置は該当filterの次数以上なので、低次へ回り込む巡回畳み込みでも値が変わらない。
    center.resize(std::bit_ceil(count_half));
    ntt.transform(center);
    ConsecutiveFromTransform(even, center, ntt);
    ConsecutiveFromTransform(odd, center, ntt);
    ConsecutiveCoefficients result(count);
    for (std::size_t j = 0; j < count; ++j)
    {
        const auto index = first + j;
        result[j] = ((index & 1) == 0 ? even : odd)[static_cast<std::size_t>(index / 2 - first_half)];
    }
    return result;
}
} // namespace reinsirk::detail

inline std::vector<std::uint32_t> consecutive_linear_recurrence(
    std::span<const std::uint32_t> initial, std::span<const std::uint32_t> coefficients,
    std::uint64_t first, std::size_t count)
{
    if (count == 0)
    {
        return {};
    }
    if (count - 1 > std::numeric_limits<std::uint64_t>::max() - first)
    {
        throw std::overflow_error("consecutive_linear_recurrence: index overflow");
    }
    const auto order = initial.size();
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckConsecutiveRecurrenceSize(order, count);
    if (order == 0)
    {
        return std::vector<std::uint32_t>(count);
    }
    const auto last = first + (count - 1);
    constexpr auto kMod = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::kConsecutiveRecurrenceMod;
    if (last < order)
    {
        const auto requested = initial.subspan(static_cast<std::size_t>(first), count);
        std::vector<std::uint32_t> result;
        result.reserve(count);
        for (const auto value : requested)
        {
            result.push_back(value % kMod);
        }
        return result;
    }

    std::vector<std::uint32_t> denominator(order + 1);
    denominator[0] = 1;
    for (std::size_t i = 0; i < order; ++i)
    {
        denominator[i + 1] = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ConsecutiveSubtract(0, coefficients[i] % kMod);
    }
    while (denominator.size() > 1 && denominator.back() == 0)
    {
        denominator.pop_back();
    }
    NTT<> ntt;
    auto numerator = ntt.convolution(std::vector(initial.begin(), initial.end()), denominator);
    numerator.resize(order);
    const auto window_start = first - std::min(first, static_cast<std::uint64_t>(order - 1));
    const auto window_count = static_cast<std::size_t>(last - window_start + 1);
    auto window = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ConsecutiveReciprocalSlice(std::move(denominator), window_start, window_count, ntt);
    auto series = ntt.convolution_normalized(std::move(numerator), std::move(window));
    const auto start = static_cast<std::size_t>(first - window_start);
    return {series.begin() + start, series.begin() + start + count};
}

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

#ifndef SIRKIT_TRIDIAGONAL_MATRIX_POWER_DEFINED
#define SIRKIT_TRIDIAGONAL_MATRIX_POWER_DEFINED

namespace detail
{
inline constexpr std::uint32_t kTridiagonalPowerMod = kConsecutiveRecurrenceMod;

inline std::uint32_t TridiagonalResidue(std::int64_t value)
{
    const auto residue = value % kTridiagonalPowerMod;
    return static_cast<std::uint32_t>(residue < 0 ? residue + kTridiagonalPowerMod : residue);
}

inline std::uint32_t TridiagonalMultiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kTridiagonalPowerMod);
}

inline std::uint32_t TridiagonalAdd(std::uint32_t a, std::uint32_t b)
{
    const auto value = a + b;
    return value >= kTridiagonalPowerMod ? value - kTridiagonalPowerMod : value;
}

class TridiagonalContinuants
{
    using Polynomial = std::vector<std::uint32_t>;
    using Matrix = std::array<Polynomial, 4>;
    using Coefficients = std::span<const std::uint32_t>;

public:
    TridiagonalContinuants(std::span<const std::int64_t> diagonal, std::span<const std::int64_t> lower,
                           std::span<const std::int64_t> upper)
        : diagonal_(diagonal), lower_(lower), upper_(upper), tree_(2 * diagonal.size() - 1)
    {
        Build(0, 0, diagonal.size());
    }

    void WriteColumn(std::uint64_t exponent, Coefficients path_weights, std::span<std::uint32_t> output)
    {
        const auto side = diagonal_.size();
        auto reciprocal = ConsecutiveReciprocalSlice(std::move(tree_[0][0]), exponent - (side - 1), side, ntt_);
        WriteRange(0, 0, side, reciprocal, {}, path_weights, output);
    }

private:
    [[nodiscard]] Polynomial Product(Coefficients a, Coefficients b)
    {
        if (a.empty() || b.empty())
        {
            return {};
        }
        return ntt_.convolution_normalized(Polynomial(a.begin(), a.end()), Polynomial(b.begin(), b.end()));
    }

    [[nodiscard]] Polynomial Dot(const Polynomial &a, const Polynomial &b, const Polynomial &c, const Polynomial &d)
    {
        auto left = Product(a, b);
        auto right = Product(c, d);
        left.resize(std::max(left.size(), right.size()));
        for (std::size_t i = 0; i < right.size(); ++i)
        {
            left[i] = TridiagonalAdd(left[i], right[i]);
        }
        while (!left.empty() && left.back() == 0)
        {
            left.pop_back();
        }
        return left;
    }

    void Build(std::size_t node, std::size_t first, std::size_t last)
    {
        auto &matrix = tree_[node];
        if (last - first == 1)
        {
            matrix[0] = {1};
            const auto diagonal = TridiagonalResidue(diagonal_[first]);
            if (diagonal != 0)
            {
                matrix[0].push_back(ConsecutiveSubtract(0, diagonal));
            }
            if (first + 1 < diagonal_.size())
            {
                const auto product = TridiagonalMultiply(TridiagonalResidue(lower_[first]), TridiagonalResidue(upper_[first]));
                if (product != 0)
                {
                    matrix[1] = {0, 0, ConsecutiveSubtract(0, product)};
                }
            }
            matrix[2] = {1};
            return;
        }

        const auto middle = first + (last - first) / 2;
        const auto left = node + 1;
        const auto right = node + 2 * (middle - first);
        Build(left, first, middle);
        Build(right, middle, last);
        const auto &a = tree_[left];
        const auto &b = tree_[right];
        matrix[0] = Dot(a[0], b[0], a[1], b[2]);
        matrix[1] = Dot(a[0], b[1], a[1], b[3]);
        matrix[2] = Dot(a[2], b[0], a[3], b[2]);
        matrix[3] = Dot(a[2], b[1], a[3], b[3]);
    }

    static std::uint32_t Get(const Polynomial &coefficients, std::size_t index)
    {
        return index < coefficients.size() ? coefficients[index] : 0;
    }

    void WriteRange(std::size_t node, std::size_t first, std::size_t last,
                    Coefficients tail, Coefficients next_tail, Coefficients path_weights,
                    std::span<std::uint32_t> output)
    {
        if (last - first == 1)
        {
            output[first] = TridiagonalMultiply(path_weights[first], tail.front());
            return;
        }
        const auto middle = first + (last - first) / 2;
        const auto right = node + 2 * (middle - first);
        const auto right_size = last - middle;
        WriteRange(right, middle, last, tail.first(right_size),
                   next_tail.first(std::min(next_tail.size(), right_size + 1)), path_weights, output);

        // 左側の各係数に必要な区間だけ、右側の余因子を掛けて切り出す。
        const auto &matrix = tree_[right];
        const auto a = Product(matrix[0], tail);
        const auto b = Product(matrix[1], next_tail);
        const auto c = Product(matrix[2], tail);
        const auto d = Product(matrix[3], next_tail);
        Polynomial left(middle - first);
        Polynomial left_next(left.size() + 1);
        for (std::size_t i = 0; i < left.size(); ++i)
        {
            left[i] = TridiagonalAdd(Get(a, right_size + i), Get(b, right_size + 1 + i));
        }
        for (std::size_t i = 0; i < left_next.size(); ++i)
        {
            left_next[i] = TridiagonalAdd(Get(c, right_size - 1 + i), Get(d, right_size + i));
        }
        WriteRange(node + 1, first, middle, left, left_next, path_weights, output);
    }

    std::span<const std::int64_t> diagonal_;
    std::span<const std::int64_t> lower_;
    std::span<const std::int64_t> upper_;
    std::vector<Matrix> tree_;
    NTT<> ntt_;
};
} // namespace detail

[[nodiscard]] inline std::vector<std::uint32_t> tridiagonal_power_first_column(
    std::uint64_t exponent, std::span<const std::int64_t> lower, std::span<const std::int64_t> diagonal,
    std::span<const std::int64_t> upper)
{
    const auto side = diagonal.size();
    const auto off_diagonal = side == 0 ? 0 : side - 1;
    if (lower.size() != off_diagonal || upper.size() != off_diagonal)
    {
        throw std::invalid_argument("tridiagonal_power_first_column: diagonal sizes differ");
    }
    if (side == 0)
    {
        return {};
    }
    constexpr std::size_t kMaxSide = std::size_t{1} << 20;
    auto active = exponent < side ? static_cast<std::size_t>(exponent) + 1 : side;
    if (active > kMaxSide)
    {
        throw std::length_error("tridiagonal_power_first_column: too many active rows");
    }
    std::vector<std::uint32_t> result(side);
    if (exponent == 0)
    {
        result[0] = 1;
        return result;
    }
    std::vector<std::uint32_t> paths(active, 1);
    for (std::size_t i = 1; i < active; ++i)
    {
        const auto edge = detail::TridiagonalResidue(lower[i - 1]);
        if (edge == 0)
        {
            active = i; // この先の行には第0列から到達できない。
            break;
        }
        paths[i] = detail::TridiagonalMultiply(paths[i - 1], edge);
    }
    detail::TridiagonalContinuants system(diagonal.first(active), lower.first(active - 1), upper.first(active - 1));
    system.WriteColumn(exponent, paths, result);
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
