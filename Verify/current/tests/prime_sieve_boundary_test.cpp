// SPDX-License-Identifier: CC0-1.0
#ifndef SIEVE_SOURCE
#define SIEVE_SOURCE "../../../Math/Eratosthenes.cpp"
#endif
#include SIEVE_SOURCE
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <source_location>
#include <stdexcept>
#include <utility>
#include <vector>

namespace independent {
std::size_t total = 0, case_count = 0, primality_checks = 0;
void check(bool condition, std::source_location at = std::source_location::current()) {
    ++total;
    if (!condition) {
        std::fprintf(stderr, "independent failure at line %u, check %zu\n", at.line(), total);
        std::abort();
    }
}
struct Oracle {
    std::vector<std::uint8_t> flags;
    std::vector<std::uint64_t> primes;
    explicit Oracle(std::size_t n) : flags(n + 1, 1) {
        flags[0] = flags[1] = 0;
        for (std::size_t p = 2; p <= n / p; ++p) {
            if (!flags[p]) continue;
            for (std::size_t x = p * p; x <= n; x += p) flags[x] = 0;
        }
        for (std::size_t x = 2; x <= n; ++x) if (flags[x]) primes.push_back(x);
    }
    std::vector<std::uint64_t> expected(std::uint64_t n) const {
        return {primes.begin(), std::upper_bound(primes.begin(), primes.end(), n)};
    }
};
void boundaries(const Oracle& oracle) {
    std::set<std::uint64_t> limits;
    for (std::uint64_t n = 0; n < 256; ++n) limits.insert(n);
    const auto around = [&](std::uint64_t n, int radius) {
        for (int delta = -radius; delta <= radius; ++delta)
            limits.insert(static_cast<std::uint64_t>(static_cast<std::int64_t>(n) + delta));
    };
    for (std::uint64_t n : {3969,8192,16384,262144,524288,1048576,1572864,2097152,4194304})
        around(n, 3);
    for (std::uint64_t n : {32640,32768,32896,65408,65536,131072,147840,295680,443520})
        around(n, 4);
    for (std::uint64_t p : {3,5,7,11,13,17,19,23,29,31,37,41,43,47,61,97,127,509,521,1021,1447,2039})
        around(p * p, 2);
    around(oracle.primes[131071], 3);
    around(oracle.primes[131072], 3);
    std::printf("MEDIUM_BOUNDARY primes[131071]=%ju primes[131072]=%ju\n",
        static_cast<std::uintmax_t>(oracle.primes[131071]),
        static_cast<std::uintmax_t>(oracle.primes[131072]));
    // 新しいmask対象素数の平方と、4096語blockからの位相・末尾を独立照合する。
    for (std::uint64_t prime : {37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131}) {
        around(prime * prime, 2);
        for (std::uint64_t block : {1,3,7}) {
            for (int tail : {-1,0,1}) {
                const auto words = 4096 * block + prime + tail;
                around(words * 128, 1);
            }
        }
    }
    // reserve/insert初期化の1155語周期、その最終語のbit端点。
    for (std::uint64_t periods : {1,2,3,7,14,28}) {
        for (int tail : {-1,0,1}) {
            const auto words = periods * 1155 + tail;
            around(words * 128, 1);
        }
    }
    // 融合maskの周期・非零prefix終了・完全周期の直後を各blockで確認する。
    for (std::uint64_t block_words : {4096,8192}) {
        for (std::uint64_t block : {1,3,7}) {
            const auto first_word = block_words * block;
            if (first_word * 128 > 4200000) continue;
            for (std::uint64_t period : {221,437,899,1517}) {
                const auto prefix = (period - first_word % period) % period;
                for (std::uint64_t full_periods : {0,1}) {
                    for (int delta : {-1,0,1}) {
                        const auto tail = static_cast<std::int64_t>(prefix + full_periods * period) + delta;
                        if (tail < 0) continue;
                        const auto limit = (first_word + static_cast<std::uint64_t>(tail)) * 128;
                        if (limit + 1 <= 4200000) around(limit, 1);
                    }
                }
            }
        }
    }
    std::mt19937_64 random(0xCCEAA281ULL);
    for (unsigned round = 0; round < 20; ++round) limits.insert(random() % 4194305);
    for (std::uint64_t n : limits) {
        ++case_count;
        PrimeSieve sieve(n);
        const auto expected = oracle.expected(n);
        check(sieve.limit() == n);
        check(sieve.prime_count() == expected.size());
        check(sieve.primes() == expected);
        std::vector<std::uint64_t> appended{0, 41, 0};
        sieve.append_to(appended);
        check(appended.size() == expected.size() + 3);
        check(appended[0] == 0 && appended[1] == 41 && appended[2] == 0);
        check(std::equal(expected.begin(), expected.end(), appended.begin() + 3));
        std::size_t visited = 0;
        sieve.for_each_prime([&](std::uint64_t value) {
            if (visited >= expected.size() || value != expected[visited]) std::abort();
            ++visited;
        });
        check(visited == expected.size());
        auto primality = [&](std::uint64_t x) {
            ++primality_checks;
            check(sieve.is_prime(x) == (oracle.flags[x] != 0));
        };
        if (n < 256 || n == 262144 || n == 4194304) {
            for (std::uint64_t x = 0; x <= n; ++x) primality(x);
        } else {
            for (std::uint64_t x = 0; x <= std::min(n,std::uint64_t{512}); ++x) primality(x);
            for (std::uint64_t x = n > 512 ? n - 512 : 0; x <= n; ++x) primality(x);
            for (unsigned sample = 0; sample < 512; ++sample) primality(random() % (n + 1));
        }
#ifndef SKIP_RUNTIME_CHECKS
        bool threw = false;
        try { (void)sieve.is_prime(n + 1); } catch (const std::out_of_range&) { threw = true; }
        check(threw);
#endif
        if (n < 256 || n == 262144 || n == 524288 || n == 1048576) {
            check(eratosthenes(n) == expected);
            std::vector<std::uint64_t> sink{17, 17};
            eratosthenes(n, sink);
            check(sink.size() == expected.size() + 2);
            check(sink[0] == 17 && sink[1] == 17);
            check(std::equal(expected.begin(), expected.end(), sink.begin() + 2));
        }
    }
}
void carry_words() {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    if (!__builtin_cpu_supports("avx2")) return;
    std::vector<std::uint64_t> data(65536 + 8);
    std::vector<std::size_t> lengths{0,1,2,3,4,5,7,8,15,16,31,32};
    for (std::size_t boundary : {64,128,192,256,320,384,512,1024,4096,65536})
        for (int delta = -4; delta <= 4; ++delta)
            lengths.push_back(static_cast<std::size_t>(static_cast<std::int64_t>(boundary) + delta));
    std::mt19937_64 gen(0x3FFC1E23ULL);
    for (unsigned pattern = 0; pattern < 6; ++pattern) {
        for (std::size_t i = 0; i < data.size(); ++i)
            data[i] = pattern == 0 ? 0 : pattern == 1 ? ~std::uint64_t{0} :
                pattern == 2 ? UINT64_C(0xaaaaaaaaaaaaaaaa) : pattern == 3 ?
                std::uint64_t{1} << (i % 64) : pattern == 4 ?
                ~std::uint64_t{0} >> (i % 64) : gen();
        for (std::size_t offset = 0; offset < 4; ++offset) {
            for (auto count : lengths) {
                std::uint64_t expected = 0;
                for (std::size_t i = 0; i < count; ++i) expected += std::popcount(data[offset + i]);
                check(reinsirk::detail::popcount_words_avx2(data.data() + offset, count) == expected);
                check(reinsirk::detail::popcount_words_carry_avx2(data.data() + offset, count) == expected);
            }
        }
    }
#endif
}
void ownership(const Oracle& oracle) {
    for (std::uint64_t n : {0,1,2,3,127,128,129,3969,16384,262143,262144,262145,524287,524288,524289,1048577}) {
        const auto expected = oracle.expected(n);
        PrimeSieve source(n), copy(source), assigned(1000000);
        assigned = source;
        check(copy.limit() == n && copy.primes() == expected);
        check(assigned.limit() == n && assigned.primes() == expected);
        auto* same = &assigned;
        assigned = *same;
        check(assigned.limit() == n && assigned.primes() == expected);
        assigned = std::move(*same);
        check(assigned.limit() == n && assigned.primes() == expected);
        PrimeSieve moved(std::move(copy));
        check(copy.limit() == 0 && copy.primes().empty() && copy.prime_count() == 0);
        check(!copy.is_prime(0));
        copy = PrimeSieve(31);
        check(copy.primes() == oracle.expected(31));
        check(moved.limit() == n && moved.primes() == expected);
        copy = std::move(moved);
        check(moved.limit() == 0 && moved.primes().empty() && moved.prime_count() == 0);
        check(copy.limit() == n && copy.primes() == expected);
        moved = PrimeSieve(2);
        check(moved.primes() == oracle.expected(2));
        source = PrimeSieve(0);
        check(source.primes().empty() && !source.is_prime(0));
        check(copy.primes() == expected && assigned.primes() == expected);
    }
}
}
int main() {
    const independent::Oracle oracle(4200000);
    independent::boundaries(oracle);
    independent::ownership(oracle);
    independent::carry_words();
    std::printf("INDEPENDENT PASS %zu checks; %zu is_prime; %zu boundary limits\n", independent::total, independent::primality_checks, independent::case_count);
}
