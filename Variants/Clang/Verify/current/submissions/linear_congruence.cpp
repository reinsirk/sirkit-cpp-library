#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc186/tasks/abc186_e

// Library: Math/Modular_Arithmetic.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
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


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MODULAR_ARITHMETIC_DETAIL_DEFINED
#define SIRKIT_MODULAR_ARITHMETIC_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
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
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(modulus);
    if (value >= 0) return static_cast<std::uint64_t>(value) % modulus;
    const auto residue = (std::uint64_t{0} - static_cast<std::uint64_t>(value)) % modulus;
    return residue ? modulus - residue : 0;
}
inline constexpr std::uint64_t ModAdd(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(modulus);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_add_residues64(a % modulus, b % modulus, modulus);
}
inline constexpr std::uint64_t ModSubtract(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(modulus);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_subtract_residues64(a % modulus, b % modulus, modulus);
}
inline constexpr std::uint64_t ModMultiply(std::uint64_t a, std::uint64_t b, std::uint64_t modulus) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(modulus);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(a, b, modulus);
}
inline constexpr std::uint64_t ModPow(std::uint64_t a, std::uint64_t exponent, std::uint64_t modulus) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(modulus);
    a %= modulus;
    if (!exponent) return 1 % modulus;
    if (a <= 1) return a;
    if (modulus <= (std::uint64_t{1} << 32))
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_power_residue64<true>(a, exponent, modulus);
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    if(exponent>=(std::uint64_t{1}<<15)&&(modulus&1))return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_montgomery_power64(a,exponent,modulus);
#endif
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_power_residue64<false>(a, exponent, modulus);
}
inline constexpr std::optional<std::uint64_t> ModInverse(std::uint64_t a, std::uint64_t modulus) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(modulus);
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
        const auto next = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_subtract_residues64(u0,
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(quotient, u1, modulus), modulus);
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
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(modulus);
    a %= modulus; b %= modulus;
    const auto divisor = std::gcd(a, modulus);
    if (b % divisor) return std::nullopt;
    const auto period = modulus / divisor;
    const auto inverse = ModInverse(a / divisor, period);
    return Congruence{/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(b / divisor, *inverse, period), period};
}
inline constexpr std::optional<Congruence> ChineseRemainder(Congruence first, Congruence second) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(first.modulus);
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(second.modulus);
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
    const auto multiplier = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(delta, *inverse, reduced_second);
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
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::check_modulus64(prime);
#ifndef SKIP_RUNTIME_CHECKS
    if (prime < 2 || (prime != 2 && !(prime & 1))) throw std::invalid_argument("ModSqrt: prime required");
#endif
    a %= prime;
    if (!a || prime == 2) return a;
    if(a==1)return a;
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
    if((prime>(std::uint64_t{1}<<32)||(prime>=(std::uint64_t{1}<<16)&&prime%8==1))&&(prime&1))return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_montgomery_sqrt64(a,prime);
#endif
    if (prime % 4 == 3) {
        const auto root = ModPow(a, prime / 4 + 1, prime);
        if(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(root,root,prime)!=a)return std::nullopt;
        return root < prime - root ? root : prime - root;
    }
    if(prime%8==5){
        auto root=ModPow(a,prime/8+1,prime);const auto square=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(root,root,prime);
        if(square!=a){
            if(square!=prime-a)return std::nullopt;
            root=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(root,ModPow(2,prime/4,prime),prime);
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
        auto value = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(t, t, prime);
        unsigned i = 1;
        while (i < remaining && value != 1) { value = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(value, value, prime); ++i; }
        if (i == remaining) return std::nullopt;
        auto factor = c;
        for (unsigned j = 0; j < remaining - i - 1; ++j) factor = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(factor, factor, prime);
        root = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(root, factor, prime);
        c = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(factor, factor, prime);
        t = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::modular_multiply64(t, c, prime);
        remaining = i;
    }
    return root < prime - root ? root : prime - root;
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
int main() {
    std::size_t tests{}; verify_in >> tests;
    while (tests--) {
        std::uint64_t n{}, s{}, k{}; verify_in >> n >> s >> k;
        const auto answer = SolveLinearCongruence(k, n - s, n);
        if (answer) verify_out << answer->remainder << '\n';
        else verify_out << -1 << '\n';
    }
}
