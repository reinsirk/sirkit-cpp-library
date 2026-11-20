// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Math/Eratosthenes.cpp
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
// for_each_prime_stride(first,step,f): 0始まりの素数番号first,first+step,...を同様に列挙。step>0。
// 最悪O(limit/64+素数数)、追加領域O(1)。対象外のwordはまとめて飛ばす。
//
// [eratosthenes]
// eratosthenes(limit, ans): limit以下の素数を昇順でansの末尾へ追加する。O(limit log log limit)。
// eratosthenes(limit): limit以下の素数を昇順のvectorで返す。同じ計算量。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>




// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PRIME_SIEVE_POPCOUNT_DEFINED
#define SIRKIT_PRIME_SIEVE_POPCOUNT_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
} // namespace reinsirk::detail
#endif


#ifndef SIRKIT_PRIME_SIEVE_PATTERN_DEFINED
#define SIRKIT_PRIME_SIEVE_PATTERN_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline constexpr auto sieve_pattern = []() consteval {
    constexpr std::size_t kPeriod = 3 * 5 * 7 * 11;
    // 2周期を用意し、周期の境目をまたぐ64ビットも同じ方法で取り出す。
    std::array<std::uint64_t, (2 * kPeriod + 63) / 64> period{};
    for (std::size_t bit = 0; bit < kPeriod; ++bit) {
        const auto n = 2 * bit + 1;
        if (n % 3 != 0 && n % 5 != 0 && n % 7 != 0 && n % 11 != 0) {
            period[bit / 64] |= std::uint64_t{1} << (bit % 64);
            const auto next = bit + kPeriod;
            period[next / 64] |= std::uint64_t{1} << (next % 64);
        }
    }
    std::array<std::uint64_t, kPeriod> values{};
    for (std::size_t word = 0; word < values.size(); ++word) {
        const auto bit = (64 * word) % kPeriod;
        const auto offset = bit % 64;
        const auto low = period[bit / 64] >> offset;
        values[word] = offset == 0 ? low : low | (period[bit / 64 + 1] << (64 - offset));
    }
    return values;
}();
inline std::size_t sieve_checked_capacity(std::uint64_t count, std::size_t maximum,
                                          const char* error) {
    if (count > maximum) throw std::length_error(error);
    return static_cast<std::size_t>(count);
}
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
                constexpr auto primes = (/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::sieve_pattern[0] |
                    (std::uint64_t{1} << 1) | (std::uint64_t{1} << 2) |
                    (std::uint64_t{1} << 3) | (std::uint64_t{1} << 5)) & ~std::uint64_t{1};
                const auto bits = static_cast<unsigned>((limit + 1) / 2);
                flags_.push_back(primes & (~std::uint64_t{0} >> (64 - bits)));
            }
            return;
        }
        const std::uint64_t odds = limit / 2 + limit % 2;
        const auto words = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::sieve_checked_capacity(
            odds / 64 + (odds % 64 != 0), flags_.max_size(), "PrimeSieve: limit exceeds capacity");
        // 周期パターンから直接構築し、同じ領域の二重初期化を避ける。
        flags_.reserve(words);
        const auto& pattern = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::sieve_pattern;
        constexpr auto period = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::sieve_pattern.size();
        auto remaining = words;
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
        std::uint64_t count = limit_ >= 2;
        for (std::uint64_t word : flags_) count += std::popcount(word);
        return count;
    }
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

    template <class Function>
    void for_each_prime_stride(std::uint64_t first, std::uint64_t step, Function&& function) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (step == 0) throw std::invalid_argument("PrimeSieve::for_each_prime_stride: zero step");
#endif
        if (step < 8) {
            if (step == 1 && first == 0) { for_each_prime(function); return; }
            std::uint64_t skip = first;
            for_each_prime([&](std::uint64_t prime) {
                if (skip == 0) { function(prime); skip = step; }
                --skip;
            });
            return;
        }
        for_each_prime_stride_words(first, step, function);
    }


private:
    template <class Function>
    void for_each_prime_stride_words(std::uint64_t first, std::uint64_t step, Function& function) const {
        std::uint64_t skip = first;
        if (limit_ >= 2) {
            if (skip == 0) { function(std::uint64_t{2}); skip = step; }
            --skip;
        }
        for (std::size_t i = 0; i < flags_.size(); ++i) {
            std::uint64_t word = flags_[i];
            if (skip >= 8) {
                const auto count = static_cast<std::uint64_t>(std::popcount(word));
                if (skip >= count) { skip -= count; continue; }
            }
            while (word != 0) {
                if (skip == 0) {
                    function((static_cast<std::uint64_t>(i) * 64 + std::countr_zero(word)) * 2 + 1);
                    skip = step;
                }
                --skip;
                word &= word - 1;
            }
        }
    }

    std::vector<std::uint64_t> small_primes() const {
        std::vector<std::uint64_t> result;
        const auto count = prime_count();
        if (count > result.max_size())
            throw std::length_error("PrimeSieve::primes: capacity exceeded");
        result.reserve(static_cast<std::size_t>(count));
        append_to(result);
        return result;
    }

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
                for (; k + step * 3 < end; k += step * 4) {
                    flags[k / 64] &= ~(std::uint64_t{1} << (k % 64));
                    flags[(k + step * 1) / 64] &= ~(std::uint64_t{1} << ((k + step * 1) % 64));
                    flags[(k + step * 2) / 64] &= ~(std::uint64_t{1} << ((k + step * 2) % 64));
                    flags[(k + step * 3) / 64] &= ~(std::uint64_t{1} << ((k + step * 3) % 64));
                }
                for (; k < end; k += step)
                    flags[k / 64] &= ~(std::uint64_t{1} << (k % 64));
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
