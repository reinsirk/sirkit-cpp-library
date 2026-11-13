#ifndef MODULAR_SOURCE
#define MODULAR_SOURCE "../../../Math/Modular_Arithmetic.cpp"
#endif
#include MODULAR_SOURCE
#include <array>
#include <iostream>
static_assert(NormalizeMod(INT64_MIN,UINT64_MAX)==(std::uint64_t{1}<<63)-1);
static_assert(ModAdd(UINT64_MAX-1,UINT64_MAX-1,UINT64_MAX)==UINT64_MAX-2);
static_assert(ModSubtract(0,UINT64_MAX-1,UINT64_MAX)==1);
static_assert(ModMultiply(UINT64_MAX,UINT64_MAX,UINT64_MAX-1)==1);
static_assert(ModMultiply(UINT64_MAX-1,UINT64_MAX-1,UINT64_MAX)==1);
static_assert(ModPow(0,0,17)==1&&ModPow(0,0,1)==0&&ModPow(0,1,17)==0);
static_assert(ModPow(1,UINT64_MAX,UINT64_MAX)==1);
static_assert(ModPow(2,64,UINT64_MAX)==1);
static_assert(ModPow(2,63,UINT64_MAX)==(std::uint64_t{1}<<63));
static_assert(ModInverse(1,1)==0);
static_assert(ModInverse(7,19)==11);
static_assert(ModInverse(2,UINT64_MAX)==(std::uint64_t{1}<<63));
static_assert(!ModInverse(6,9));
static_assert(SolveLinearCongruence(0,0,17)==Congruence{0,1});
static_assert(SolveLinearCongruence(6,8,14)==Congruence{6,7});
static_assert(!SolveLinearCongruence(6,3,14));
static_assert(ChineseRemainder(Congruence{2,6},Congruence{8,9})==Congruence{8,18});
static_assert(!ChineseRemainder(Congruence{1,2},Congruence{0,2}));
constexpr std::array<Congruence,3> equations{{{2,6},{8,9},{5,7}}};
static_assert(ChineseRemainder(equations)==Congruence{26,126});
static_assert(ChineseRemainder(std::span<const Congruence>{})==Congruence{0,1});
static_assert(ModSqrt(0,2)==0&&ModSqrt(1,2)==1);
static_assert(ModSqrt(10,13)==6&&ModSqrt(9,17)==3);
static_assert(!ModSqrt(2,13));
static_assert(ModSqrt(16,18446744069414584321ull)==4);
static_assert(reinsirk::detail::modular_multiply_full64(UINT64_MAX,UINT64_MAX,UINT64_MAX-1)==1);
constexpr std::uint64_t boundary_product_ref(std::uint64_t a,std::uint64_t b,std::uint64_t m){
    std::uint64_t result=0;a%=m;for(;b;b>>=1,a=(a+a)%m)if(b&1)result=(result+a)%m;return result;
}
constexpr std::uint64_t boundary_power_ref(std::uint64_t a,std::uint64_t exponent,std::uint64_t m){
    std::uint64_t result=1%m;for(;exponent;exponent>>=1,a=boundary_product_ref(a,a,m))if(exponent&1)result=boundary_product_ref(result,a,m);return result;
}
constexpr bool all_pow_boundaries(){
    for(const auto m:std::array<std::uint64_t,3>{UINT32_MAX,std::uint64_t{1}<<32,(std::uint64_t{1}<<32)+1})
        for(const auto exponent:std::array<std::uint64_t,6>{0,1,2,65535,65536,UINT64_MAX})
            if(ModPow(UINT32_MAX,exponent,m)!=boundary_power_ref(UINT32_MAX,exponent,m))return false;
    return true;
}
static_assert(all_pow_boundaries());
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__)) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
constexpr reinsirk::detail::ModularMontgomery64 additive(UINT64_MAX);
constexpr reinsirk::detail::ModularMontgomery64 subtractive(UINT64_MAX);
static_assert(additive.multiply(UINT64_MAX-1,UINT64_MAX-1)==1);
static_assert(additive.multiply(1,1)==1);
static_assert(subtractive.multiply(UINT64_MAX-1,UINT64_MAX-1)==1);
static_assert(additive.decode(additive.encode(UINT64_MAX-1))==UINT64_MAX-1);
static_assert(subtractive.decode(subtractive.encode(UINT64_MAX-1))==UINT64_MAX-1);
static_assert(additive.power<true>(2,64)==1&&additive.power<false>(2,64)==1);
static_assert(subtractive.power<true>(2,64)==1&&subtractive.power<false>(2,64)==1);
#endif
int main(){std::cout<<"PASS constexpr all public APIs, uint64 boundaries, small/large prime roots and standard arithmetic\n";}
