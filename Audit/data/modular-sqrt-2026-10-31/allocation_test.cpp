// SPDX-License-Identifier: CC0-1.0
#ifndef MODULAR_SOURCE
#define MODULAR_SOURCE "/tmp/sirkit_12h_20261031_095733/modular_arithmetic/modular_arithmetic.hpp"
#endif
#include MODULAR_SOURCE
#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <new>
static bool forbid_allocation = false;
void* operator new(std::size_t bytes) {
    if (forbid_allocation) std::abort();
    if (auto p = std::malloc(bytes ? bytes : 1)) return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void* operator new[](std::size_t bytes) { return ::operator new(bytes); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
int main() {
    forbid_allocation = true;
    std::uint64_t checksum = 0;
    const std::array<Congruence,3> equations{{{2,6},{8,9},{5,7}}};
    for(std::uint64_t i=1;i<=1000;++i) {
        checksum ^= NormalizeMod(-static_cast<std::int64_t>(i),UINT64_MAX);
        checksum ^= ModAdd(i,UINT64_MAX-i,UINT64_MAX);
        checksum ^= ModSubtract(i,UINT64_MAX-i,UINT64_MAX);
        checksum ^= ModMultiply(UINT64_MAX-i,i,UINT64_MAX);
        checksum ^= ModPow(i,79,UINT64_MAX);
        checksum ^= ModPow(i,UINT64_MAX,UINT64_MAX);
        auto wide_root=ModSqrt(i*i,UINT64_C(18446744073709551557));assert(wide_root);checksum^=*wide_root;
        auto inverse=ModInverse(i,998244353);assert(inverse);checksum^=*inverse;
        auto linear=SolveLinearCongruence(i,7,998244353);assert(linear);checksum^=linear->remainder;
        auto combined=ChineseRemainder(equations);assert(combined);checksum^=combined->remainder;
        auto root=ModSqrt(i*i,998244353);assert(root);checksum^=*root;
    }
    forbid_allocation = false;
    std::cout << "PASS no allocations; checksum="<<checksum<<'\n';
}
