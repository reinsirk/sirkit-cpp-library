// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/NTT.cpp
// 参考（NTTの利用範囲・Verify問題。バタフライと所有構造は独自実装）:
// https://nyaannyaan.github.io/library/ntt/ntt.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/ntt.md
//
// API一覧
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
// SIRKIT_NAMESPACE_END_V1
