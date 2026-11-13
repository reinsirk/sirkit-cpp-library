// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
int main() {
    std::uint64_t n,a,b;verify_in>>n>>a>>b;
    PrimeSieve sieve(n);
    const auto size=sieve.prime_count();const auto x=size>b?(size-1-b)/a+1:0;
    verify_out<<size<<' '<<x<<'\n';
    sieve.for_each_prime_stride(b, a, [&](std::uint64_t prime) { verify_out << prime << ' '; });
    verify_out<<'\n';
}
