// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Prime_Counting.cpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

int main()
{
    try
    {
        sirkit::detail::prime_count_table_size(10, 10);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    constexpr int kLimit = 1000000;
    std::vector<bool> prime(kLimit + 1, true);
    prime[0] = prime[1] = false;
    for (int p = 2; p * p <= kLimit; ++p)
    {
        if (prime[p])
        {
            for (int j = p * p; j <= kLimit; j += p)
            {
                prime[j] = false;
            }
        }
    }
    std::vector<std::uint64_t> count(kLimit + 1);
    for (int i = 1; i <= kLimit; ++i)
    {
        count[i] = count[i - 1] + prime[i];
    }
    for (int n = 0; n <= 4000; ++n)
    {
        assert(sirkit::count_primes(n) == count[n]);
    }
    std::mt19937 random(1567);
    for (int i = 0; i < 160; ++i)
    {
        const auto n = random() % (kLimit + 1);
        assert(sirkit::count_primes(n) == count[n]);
    }
    assert(sirkit::count_primes(100000000) == 5761455);
}
