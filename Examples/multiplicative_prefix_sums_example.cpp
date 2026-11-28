// SPDX-License-Identifier: CC0-1.0
#include "../Math/Multiplicative_Prefix_Sums.cpp"
#include <cstdint>
#include <iostream>

int main()
{
    const sirkit::MultiplicativePrefixSums sieve(10);
    auto primes = sieve.prime_prefix_sums([](std::uint64_t)
                                          {
                                              return std::uint64_t{1};
                                          },
                                          [](std::uint64_t x)
                                          {
                                              return x;
                                          });
    for (auto &count : primes)
    {
        count *= 2; // 約数個数は素数に対して2。
    }
    const auto sum = sieve.prefix_sum(primes, [](std::uint64_t, unsigned exponent)
                                      {
                                          return static_cast<std::uint64_t>(exponent + 1);
                                      });
    std::cout << sum << '\n'; // 27
}
