// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SIEVE_SOURCE
#define SIRKIT_SIEVE_SOURCE "../../../Math/Eratosthenes.cpp"
#endif
#include SIRKIT_SIEVE_SOURCE
#include SIRKIT_SIEVE_SOURCE
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

static std::uint64_t checks = 0, cases = 0;
static std::uint64_t active_limit = 0, active_first = 0, active_step = 0;
static constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();

static void check(bool value) {
    ++checks;
    if (!value) {
        std::fprintf(stderr, "stride failure limit=%ju first=%ju step=%ju cases=%ju checks=%ju\n",
                     static_cast<std::uintmax_t>(active_limit), static_cast<std::uintmax_t>(active_first),
                     static_cast<std::uintmax_t>(active_step), static_cast<std::uintmax_t>(cases),
                     static_cast<std::uintmax_t>(checks));
        std::abort();
    }
}

static std::vector<std::uint64_t> naive(std::size_t n) {
    std::vector<bool> prime(n + 1, true);
    prime[0] = false;
    if (n != 0) prime[1] = false;
    for (std::size_t p = 2; p <= n / p; ++p)
        if (prime[p])
            for (std::size_t multiple = p * p; multiple <= n; multiple += p)
                prime[multiple] = false;
    std::vector<std::uint64_t> result;
    for (std::size_t p = 2; p <= n; ++p) if (prime[p]) result.push_back(p);
    return result;
}

static void one(const PrimeSieve& sieve, const std::vector<std::uint64_t>& primes,
                std::uint64_t count, std::uint64_t first, std::uint64_t step) {
    active_limit = sieve.limit(); active_first = first; active_step = step;
    check(step != 0);
    std::uint64_t index = first, called = 0;
    sieve.for_each_prime_stride(first, step, [&](auto prime) {
        static_assert(std::is_same_v<decltype(prime), std::uint64_t>);
        check(index < count);
        check(prime == primes[static_cast<std::size_t>(index)]);
        ++called;
        index = step > maximum - index ? maximum : index + step;
    });
    check(called == (first < count ? (count - 1 - first) / step + 1 : 0));
    ++cases;
}

static void edge_cases(const PrimeSieve& sieve, const std::vector<std::uint64_t>& primes,
                       std::uint64_t count) {
    for (std::uint64_t step : {UINT64_C(1), UINT64_C(2), UINT64_C(7), UINT64_C(8),
                              UINT64_C(9), UINT64_C(63), UINT64_C(64), UINT64_C(65),
                              maximum - 1, maximum})
        for (std::uint64_t first : {UINT64_C(0), UINT64_C(1), UINT64_C(2), UINT64_C(7),
                                   UINT64_C(8), UINT64_C(63), UINT64_C(64), UINT64_C(65),
                                   count == 0 ? 0 : count - 1, count, count + 1, maximum})
            one(sieve, primes, count, first, step);
#ifndef SKIP_RUNTIME_CHECKS
    for (std::uint64_t first : {UINT64_C(0), count, maximum}) {
        active_first = first; active_step = 0;
        bool caught = false;
        try { sieve.for_each_prime_stride(first, 0, [](std::uint64_t) {}); }
        catch (const std::invalid_argument&) { caught = true; }
        check(caught);
    }
#endif
}

struct MoveOnly {
    std::unique_ptr<int> owned = std::make_unique<int>(0);
    std::uint64_t* count;
    explicit MoveOnly(std::uint64_t& value) : count(&value) {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&) = default;
    void operator()(std::uint64_t) & { ++*count; ++*owned; }
};

static void callbacks() {
    const PrimeSieve sieve(1000);
    const auto primes = naive(1000);
    const auto count = static_cast<std::uint64_t>(primes.size());
    std::uint64_t called = 0;
    MoveOnly callback(called);
    for (std::uint64_t step : {UINT64_C(1), UINT64_C(3), UINT64_C(7), UINT64_C(8),
                              UINT64_C(64), maximum}) {
        active_limit = sieve.limit(); active_first = 0; active_step = step;
        const auto expected = (count - 1) / step + 1;
        called = 0; *callback.owned = 0;
        sieve.for_each_prime_stride(0, step, callback);
        check(callback.owned && called == expected && *callback.owned == static_cast<int>(expected));
        called = 0;
        sieve.for_each_prime_stride(0, step, MoveOnly(called));
        check(called == expected);
        struct Counter {
            std::uint64_t count = 0;
            void operator()(std::uint64_t) { ++count; }
        } counter;
        sieve.for_each_prime_stride(0, step, counter);
        check(counter.count == expected);

        // どの列挙経路でも例外を伝え、元の素数表を変更しない。
        struct CallbackException {};
        bool caught = false;
        called = 0;
        active_first = 1;
        try {
            sieve.for_each_prime_stride(1, step, [&](std::uint64_t prime) {
                ++called; check(prime == 3); throw CallbackException{};
            });
        } catch (const CallbackException&) { caught = true; }
        check(caught && called == 1);
        check(sieve.prime_count() == count && sieve.primes() == primes);
        one(sieve, primes, count, 0, step);
    }
}

#ifndef SIRKIT_PRIME_STRIDE_TEST_QUICK
struct Digest {
    std::uint64_t hash = 1, count = 0;
    void operator()(std::uint64_t prime) {
        hash = hash * UINT64_C(0x9e3779b97f4a7c15) + prime; ++count;
    }
};
#endif

int main() {
    // CIでは全小上限・stepの範囲を縮め、大規模走査は手動実行に残す。
#ifdef SIRKIT_PRIME_STRIDE_TEST_QUICK
    constexpr std::uint64_t small_limit = 256, last_step = 32;
#else
    constexpr std::uint64_t small_limit = 2000, last_step = 200;
#endif
    const auto reference = naive(small_limit);
    for (std::uint64_t limit = 0; limit <= small_limit; ++limit) {
        const PrimeSieve sieve(limit);
        const auto count = static_cast<std::uint64_t>(
            std::upper_bound(reference.begin(), reference.end(), limit) - reference.begin());
        active_limit = limit;
        check(sieve.prime_count() == count);
        for (std::uint64_t step = 1; step <= last_step; ++step)
            for (std::uint64_t first = 0; first < step; ++first)
                one(sieve, reference, count, first, step);
        edge_cases(sieve, reference, count);
    }

    // 奇数bit列のword境界と、構築経路の境界の前後を独立表で照合する。
    for (std::uint64_t limit : {UINT64_C(127), UINT64_C(128), UINT64_C(129),
                               UINT64_C(255), UINT64_C(256), UINT64_C(257),
                               UINT64_C(511), UINT64_C(512), UINT64_C(513),
                               UINT64_C(1023), UINT64_C(1024), UINT64_C(1025),
                               UINT64_C(32767), UINT64_C(32768), UINT64_C(32769),
                               UINT64_C(65535), UINT64_C(65536), UINT64_C(65537),
                               UINT64_C(262143), UINT64_C(262144), UINT64_C(262145)}) {
        const PrimeSieve sieve(limit);
        const auto primes = naive(static_cast<std::size_t>(limit));
        active_limit = limit;
        check(sieve.prime_count() == primes.size());
        edge_cases(sieve, primes, static_cast<std::uint64_t>(primes.size()));
    }
    callbacks();

#ifdef SIRKIT_PRIME_STRIDE_TEST_QUICK
    const PrimeSieve million(1000000);
    const auto primes = naive(1000000);
    active_limit = million.limit();
    check(million.prime_count() == primes.size());
    edge_cases(million, primes, static_cast<std::uint64_t>(primes.size()));
#else
    std::mt19937_64 rng(771791);
    for (std::uint64_t limit : {UINT64_C(1000000), UINT64_C(100000000), UINT64_C(500000000)}) {
        const PrimeSieve sieve(limit);
        const auto count = sieve.prime_count();
        for (int trial = 0; trial < 12; ++trial) {
            const auto step = trial == 0 ? UINT64_C(1) : trial == 1 ? maximum : rng() % 100000 + 1;
            const auto first = trial == 0 ? UINT64_C(0) : trial == 1 ? maximum
                : trial % 2 ? count + rng() % 99 : rng() % (count + 1);
            Digest expected, actual;
            auto skip = first;
            sieve.for_each_prime([&](std::uint64_t prime) {
                if (skip == 0) { expected(prime); skip = step; } --skip;
            });
            sieve.for_each_prime_stride(first, step, actual);
            check(expected.hash == actual.hash && expected.count == actual.count); ++cases;
        }
        check(sieve.prime_count() == count);
    }
#endif
    std::printf("PASS PrimeSieve stride: cases=%ju checks=%ju\n",
                static_cast<std::uintmax_t>(cases), static_cast<std::uintmax_t>(checks));
}
