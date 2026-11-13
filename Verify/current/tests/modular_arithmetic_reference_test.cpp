// SPDX-License-Identifier: CC0-1.0
#ifndef MODULAR_SOURCE
#define MODULAR_SOURCE "../../../Math/Modular_Arithmetic.cpp"
#endif
#include MODULAR_SOURCE
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using Big = boost::multiprecision::cpp_int;
using U = std::uint64_t;
std::uint64_t checks = 0;
void require(bool value) { if (!value) { std::cerr << "failed at " << checks << '\n'; std::abort(); } ++checks; }
Big norm(Big value, const Big& modulus) { value %= modulus; if (value < 0) value += modulus; return value; }
Big power(Big value, U exponent, const Big& modulus) {
    Big result = Big(1) % modulus; value %= modulus;
    while (exponent) { if (exponent & 1) result = result * value % modulus; value = value * value % modulus; exponent >>= 1; }
    return result;
}
std::pair<Big, Big> bezout(Big a, Big b) {
    Big x0 = 1, x1 = 0;
    while (b != 0) { Big q = a / b, next = a - q*b, x = x0-q*x1; a = b; b = next; x0=x1; x1=x; }
    return {a,x0};
}
void arithmetic(U a,U b,U m,U exponent) {
    const Big aa=a, bb=b, mm=m;
    require(ModAdd(a,b,m)==static_cast<U>((aa+bb)%mm));
    require(ModSubtract(a,b,m)==static_cast<U>(norm(aa-bb,mm)));
    require(ModMultiply(a,b,m)==static_cast<U>(aa*bb%mm));
    require(ModPow(a,exponent,m)==static_cast<U>(power(aa,exponent,mm)));
    auto [g,x]=bezout(aa,mm);auto inverse=ModInverse(a,m);
    require(inverse.has_value()==(g==1));
    if(inverse)require(*inverse==static_cast<U>(norm(x,mm)));
    auto linear=SolveLinearCongruence(a,b,m);
    require(linear.has_value()==(bb%g==0));
    if(linear) {
        const Big period=mm/g;
        auto [one,unit]=bezout(aa/g,period);require(one==1);
        require(linear->modulus==static_cast<U>(period));
        require(linear->remainder==static_cast<U>(norm((bb/g)*unit,period)));
    }
}
void crt(Congruence a,Congruence b) {
    const Big m=a.modulus,n=b.modulus,r=Big(a.remainder)%m,s=Big(b.remainder)%n;
    auto [g,x]=bezout(m,n);bool possible=(s-r)%g==0;
    const Big period=m/g*n;
    try {
        auto result=ChineseRemainder(a,b);
        require(result.has_value()==possible);
        if(possible) {
            require(period<=std::numeric_limits<U>::max());
            const Big multiplier=norm((s-r)/g*x,n/g);
            const Big expected=norm(r+m*multiplier,period);
            require(result->modulus==static_cast<U>(period));require(result->remainder==static_cast<U>(expected));
        }
    }catch(const std::overflow_error&){require(possible&&period>std::numeric_limits<U>::max());}
}
int main() {
    const U maximum=std::numeric_limits<U>::max();
    const std::array<U,12> values{0,1,2,3,15,UINT32_MAX-1,UINT32_MAX,U{1}<<32,(U{1}<<63)-1,U{1}<<63,maximum-1,maximum};
    const std::array<U,12> moduli{1,2,3,4,17,UINT32_MAX,U{1}<<32,(U{1}<<32)+1,(U{1}<<63)-1,U{1}<<63,maximum-1,maximum};
    for(auto a:values)for(auto b:values)for(auto m:moduli)arithmetic(a,b,m,37);
    for(auto m:moduli)for(std::int64_t x:{std::numeric_limits<std::int64_t>::min(),std::int64_t{-7},std::int64_t{-1},std::int64_t{0},std::int64_t{1},std::numeric_limits<std::int64_t>::max()})
        require(NormalizeMod(x,m)==static_cast<U>(norm(Big(x),Big(m))));
    arithmetic(UINT64_C(7540113804746346429),UINT64_MAX,UINT64_C(12200160415121876738),UINT64_MAX);
    arithmetic(UINT64_C(12200160415121876738),UINT64_MAX,UINT64_C(7540113804746346429),UINT64_MAX);
    std::mt19937_64 random(0x41e7df135);
    for(unsigned i=0;i<12000;++i){auto a=random(),b=random(),m=random(),e=random();if(!m)m=1;arithmetic(a,b,m,e);}
    for(U m=1;m<=18;++m)for(U n=1;n<=18;++n)for(U a=0;a<m;++a)for(U b=0;b<n;++b) {
        auto result=ChineseRemainder({a,m},{b,n});U period=std::lcm(m,n),first=period;
        for(U x=0;x<period;++x)if(x%m==a&&x%n==b){first=x;break;}
        require(result.has_value()==(first<period));if(result){require(result->remainder==first);require(result->modulus==period);}
    }
    for(unsigned i=0;i<5000;++i) {
        auto a=random(),b=random(),m=random(),n=random();if(!m)m=1;if(!n)n=1;
        if(i%3==0){m=m%100000+1;n=n%100000+1;}
        crt({a,m},{b,n});
    }
    for(auto m:moduli)for(auto n:moduli)for(auto r:values)crt({r,m},{maximum-r,n});
    require(ChineseRemainder(std::span<const Congruence>{})==std::optional<Congruence>{Congruence{0,1}});
    const std::array<Congruence,3> equations{{{2,6},{8,9},{5,7}}};
    require(ChineseRemainder(equations)==std::optional<Congruence>{Congruence{26,126}});
    const std::array<Congruence,3> stopped{{{1,2},{0,2},{0,0}}};require(!ChineseRemainder(stopped));
    for(U prime=2;prime<=1000;++prime){bool is_prime=true;for(U d=2;d*d<=prime;++d)if(prime%d==0)is_prime=false;if(!is_prime)continue;
        std::vector<U> roots(prime,prime);for(U x=0;x<prime;++x)roots[x*x%prime]=std::min(roots[x*x%prime],x);
        for(U a=0;a<prime*2;++a){auto result=ModSqrt(a,prime);require(result.has_value()==(roots[a%prime]!=prime));if(result)require(*result==roots[a%prime]);}
    }
    for(U p:{U{998244353},U{1000000007},U{2305843009213693951},U{18446744069414584321ULL},U{18446744073709551557ULL}}) {
        for(unsigned i=0;i<80;++i){auto a=random();if(i%2==0)a=static_cast<U>(Big(a)*a%p);
            auto root=ModSqrt(a,p);const bool has=a%p==0||power(Big(a),(p-1)/2,Big(p))==1;require(root.has_value()==has);
            if(root){require(Big(*root)*(*root)%p==Big(a)%p);require(*root<=p-*root);}
        }
    }
    for(auto function:{+[](U a,U b,U m){return ModAdd(a,b,m);},+[](U a,U b,U m){return ModSubtract(a,b,m);},+[](U a,U b,U m){return ModMultiply(a,b,m);},+[](U a,U b,U m){return ModPow(a,b,m);},+[](U a,U,U m){(void)ModInverse(a,m);return U{0};},+[](U a,U b,U m){(void)SolveLinearCongruence(a,b,m);return U{0};},+[](U a,U b,U m){(void)ChineseRemainder({a,m},{b,1});return U{0};},+[](U a,U,U m){return NormalizeMod(static_cast<std::int64_t>(a),m);}}) {
        bool threw=false;try{(void)function(1,2,0);}catch(const std::invalid_argument&){threw=true;}require(threw);
    }
    bool zero_threw=false;try{(void)ModSqrt(1,0);}catch(const std::invalid_argument&){zero_threw=true;}require(zero_threw);
#ifndef SKIP_RUNTIME_CHECKS
    bool threw=false;try{(void)ModSqrt(1,1);}catch(const std::invalid_argument&){threw=true;}require(threw);
#endif
    std::cout<<"PASS modular checks="<<checks<<"; multiprecision Euclid/CRT/power, exhaustive CRT/square roots, uint64 boundaries\n";
}
