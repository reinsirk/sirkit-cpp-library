// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prime_sieve.md
//
// API一覧
// [PrimeSieve]
// PrimeSieve(limit=0): [0,limit]の素数表を構築する。O(limit log log limit)、奇数1個につき1bit。
// コピーは独立した表を持ち、移動元はlimit=0の空の素数表として再利用できる。
// is_prime(n): 0<=n<=limitの素数判定をO(1)で返す。
// limit(): 構築時の上限を返す。O(1)。
// prime_count(): 上限以下の素数の個数を返す。O(limit/64)。
// primes(): 上限以下の素数を昇順のvectorで返す。O(limit/64+素数数)。
// append_to(ans): 上限以下の素数を昇順でansの末尾に追加する。O(limit/64+素数数)、償却。
// for_each_prime(f): 昇順にf(uint64_tの素数)を呼ぶ。O(limit/64+素数数)、呼出し中に表を変更しない。
//
// [eratosthenes]
// eratosthenes(limit, ans): limit以下の素数を昇順でansの末尾へ追加する。O(limit log log limit)。
// eratosthenes(limit): limit以下の素数を昇順のvectorで返す。同じ計算量。

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>


#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
#include <immintrin.h>
#endif

#ifndef SIRKIT_PRIME_SIEVE_POPCOUNT_DEFINED
#define SIRKIT_PRIME_SIEVE_POPCOUNT_DEFINED
namespace reinsirk::detail {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
// 各bit位置で3個の値を足し、桁上がりと下位bitへ分ける。
[[gnu::target("avx2"), gnu::always_inline]] inline void carry_save_avx2(
    __m256i& high, __m256i& low, __m256i x, __m256i y, __m256i z) noexcept {
    const auto partial = _mm256_xor_si256(x, y);
    high = _mm256_or_si256(_mm256_and_si256(x, y), _mm256_and_si256(partial, z));
    low = _mm256_xor_si256(partial, z);
}

[[gnu::target("avx2"), gnu::always_inline]] inline __m256i popcount_lanes_avx2(
    __m256i value) noexcept {
    const auto table = _mm256_setr_epi8(
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
    const auto mask = _mm256_set1_epi8(15);
    const auto low = _mm256_shuffle_epi8(table, _mm256_and_si256(value, mask));
    const auto high = _mm256_shuffle_epi8(table,
        _mm256_and_si256(_mm256_srli_epi16(value, 4), mask));
    return _mm256_sad_epu8(_mm256_add_epi8(low, high), _mm256_setzero_si256());
}

[[gnu::target("avx2"), gnu::always_inline]] inline __m256i load_words_avx2(
    const std::uint64_t* words) noexcept {
    return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(words));
}

// 16ベクトル分をbitごとの加算器でまとめ、各桁の重みを掛けて数える。
[[gnu::target("avx2"), gnu::noinline]] inline std::uint64_t popcount_words_carry_avx2(
    const std::uint64_t* words, std::size_t count) noexcept {
    const auto zero = _mm256_setzero_si256();
    auto ones = zero, twos = zero, fours = zero, eights = zero, total = zero;
    std::size_t i = 0;
    for (; count - i >= 64; i += 64) {
        __m256i two_a, two_b, four_a, four_b, eight_a, eight_b, sixteens;
        const auto* block = words + i;
        carry_save_avx2(two_a, ones, ones, load_words_avx2(block), load_words_avx2(block + 4));
        carry_save_avx2(two_b, ones, ones, load_words_avx2(block + 8), load_words_avx2(block + 12));
        carry_save_avx2(four_a, twos, twos, two_a, two_b);
        carry_save_avx2(two_a, ones, ones, load_words_avx2(block + 16), load_words_avx2(block + 20));
        carry_save_avx2(two_b, ones, ones, load_words_avx2(block + 24), load_words_avx2(block + 28));
        carry_save_avx2(four_b, twos, twos, two_a, two_b);
        carry_save_avx2(eight_a, fours, fours, four_a, four_b);
        carry_save_avx2(two_a, ones, ones, load_words_avx2(block + 32), load_words_avx2(block + 36));
        carry_save_avx2(two_b, ones, ones, load_words_avx2(block + 40), load_words_avx2(block + 44));
        carry_save_avx2(four_a, twos, twos, two_a, two_b);
        carry_save_avx2(two_a, ones, ones, load_words_avx2(block + 48), load_words_avx2(block + 52));
        carry_save_avx2(two_b, ones, ones, load_words_avx2(block + 56), load_words_avx2(block + 60));
        carry_save_avx2(four_b, twos, twos, two_a, two_b);
        carry_save_avx2(eight_b, fours, fours, four_a, four_b);
        carry_save_avx2(sixteens, eights, eights, eight_a, eight_b);
        total = _mm256_add_epi64(total, popcount_lanes_avx2(sixteens));
    }
    total = _mm256_slli_epi64(total, 4);
    total = _mm256_add_epi64(total, _mm256_slli_epi64(popcount_lanes_avx2(eights), 3));
    total = _mm256_add_epi64(total, _mm256_slli_epi64(popcount_lanes_avx2(fours), 2));
    total = _mm256_add_epi64(total, _mm256_slli_epi64(popcount_lanes_avx2(twos), 1));
    total = _mm256_add_epi64(total, popcount_lanes_avx2(ones));
    alignas(32) std::uint64_t lanes[4];
    _mm256_store_si256(reinterpret_cast<__m256i*>(lanes), total);
    std::uint64_t result = lanes[0] + lanes[1] + lanes[2] + lanes[3];
    for (; count - i >= 4; i += 4) {
        _mm256_store_si256(reinterpret_cast<__m256i*>(lanes),
            popcount_lanes_avx2(load_words_avx2(words + i)));
        result += lanes[0] + lanes[1] + lanes[2] + lanes[3];
    }
    for (; i < count; ++i) result += std::popcount(words[i]);
    return result;
}

// 内部用。32語単位で集計し、残りを4語・1語ずつ処理する。
[[gnu::target("avx2"), gnu::noinline]] inline std::uint64_t popcount_words_avx2(
    const std::uint64_t* words, std::size_t count) noexcept {
    if (count >= 256) return popcount_words_carry_avx2(words, count);
    const auto table = _mm256_setr_epi8(
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
    const auto mask = _mm256_set1_epi8(15);
    const auto zero = _mm256_setzero_si256();
    auto sum = zero;
    std::size_t i = 0;
    for (; count - i >= 32; i += 32) {
        auto bytes = zero;
        for (std::size_t j = 0; j < 32; j += 4) {
            const auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(words + i + j));
            const auto low = _mm256_and_si256(value, mask);
            const auto high = _mm256_and_si256(_mm256_srli_epi16(value, 4), mask);
            const auto bits = _mm256_add_epi8(_mm256_shuffle_epi8(table, low),
                                             _mm256_shuffle_epi8(table, high));
            bytes = _mm256_add_epi8(bytes, bits);
        }
        sum = _mm256_add_epi64(sum, _mm256_sad_epu8(bytes, zero));
    }
    for (; count - i >= 4; i += 4) {
        const auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(words + i));
        const auto low = _mm256_and_si256(value, mask);
        const auto high = _mm256_and_si256(_mm256_srli_epi16(value, 4), mask);
        const auto bits = _mm256_add_epi8(_mm256_shuffle_epi8(table, low),
                                         _mm256_shuffle_epi8(table, high));
        sum = _mm256_add_epi64(sum, _mm256_sad_epu8(bits, zero));
    }
    alignas(32) std::uint64_t lanes[4];
    _mm256_store_si256(reinterpret_cast<__m256i*>(lanes), sum);
    std::uint64_t result = lanes[0] + lanes[1] + lanes[2] + lanes[3];
    for (; i < count; ++i) result += std::popcount(words[i]);
    return result;
}
#endif
} // namespace reinsirk::detail
#endif


#ifndef SIRKIT_PRIME_SIEVE_PATTERN_DEFINED
#define SIRKIT_PRIME_SIEVE_PATTERN_DEFINED
namespace reinsirk::detail {
inline constexpr auto sieve_pattern = [] {
    std::array<std::uint64_t, 1155> values{};
    for (std::size_t word = 0; word < values.size(); ++word)
        for (unsigned bit = 0; bit < 64; ++bit) {
            const auto n = (word * 64 + bit) * 2 + 1;
            if (n % 3 != 0 && n % 5 != 0 && n % 7 != 0 && n % 11 != 0)
                values[word] |= std::uint64_t{1} << bit;
        }
    return values;
}();
} // namespace reinsirk::detail
#endif
#ifndef SIRKIT_PRIME_SIEVE_DEFINED
#define SIRKIT_PRIME_SIEVE_DEFINED

class PrimeSieve {
public:
    explicit PrimeSieve(std::uint64_t limit = 0) : limit_(limit) {
        if (limit < 128) {
            if (limit != 0) {
                // 127以下の奇合成数は3・5・7・11のいずれかの倍数。
                constexpr auto primes = (reinsirk::detail::sieve_pattern[0] |
                    (std::uint64_t{1} << 1) | (std::uint64_t{1} << 2) |
                    (std::uint64_t{1} << 3) | (std::uint64_t{1} << 5)) & ~std::uint64_t{1};
                const auto bits = static_cast<unsigned>((limit + 1) / 2);
                flags_.push_back(primes & (~std::uint64_t{0} >> (64 - bits)));
            }
            return;
        }
        const std::uint64_t odds = limit / 2 + limit % 2;
        const std::uint64_t words = odds / 64 + (odds % 64 != 0);
        if (words > flags_.max_size())
            throw std::length_error("PrimeSieve: limit exceeds capacity");
        // 周期パターンから直接構築し、同じ領域の二重初期化を避ける。
        flags_.reserve(static_cast<std::size_t>(words));
        const auto& pattern = reinsirk::detail::sieve_pattern;
        constexpr auto period = reinsirk::detail::sieve_pattern.size();
        auto remaining = static_cast<std::size_t>(words);
        while (remaining >= period) {
            flags_.insert(flags_.end(), pattern.begin(), pattern.end());
            remaining -= period;
        }
        flags_.insert(flags_.end(), pattern.begin(), pattern.begin() + remaining);
        build_large(odds);
    }

    PrimeSieve(const PrimeSieve&) = default;
    PrimeSieve& operator=(const PrimeSieve& other) {
        flags_ = other.flags_;
        limit_ = other.limit_;
        return *this;
    }
    PrimeSieve(PrimeSieve&& other) noexcept : limit_(std::exchange(other.limit_, 0)) {
        flags_.swap(other.flags_);
    }
    PrimeSieve& operator=(PrimeSieve&& other) noexcept {
        if (this != &other) {
            PrimeSieve next(std::move(other));
            std::swap(limit_, next.limit_);
            flags_.swap(next.flags_);
        }
        return *this;
    }

    bool is_prime(std::uint64_t n) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (n > limit_) [[unlikely]]
            throw std::out_of_range("PrimeSieve::is_prime: beyond limit");
#endif
        if ((n & 1) == 0) return n == 2;
        const std::uint64_t index = n / 2;
        return (flags_[static_cast<std::size_t>(index / 64)] >> (index % 64) & 1) != 0;
    }
    std::uint64_t limit() const noexcept { return limit_; }
    std::uint64_t prime_count() const noexcept {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
#if !(defined(__clang__) && defined(__AVX2__))
        if (limit_ >= 3969 && __builtin_cpu_supports("avx2")) [[unlikely]]
            return static_cast<std::uint64_t>(limit_ >= 2) +
                   reinsirk::detail::popcount_words_avx2(flags_.data(), flags_.size());
#endif
#endif
        std::uint64_t count = limit_ >= 2;
        for (std::uint64_t word : flags_) count += std::popcount(word);
        return count;
    }
#if defined(__clang__) && !defined(__AVX2__)
    std::vector<std::uint64_t> primes() const {
        std::vector<std::uint64_t> result;
        if (limit_ < 2) return result;
        if (limit_ >= 16) {
            const auto count = prime_count();
            if (count > result.max_size())
                throw std::length_error("PrimeSieve::primes: capacity exceeded");
            result.reserve(static_cast<std::size_t>(count));
        }
        append_to(result);
        return result;
    }
#else
    std::vector<std::uint64_t> primes() const {
        std::vector<std::uint64_t> result;
        if (limit_ < 2) return result;
        if (limit_ >= 16384) {
            const auto count = prime_count();
            if (count > result.max_size())
                throw std::length_error("PrimeSieve::primes: capacity exceeded");
            result.reserve(static_cast<std::size_t>(count));
        } else if (limit_ >= 16) {
            return small_primes();
        }
        append_to(result);
        return result;
    }
#endif
#if defined(__clang__)
    [[gnu::always_inline]]
#endif
    void append_to(std::vector<std::uint64_t>& ans) const {
        if (limit_ >= 2) ans.push_back(2);
        std::uint64_t base = 1;
        for (std::uint64_t word : flags_) {
            while (word != 0) {
                const unsigned bit = std::countr_zero(word);
                ans.push_back(base + bit * 2);
                word &= word - 1;
            }
            base += 128;
        }
    }

    template <class Function>
    void for_each_prime(Function&& function) const {
        if (limit_ >= 2) function(std::uint64_t{2});
        for (std::size_t i = 0; i < flags_.size(); ++i) {
            std::uint64_t word = flags_[i];
            while (word != 0) {
                const unsigned bit = std::countr_zero(word);
                function((static_cast<std::uint64_t>(i) * 64 + bit) * 2 + 1);
                word &= word - 1;
            }
        }
    }

private:
#if !defined(__clang__) || defined(__AVX2__)
#if defined(__GNUC__) || defined(__clang__)
    [[gnu::noinline]]
#endif
    std::vector<std::uint64_t> small_primes() const {
        std::vector<std::uint64_t> result;
        const auto count = prime_count();
        if (count > result.max_size())
            throw std::length_error("PrimeSieve::primes: capacity exceeded");
        result.reserve(static_cast<std::size_t>(count));
        append_to(result);
        return result;
    }
#endif

    void build_large(std::uint64_t odds) {
        flags_[0] |= (std::uint64_t{1} << 1) | (std::uint64_t{1} << 2) |
                     (std::uint64_t{1} << 3) | (std::uint64_t{1} << 5);
        flags_[0] &= ~std::uint64_t{1};
        if (odds % 64 != 0)
            flags_.back() &= (std::uint64_t{1} << (odds % 64)) - 1;
        if (limit_ >= 262144) {
            build_blocked(odds);
            return;
        }
        for (std::uint64_t p = 13; p <= limit_ / p; p += 2) {
            const auto index = p / 2;
            if (((flags_[index / 64] >> (index % 64)) & 1) == 0) continue;
            for (auto k = p * p / 2; k < odds; k += p)
                flags_[k / 64] &= ~(std::uint64_t{1} << (k % 64));
        }
    }

    // 13〜127の素数の倍数をp語周期のANDで消す。素数自身は呼出側で復元する。
    template <unsigned Prime>
#if defined(__GNUC__) || defined(__clang__)
    [[gnu::noinline]]
#endif
    static void mask_multiples(std::uint64_t* flags, std::uint64_t first_word,
                               std::uint64_t word_count) noexcept {
        static constexpr auto pattern = [] {
            std::array<std::uint64_t, Prime> result{};
            for (unsigned w = 0; w < Prime; ++w)
                for (unsigned bit = 0; bit < 64; ++bit)
                    if ((w * 128 + bit * 2 + 1) % Prime != 0)
                        result[w] |= std::uint64_t{1} << bit;
            return result;
        }();
        auto* dest = flags + first_word;
        auto phase = first_word % Prime;
        while (phase != 0 && word_count != 0) {
            *dest++ &= pattern[phase];
            --word_count;
            if (++phase == Prime) phase = 0;
        }
        while (word_count >= Prime) {
            for (unsigned j = 0; j < Prime; ++j) dest[j] &= pattern[j];
            dest += Prime;
            word_count -= Prime;
        }
        for (unsigned j = 0; j < word_count; ++j) dest[j] &= pattern[j];
    }

    // 二つの小素数の倍数を、積周期のANDマスクで同時に消す。
    template <unsigned P, unsigned Q>
#if defined(__GNUC__) || defined(__clang__)
    [[gnu::noinline]]
#endif
    static void mask_pair(std::uint64_t* flags, std::uint64_t first_word,
                          std::uint64_t word_count) noexcept {
        constexpr unsigned Period = P * Q;
        static constexpr auto pattern = [] {
            std::array<std::uint64_t, Period> result{};
            result.fill(~std::uint64_t{0});
            for (std::uint64_t k = P / 2; k < std::uint64_t{Period} * 64; k += P)
                result[k / 64] &= ~(std::uint64_t{1} << (k % 64));
            for (std::uint64_t k = Q / 2; k < std::uint64_t{Period} * 64; k += Q)
                result[k / 64] &= ~(std::uint64_t{1} << (k % 64));
            return result;
        }();
        auto* dest = flags + first_word;
        auto phase = first_word % Period;
        while (phase != 0 && word_count != 0) {
            *dest++ &= pattern[phase];
            --word_count;
            if (++phase == Period) phase = 0;
        }
        while (word_count >= Period) {
            for (unsigned j = 0; j < Period; ++j) dest[j] &= pattern[j];
            dest += Period;
            word_count -= Period;
        }
        for (unsigned j = 0; j < word_count; ++j) dest[j] &= pattern[j];
    }

    void build_blocked(std::uint64_t odds) {
        std::uint64_t root = std::uint64_t{1} << ((std::bit_width(limit_) + 1) / 2);
        for (;;) {
            const auto next = (root + limit_ / root) / 2;
            if (next >= root) break;
            root = next;
        }
        const std::uint64_t base_odds = root / 2 + root % 2;
        auto* const flags = flags_.data();
        for (std::uint64_t p = 13; p <= root / p; p += 2) {
            const auto index = p / 2;
            if (((flags[index / 64] >> (index % 64)) & 1) == 0) continue;
            for (auto k = p * p / 2; k < base_odds; k += p)
                flags[k / 64] &= ~(std::uint64_t{1} << (k % 64));
        }
        struct Strike { std::uint64_t prime, next; };
        std::vector<Strike> strikes;
        for (std::uint64_t p = 131; p <= root; p += 2) {
            const auto index = p / 2;
            if ((flags[index / 64] >> (index % 64)) & 1)
                strikes.push_back({p, p * p / 2});
        }
        const std::uint64_t block_bits = std::max(std::uint64_t{262144},
            (root + 63) & ~std::uint64_t{63});
        std::size_t active = 0;
        for (std::uint64_t begin = 0; begin < odds; begin += block_bits) {
            const auto end = std::min(odds, begin + block_bits);
            const auto first_word = begin / 64;
            const auto word_count = (end + 63) / 64 - first_word;
            mask_pair<13, 17>(flags, first_word, word_count);
            mask_pair<19, 23>(flags, first_word, word_count);
            mask_pair<29, 31>(flags, first_word, word_count);
            mask_pair<37, 41>(flags, first_word, word_count);
            mask_multiples<43>(flags, first_word, word_count);
            mask_multiples<47>(flags, first_word, word_count);
            mask_multiples<53>(flags, first_word, word_count);
            mask_multiples<59>(flags, first_word, word_count);
            mask_multiples<61>(flags, first_word, word_count);
            mask_multiples<67>(flags, first_word, word_count);
            mask_multiples<71>(flags, first_word, word_count);
            mask_multiples<73>(flags, first_word, word_count);
            mask_multiples<79>(flags, first_word, word_count);
            mask_multiples<83>(flags, first_word, word_count);
            mask_multiples<89>(flags, first_word, word_count);
            mask_multiples<97>(flags, first_word, word_count);
            mask_multiples<101>(flags, first_word, word_count);
            mask_multiples<103>(flags, first_word, word_count);
            mask_multiples<107>(flags, first_word, word_count);
            mask_multiples<109>(flags, first_word, word_count);
            mask_multiples<113>(flags, first_word, word_count);
            mask_multiples<127>(flags, first_word, word_count);
            while (active < strikes.size() && strikes[active].next < end) ++active;
            for (std::size_t i = 0; i < active; ++i) {
                auto k = strikes[i].next;
                const auto step = strikes[i].prime;
                // 可変シフトが重い構成では、消去bitの回転を引き継ぐ。
#if defined(__GNUC__) && !defined(__clang__) && defined(__x86_64__) && !defined(__BMI2__)
                auto mask = ~(std::uint64_t{1} << (k % 64));
                const auto shift = static_cast<int>(step % 64);
                for (; k + step * 3 < end; k += step * 4) {
                    flags[k / 64] &= mask; mask = std::rotl(mask, shift);
                    flags[(k + step) / 64] &= mask; mask = std::rotl(mask, shift);
                    flags[(k + step * 2) / 64] &= mask; mask = std::rotl(mask, shift);
                    flags[(k + step * 3) / 64] &= mask; mask = std::rotl(mask, shift);
                }
                for (; k < end; k += step) {
                    flags[k / 64] &= mask; mask = std::rotl(mask, shift);
                }
#else
                for (; k + step * 3 < end; k += step * 4) {
                    flags[k / 64] &= ~(std::uint64_t{1} << (k % 64));
                    flags[(k + step * 1) / 64] &= ~(std::uint64_t{1} << ((k + step * 1) % 64));
                    flags[(k + step * 2) / 64] &= ~(std::uint64_t{1} << ((k + step * 2) % 64));
                    flags[(k + step * 3) / 64] &= ~(std::uint64_t{1} << ((k + step * 3) % 64));
                }
                for (; k < end; k += step)
                    flags[k / 64] &= ~(std::uint64_t{1} << (k % 64));
#endif
                strikes[i].next = k;
            }
        }
        flags[0] |= (std::uint64_t{1} << 6) |
                    (std::uint64_t{1} << 8) |
                    (std::uint64_t{1} << 9) |
                    (std::uint64_t{1} << 11) |
                    (std::uint64_t{1} << 14) |
                    (std::uint64_t{1} << 15) |
                    (std::uint64_t{1} << 18) |
                    (std::uint64_t{1} << 20) |
                    (std::uint64_t{1} << 21) |
                    (std::uint64_t{1} << 23) |
                    (std::uint64_t{1} << 26) |
                    (std::uint64_t{1} << 29) |
                    (std::uint64_t{1} << 30) |
                    (std::uint64_t{1} << 33) |
                    (std::uint64_t{1} << 35) |
                    (std::uint64_t{1} << 36) |
                    (std::uint64_t{1} << 39) |
                    (std::uint64_t{1} << 41) |
                    (std::uint64_t{1} << 44) |
                    (std::uint64_t{1} << 48) |
                    (std::uint64_t{1} << 50) |
                    (std::uint64_t{1} << 51) |
                    (std::uint64_t{1} << 53) |
                    (std::uint64_t{1} << 54) |
                    (std::uint64_t{1} << 56) |
                    (std::uint64_t{1} << 63);
    }

    std::uint64_t limit_;
    std::vector<std::uint64_t> flags_;
};
#endif

#ifndef SIRKIT_ERATOSTHENES_DEFINED
#define SIRKIT_ERATOSTHENES_DEFINED
inline void eratosthenes(std::uint64_t limit, std::vector<std::uint64_t>& ans) {
    PrimeSieve(limit).append_to(ans);
}
inline std::vector<std::uint64_t> eratosthenes(std::uint64_t limit) {
    return PrimeSieve(limit).primes();
}
#endif
