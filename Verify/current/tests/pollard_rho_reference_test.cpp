// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef POLLARD_HEADER
#define POLLARD_HEADER "../../../Math/Pollard_Rho.cpp"
#endif
#ifndef PRIMALITY_HEADER
#define PRIMALITY_HEADER "../../../Math/prime_judge(Miller-Rabin).cpp"
#endif
#ifdef SIRKIT_TEST_PRIMALITY_FIRST
#include PRIMALITY_HEADER
#endif
#include POLLARD_HEADER
#include POLLARD_HEADER
#include PRIMALITY_HEADER
#include <cstdio>
#include <cstdlib>
#include <random>
namespace {
std::uint64_t checks=0;void check(bool x){++checks;if(!x)std::abort();}
std::vector<std::uint64_t> trial(std::uint64_t n){std::vector<std::uint64_t>r;for(std::uint64_t d=2;d<=n/d;++d)while(n%d==0){r.push_back(d);n/=d;}if(n>1)r.push_back(n);return r;}
template<class F>void zero_reject(F f){bool yes=false;try{f();}catch(const std::domain_error&){yes=true;}check(yes);}
}
int main(){std::mt19937_64 rng(0x42d617801031ULL);PollardRho64 engine;
for(int rep=0;rep<50000;++rep){auto n=rng()|1;if(n==1)n=3;::sirkit::detail::FactorizationArithmetic arithmetic(n);const auto a=rng()%n,b=rng()%n;const auto ea=arithmetic.encode(a),eb=arithmetic.encode(b);
#if defined(__SIZEOF_INT128__)
 const auto expected=static_cast<std::uint64_t>(static_cast<__uint128_t>(a)*b%n);check(arithmetic.multiply(arithmetic.multiply(ea,eb),1)==expected);check(arithmetic.add(ea,eb)==static_cast<std::uint64_t>((static_cast<__uint128_t>(ea)+eb)%n));
#else
 auto sum=std::uint64_t{0},power=a;for(auto bits=b;bits;bits>>=1){if(bits&1)sum=sum>=n-power?sum-(n-power):sum+power;power=power>=n-power?power-(n-power):power+power;}check(arithmetic.multiply(arithmetic.multiply(ea,eb),1)==sum);
#endif
}
for(std::uint64_t n=1;n<=20000;++n){const auto factors=trial(n);check(engine.prime_factors(n)==factors);check(PollardRho64::is_prime(n)==(factors.size()==1));}
for(std::uint64_t n=1;n<=600;++n){std::vector<std::uint64_t>divisors;std::uint64_t phi=0;for(std::uint64_t d=1;d<=n;++d){if(n%d==0)divisors.push_back(d);phi+=std::gcd(d,n)==1;}check(engine.divisors(n)==divisors);auto unsorted=engine.divisors(n,false);std::sort(unsorted.begin(),unsorted.end());check(unsorted==divisors);check(engine.divisor_count(n)==divisors.size());check(engine.totient(n)==phi);const auto factors=trial(n);int mu=1;for(std::size_t i=0;i<factors.size();++i){if(i&&factors[i]==factors[i-1])mu=0;else mu=-mu;}check(engine.mobius(n)==mu);}
for(std::uint64_t n:{1ULL,2ULL,3ULL,41ULL,341550071728321ULL,3825123056546413051ULL,18446744073709551615ULL,18446744073709551557ULL,18446743979220271189ULL,18446744030759878681ULL,1000000016000000063ULL,9223372036854775808ULL}){const auto factors=engine.prime_factors(n);std::uint64_t remaining=n;for(auto prime:factors){check(is_prime(prime));check(remaining%prime==0);remaining/=prime;}check(remaining==1);check(PollardRho64::is_prime(n)==is_prime(n));}
for(int rep=0;rep<48;++rep){std::uint64_t p=(static_cast<std::uint32_t>(rng())|0x80000001u),q=(static_cast<std::uint32_t>(rng())|0x80000001u);while(!is_prime(p))p-=2;while(!is_prime(q))q-=2;const auto n=p*q;auto expected=std::vector<std::uint64_t>{p,q};std::sort(expected.begin(),expected.end());check(engine.prime_factors(n)==expected);check(engine.totient(n)==(p==q?p*(p-1):(p-1)*(q-1)));check(engine.divisor_count(n)==(p==q?3:4));check(engine.mobius(n)==(p==q?0:1));}
for(auto seed:{std::uint64_t{0},std::uint64_t{1},UINT64_MAX}){PollardRho64 a(seed),b=a;check(a.prime_factors(1000000016000000063ULL)==b.prime_factors(1000000016000000063ULL));}
check(PrimeFactors64(1).empty());check(Factorize64(72)==std::vector<std::pair<std::uint64_t,int>>({{2,3},{3,2}}));check(Divisors64(1)==std::vector<std::uint64_t>{1});check(DivisorCount64(1)==1);check(EulerPhi64(1)==1);check(Mobius64(1)==1);check(!PollardRho64::is_prime(0));zero_reject([&]{engine.prime_factors(0);});zero_reject([&]{engine.divisors(0);});zero_reject([&]{engine.totient(0);});zero_reject([&]{engine.mobius(0);});std::printf("Pollard reference checks=%ju\n",static_cast<std::uintmax_t>(checks));}
