// SPDX-License-Identifier: CC0-1.0
#ifndef MODULAR_SOURCE
#define MODULAR_SOURCE "../../../Math/Modular_Arithmetic.cpp"
#endif
#ifndef SIRKIT_TEST_MODULAR_ARITHMETIC_LAST
#include MODULAR_SOURCE
#endif
#include "../../../Math/prime_judge(Miller-Rabin).cpp"
#include "../../../Math/Pollard_Rho.cpp"
#include "../../../Math/Linear_Recurrence.cpp"
#include "../../../Math/NTT.cpp"
#ifdef SIRKIT_TEST_MODULAR_ARITHMETIC_LAST
#include MODULAR_SOURCE
#endif

#include <cassert>
#include <iostream>
static_assert(ModPow(2,64,UINT64_MAX)==1);
static_assert(ModPow(UINT32_MAX,65535,std::uint64_t{1}<<32)==UINT32_MAX);
static_assert(ModPow(UINT32_MAX,65536,std::uint64_t{1}<<32)==1);
int main(){
 assert(ModPow(UINT64_MAX-1,UINT64_MAX,UINT64_MAX)==UINT64_MAX-1);
 assert(ModSqrt(16,UINT64_C(18446744073709551557))==4);
 assert(is_prime(UINT64_C(18446744073709551557)));
 assert(PollardRho64{}.prime_factors(8051)==std::vector<std::uint64_t>({83,97}));
 assert(LinearRecurrenceTerm(std::vector<std::uint32_t>{0,1},std::vector<std::uint32_t>{1,1},10)==55);
 assert(NTT<>{}.convolution(std::vector<int>{1,2},std::vector<int>{3,4})==std::vector<std::uint32_t>({3,10,8}));
 std::cout<<"PASS coexist Miller/Pollard/Recurrence/NTT/current Math both orders\n";
}
