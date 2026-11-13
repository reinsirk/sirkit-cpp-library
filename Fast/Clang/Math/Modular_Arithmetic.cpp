// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Modular_Arithmetic.cpp
// 参考（機能とVerify問題）: https://judge.yosupo.jp/problem/sqrt_mod
// 独立実装。整数演算・Euclidの互除法・中国剰余定理・Tonelli–Shanks法を用いる。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_arithmetic.md
// API一覧
// 法mは1..UINT64_MAX。以下の戻り値は特記しない限り0..m-1。引数a,bは正規化不要。
// NormalizeMod(int64_t a,uint64_t m): 負数を含めた標準剰余。O(1)。INT64_MINにも対応する。
// ModAdd/ModSubtract/ModMultiply(uint64_t a,b,m): 和/差/積の剰余。符号なし64bit全域で中間オーバーフローを避ける。
// ModPow(uint64_t a,exponent,m): a^exponent mod m。0^0は1 mod m。ModMultiplyをO(log(exponent+2))回使う。
// 以下のL(m)はModMultiply1回の費用。ModInverse(a,m): optional<uint64_t>、逆元なしはnullopt、m=1では0。
// ModInverseはO(log(m+1)*L(m))時間。型はuint64_t。入力は正規化不要。
// Congruence {uint64_t remainder=0,modulus=1}: x=remainder (mod modulus)。modulus>0が前提。==は両フィールドを比較。
// SolveLinearCongruence(uint64_t a,b,m): ax=b (mod m)のoptional<Congruence>。最小非負解と周期、不能ならnullopt。
// SolveLinearCongruenceはModInverseと同計算量。CRT2式はM=max(両法)としてO(log(M+1)*L(M))時間。
// ChineseRemainder(Congruence a,b): 両合同式を統合。法が互いに素でなくても可。不能ならnullopt。
// ChineseRemainder(span<const Congruence>): 順に統合し、空なら{0,1}。時間は各統合費用の合計。
// 全体のLCMがuint64_tに収まることが前提。全APIの追加領域はO(1)。
// CRTで解がある統合段階のLCMがuint64_tを超えると、検査省略時もoverflow_error。不能を検出した後は終了する。
// ModSqrt(uint64_t a,prime): x²=a (mod prime)のoptional<uint64_t>。根があれば小さい方、なければnullopt。
// primeは素数が前提（素数判定は行わない）。2にも対応。z個の非平方剰余候補を調べる場合、O((z+log p)log p)回の剰余乗算。
// ModMultiplyは拡張128bit整数が使えればO(1)、標準代替はO(log m)。ModAdd/Subtract/NormalizeはO(1)。
// その他の計算量は上記の剰余乗算1回の費用を含める。全APIは入力不変。正常終了は動的確保なし。
// SIRKIT_FORCE_STANDARD_ARITHMETICで128bit拡張を除外。SKIP_RUNTIME_CHECKSで素数候補の形の入力検査を除く。
// 全APIと内部演算はconstexpr。正常な入力はコンパイラの定数式評価予算内で定数式にも使える。
// GCC/Clangの128bit拡張では、奇数法>2^32・指数>=2^15の累乗にMontgomery還元を使う。
// 専用補助を外へ分け、小さい法のinline経路を保つ。標準代替とconstexprも同じ結果を返す。
// ModSqrtはp%4==3・p%8==5を直接式と二乗で判定し、その他はTonelli–Shanks法を使う。
// 128bit拡張ではp>2^32、またはp>=2^16かつp%8==1のとき、Montgomery表現を保って計算する。
// 法0は演算を定義できないため、検査省略時もinvalid_argument。
// 素数性や後続の未参照合同式は検査しない。不能の判定・容量保護は検査省略時も維持する。

#include <bit>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>

#ifndef SIRKIT_MODULAR_ARITHMETIC_DETAIL_DEFINED
#define SIRKIT_MODULAR_ARITHMETIC_DETAIL_DEFINED
namespace reinsirk::detail {
inline constexpr void check_modulus64(std::uint64_t modulus) {
    if (!modulus) throw std::invalid_argument("modular arithmetic: zero modulus");
}
inline constexpr std::uint64_t modular_add_residues64(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) noexcept {
    return a >= modulus - b ? a - (modulus - b) : a + b;
}
inline constexpr std::uint64_t modular_subtract_residues64(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) noexcept {
    return a >= b ? a - b : modulus - (b - a);
}
inline constexpr std::uint64_t modular_multiply_full64(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) noexcept {
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    return static_cast<std::uint64_t>(static_cast<unsigned __int128>(a) * b % modulus);
#else
    a %= modulus; b %= modulus;
    if (a < b) std::swap(a, b);
    std::uint64_t result = 0;
    while (b) {
        if (b & 1) result = modular_add_residues64(result, a, modulus);
        b >>= 1;
        if (b) a = modular_add_residues64(a, a, modulus);
    }
    return result;
#endif
}
inline constexpr std::uint64_t modular_multiply64(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) noexcept {
    if ((a | b) <= std::numeric_limits<std::uint32_t>::max()) return a * b % modulus;
    return modular_multiply_full64(a,b,modulus);
}
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
#ifndef SIRKIT_MODULAR_MONTGOMERY64_DEFINED
#define SIRKIT_MODULAR_MONTGOMERY64_DEFINED
// 奇数法・正規化済みの2項。減算REDCの値は(-modulus,modulus)内で、129bitの和を作らない。
class ModularMontgomery64 {
    using Wide=unsigned __int128;
    std::uint64_t modulus_,inverse_,r_squared_;
public:
    explicit constexpr ModularMontgomery64(std::uint64_t modulus) noexcept
        :modulus_(modulus),inverse_(modulus),r_squared_(static_cast<std::uint64_t>((Wide{0}-modulus)%modulus)) {
        // 奇数の二乗はmod8で1。5回のNewton反復で64bit以上の逆元を得る。
        for(unsigned i=0;i<5;++i)inverse_*=2u-modulus_*inverse_;
    }
    constexpr std::uint64_t multiply(std::uint64_t a,std::uint64_t b)const noexcept {
        const Wide product=Wide{a}*b;
        const auto high=static_cast<std::uint64_t>(product>>64);
        const auto correction=static_cast<std::uint64_t>((Wide{static_cast<std::uint64_t>(product)*inverse_}*modulus_)>>64);
        const auto value=high-correction;
        return high<correction?value+modulus_:value;
    }
    constexpr std::uint64_t encode(std::uint64_t residue)const noexcept{return multiply(residue,r_squared_);}
    constexpr std::uint64_t decode(std::uint64_t residue)const noexcept{return multiply(residue,1);}
    template<bool LeftToRight=true>
    constexpr std::uint64_t power(std::uint64_t residue,std::uint64_t exponent)const noexcept {
        if(!exponent)return 1%modulus_;
        auto base=encode(residue);
        if constexpr(LeftToRight){
            auto value=base;
            for(auto bit=std::bit_floor(exponent)>>1;bit;bit>>=1){value=multiply(value,value);if(exponent&bit)value=multiply(value,base);}
            return decode(value);
        }else{
            auto value=encode(1);
            while(exponent){if(exponent&1)value=multiply(value,base);exponent>>=1;if(exponent)base=multiply(base,base);}
            return decode(value);
        }
    }
    // 入出力ともMontgomery表現。sqrt中で表現を保ち、変換を繰り返さない。
    template<bool LeftToRight>
    constexpr std::uint64_t power_encoded(std::uint64_t base,std::uint64_t exponent)const noexcept {
        if(!exponent)return encode(1);
        if constexpr(LeftToRight){
            auto value=base;
            for(auto bit=std::bit_floor(exponent)>>1;bit;bit>>=1){value=multiply(value,value);if(exponent&bit)value=multiply(value,base);}
            return value;
        }else{
            auto value=encode(1);
            while(exponent){if(exponent&1)value=multiply(value,base);exponent>>=1;if(exponent)base=multiply(base,base);}
            return value;
        }
    }
};
[[gnu::noinline]] inline constexpr std::uint64_t modular_montgomery_power64(std::uint64_t a,std::uint64_t exponent,std::uint64_t modulus) noexcept {
    const ModularMontgomery64 arithmetic(modulus);
    if(std::has_single_bit(exponent))return arithmetic.power<true>(a,exponent);
    return arithmetic.power<false>(a,exponent);
}

[[gnu::noinline]] inline constexpr std::optional<std::uint64_t> modular_montgomery_sqrt64(std::uint64_t a,std::uint64_t prime) noexcept {
    const ModularMontgomery64 arithmetic(prime);const auto one=arithmetic.encode(1),encoded_a=arithmetic.encode(a);
    const auto power=[&arithmetic](std::uint64_t base,std::uint64_t exponent)constexpr noexcept {
        if(std::has_single_bit(exponent))return arithmetic.power_encoded<true>(base,exponent);
        return arithmetic.power_encoded<false>(base,exponent);
    };
    if(prime%4==3){
        const auto encoded_root=power(encoded_a,prime/4+1);
        if(arithmetic.multiply(encoded_root,encoded_root)!=encoded_a)return std::nullopt;
        const auto root=arithmetic.decode(encoded_root);return root<prime-root?root:prime-root;
    }
    // p=8k+5: r=a^(k+1)。r²=-aなら2^((p-1)/4)を掛けて補正する。
    if(prime%8==5){
        auto root=power(encoded_a,prime/8+1);const auto square=arithmetic.multiply(root,root);
        if(square!=encoded_a){
            if(square!=prime-encoded_a)return std::nullopt;
            root=arithmetic.multiply(root,power(arithmetic.encode(2),prime/4));
        }
        const auto result=arithmetic.decode(root);return result<prime-result?result:prime-result;
    }
    if(power(encoded_a,(prime-1)/2)!=one)return std::nullopt;
    const auto shifts=static_cast<unsigned>(std::countr_zero(prime-1));const auto odd=(prime-1)>>shifts;
    std::uint64_t non_residue=2;
    while(power(arithmetic.encode(non_residue),(prime-1)/2)!=prime-one)++non_residue;
    auto c=power(arithmetic.encode(non_residue),odd),root=power(encoded_a,(odd+1)/2),t=power(encoded_a,odd);auto remaining=shifts;
    while(t!=one){
        auto value=arithmetic.multiply(t,t);unsigned i=1;
        while(i<remaining&&value!=one){value=arithmetic.multiply(value,value);++i;}
        if(i==remaining)return std::nullopt;
        auto factor=c;for(unsigned j=0;j<remaining-i-1;++j)factor=arithmetic.multiply(factor,factor);
        root=arithmetic.multiply(root,factor);c=arithmetic.multiply(factor,factor);t=arithmetic.multiply(t,c);remaining=i;
    }
    const auto result=arithmetic.decode(root);return result<prime-result?result:prime-result;
}

#endif
#endif

template<bool Narrow>
inline constexpr std::uint64_t modular_power_residue64(std::uint64_t base, std::uint64_t exponent, std::uint64_t modulus) noexcept {
    std::uint64_t result = 1 % modulus;
    while (exponent) {
        if (exponent & 1) {
            if constexpr (Narrow) result = result * base % modulus;
            else result = modular_multiply_full64(result, base, modulus);
        }
        exponent >>= 1;
        if (exponent) {
            if constexpr (Narrow) base = base * base % modulus;
            else base = modular_multiply_full64(base, base, modulus);
        }
    }
    return result;
}
}
#endif

#ifndef SIRKIT_MODULAR_ARITHMETIC_DEFINED
#define SIRKIT_MODULAR_ARITHMETIC_DEFINED
inline constexpr std::uint64_t NormalizeMod(std::int64_t value, std::uint64_t modulus) {
    reinsirk::detail::check_modulus64(modulus);
    if (value >= 0) return static_cast<std::uint64_t>(value) % modulus;
    const auto residue = (std::uint64_t{0} - static_cast<std::uint64_t>(value)) % modulus;
    return residue ? modulus - residue : 0;
}
inline constexpr std::uint64_t ModAdd(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
    reinsirk::detail::check_modulus64(modulus);
    return reinsirk::detail::modular_add_residues64(a % modulus, b % modulus, modulus);
}
inline constexpr std::uint64_t ModSubtract(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
    reinsirk::detail::check_modulus64(modulus);
    return reinsirk::detail::modular_subtract_residues64(a % modulus, b % modulus, modulus);
}
inline constexpr std::uint64_t ModMultiply(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
    reinsirk::detail::check_modulus64(modulus);
    return reinsirk::detail::modular_multiply64(a, b, modulus);
}
inline constexpr std::uint64_t ModPow(std::uint64_t a, std::uint64_t exponent, std::uint64_t modulus) {
    reinsirk::detail::check_modulus64(modulus);
    a %= modulus;
    if (!exponent) return 1 % modulus;
    if (a <= 1) return a;
    if (modulus <= (std::uint64_t{1} << 32))
        return reinsirk::detail::modular_power_residue64<true>(a, exponent, modulus);
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    if(exponent>=(std::uint64_t{1}<<15)&&(modulus&1))return reinsirk::detail::modular_montgomery_power64(a,exponent,modulus);
#endif
    return reinsirk::detail::modular_power_residue64<false>(a, exponent, modulus);
}
inline constexpr std::optional<std::uint64_t> ModInverse(std::uint64_t a, std::uint64_t modulus) {
    reinsirk::detail::check_modulus64(modulus);
    a %= modulus;
    auto r0 = modulus, r1 = a;
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    __int128 u0 = 0, u1 = 1;
    while (r1) {
        const auto quotient = r0 / r1, remainder = r0 % r1;
        // Euclidの係数は絶対値modulus以下。中間値も符号付き128bitに収まる。
        const auto next = u0 - static_cast<__int128>(quotient) * u1;
        r0 = r1; r1 = remainder; u0 = u1; u1 = next;
    }
    if (r0 != 1) return std::nullopt;
    if (u0 < 0) u0 += modulus;
    return static_cast<std::uint64_t>(u0);
#else
    std::uint64_t u0 = 0, u1 = 1 % modulus;
    while (r1) {
        const auto quotient = r0 / r1, remainder = r0 % r1;
        const auto next = reinsirk::detail::modular_subtract_residues64(u0,
            reinsirk::detail::modular_multiply64(quotient, u1, modulus), modulus);
        r0 = r1; r1 = remainder; u0 = u1; u1 = next;
    }
    if (r0 != 1) return std::nullopt;
    return u0;
#endif
}
struct Congruence {
    std::uint64_t remainder = 0, modulus = 1;
    friend constexpr bool operator==(const Congruence&, const Congruence&) = default;
};
inline constexpr std::optional<Congruence> SolveLinearCongruence(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
    reinsirk::detail::check_modulus64(modulus);
    a %= modulus; b %= modulus;
    const auto divisor = std::gcd(a, modulus);
    if (b % divisor) return std::nullopt;
    const auto period = modulus / divisor;
    const auto inverse = ModInverse(a / divisor, period);
    return Congruence{reinsirk::detail::modular_multiply64(b / divisor, *inverse, period), period};
}
inline constexpr std::optional<Congruence> ChineseRemainder(Congruence first, Congruence second) {
    reinsirk::detail::check_modulus64(first.modulus);
    reinsirk::detail::check_modulus64(second.modulus);
    first.remainder %= first.modulus; second.remainder %= second.modulus;
    const auto divisor = std::gcd(first.modulus, second.modulus);
    if (first.remainder % divisor != second.remainder % divisor) return std::nullopt;
    const auto reduced_second = second.modulus / divisor;
    if (first.modulus > std::numeric_limits<std::uint64_t>::max() / reduced_second)
        throw std::overflow_error("ChineseRemainder: modulus overflow");
    const auto combined_modulus = first.modulus * reduced_second;
    if (reduced_second == 1) return Congruence{first.remainder, combined_modulus};
    std::uint64_t delta;
    if (second.remainder >= first.remainder) delta = (second.remainder - first.remainder) / divisor % reduced_second;
    else {
        const auto residue = (first.remainder - second.remainder) / divisor % reduced_second;
        delta = residue ? reduced_second - residue : 0;
    }
    const auto inverse = ModInverse(first.modulus / divisor, reduced_second);
    const auto multiplier = reinsirk::detail::modular_multiply64(delta, *inverse, reduced_second);
    return Congruence{first.remainder + first.modulus * multiplier, combined_modulus};
}
inline constexpr std::optional<Congruence> ChineseRemainder(std::span<const Congruence> equations) {
    Congruence result;
    for (auto equation : equations) {
        const auto next = ChineseRemainder(result, equation);
        if (!next) return std::nullopt;
        result = *next;
    }
    return result;
}
inline constexpr std::optional<std::uint64_t> ModSqrt(std::uint64_t a, std::uint64_t prime) {
    reinsirk::detail::check_modulus64(prime);
    a %= prime;
    if (!a || prime == 2) return a;
    if(a==1)return a;
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    if((prime>(std::uint64_t{1}<<32)||(prime>=(std::uint64_t{1}<<16)&&prime%8==1))&&(prime&1))return reinsirk::detail::modular_montgomery_sqrt64(a,prime);
#endif
    if (prime % 4 == 3) {
        const auto root = ModPow(a, prime / 4 + 1, prime);
        if(reinsirk::detail::modular_multiply64(root,root,prime)!=a)return std::nullopt;
        return root < prime - root ? root : prime - root;
    }
    if(prime%8==5){
        auto root=ModPow(a,prime/8+1,prime);const auto square=reinsirk::detail::modular_multiply64(root,root,prime);
        if(square!=a){
            if(square!=prime-a)return std::nullopt;
            root=reinsirk::detail::modular_multiply64(root,ModPow(2,prime/4,prime),prime);
        }
        return root<prime-root?root:prime-root;
    }
    if (ModPow(a, (prime - 1) / 2, prime) != 1) return std::nullopt;
    const auto shifts = static_cast<unsigned>(std::countr_zero(prime - 1));
    const auto odd = (prime - 1) >> shifts;
    std::uint64_t non_residue = 2;
    while (ModPow(non_residue, (prime - 1) / 2, prime) != prime - 1) ++non_residue;
    auto c = ModPow(non_residue, odd, prime), root = ModPow(a, (odd + 1) / 2, prime), t = ModPow(a, odd, prime);
    auto remaining = shifts;
    while (t != 1) {
        auto value = reinsirk::detail::modular_multiply64(t, t, prime);
        unsigned i = 1;
        while (i < remaining && value != 1) { value = reinsirk::detail::modular_multiply64(value, value, prime); ++i; }
        if (i == remaining) return std::nullopt;
        auto factor = c;
        for (unsigned j = 0; j < remaining - i - 1; ++j) factor = reinsirk::detail::modular_multiply64(factor, factor, prime);
        root = reinsirk::detail::modular_multiply64(root, factor, prime);
        c = reinsirk::detail::modular_multiply64(factor, factor, prime);
        t = reinsirk::detail::modular_multiply64(t, c, prime);
        remaining = i;
    }
    return root < prime - root ? root : prime - root;
}
#endif
