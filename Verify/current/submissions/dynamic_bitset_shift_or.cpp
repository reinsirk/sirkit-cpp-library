#define SKIP_RUNTIME_CHECKS
// Verify: https://atcoder.jp/contests/tdpc/tasks/tdpc_contest

// Library: DataStructure/Dynamic_Bitset.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考: https://nyaannyaan.github.io/library/data-structure/dynamic-bitset.hpp.html
// API選定の参考。配置・所有・可変長操作は独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_bitset.md
// API一覧
// DynamicBitset(n=0,value=false): n bitを同値で初期化。DynamicBitset(string_view): 高位からの01文字列。
// size()/empty(), words(): bit数/空判定/下位word順のspan<const uint64_t>。末尾の未使用bitは常に0。
// test(pos), operator[](pos) const: bit取得。非const []は代入・bool変換・flip・&=|=^=ができる非所有proxy。
// set()/reset()/flip(): 全bit操作。set(pos,value=true)/reset(pos)/flip(pos): 1 bit操作。全て*thisを返す。
// all()/any()/none(), count(), count(l,r): 全bit判定/1の数/[l,r)の1の数。空のallはtrue。
// intersection_count(other)/intersection_parity(other): ANDの1の数/その偶奇(bool)。&=|=^=と&|^、~、==も使える。==は長さも比較する。
// <<=k / >>=k / <<k / >>k: 固定長の論理shift。空いたbitは0。k>=sizeなら全0。
// shift_or(k): 自身に自身の左shiftをOR。元の各bitを一度だけ参照し、一時bitsetを確保しない。
// find_first()/find_last(), find_next(pos)/find_prev(pos): 1 bit位置。nextはposより後、prevはpos未満。
//   見つからなければsize()。next(pos>=size)もsize、prev(pos>size)は上限をsizeへ丸める。
// resize(n,value=false): 旧bitを保ち新bitを指定値へ。reserve_bits(n): 容量予約。clear(): 0 bitへ。
// to_string(): 高位からの01文字列。copyは独立所有、copy代入/確保失敗時は旧内容を維持。
// move元は空で再利用可。swapはO(1)。word span/proxyはサイズ変更・代入・所有権移動まで使用する。
// 1 bit操作・size/emptyはO(1)。全bit/2項/shift/探索は最悪O(ceil(n/64))、文字列変換はO(n)。
// resize/reserveは再確保時O(ceil(max(旧n,n)/64))、コピーはO(ceil(n/64))。count(l,r)はO((r-l)/64+1)。
// 番号・区間・2項演算の同長・01文字の検査はSKIP_RUNTIME_CHECKSで除外。容量保護と0判定は維持。
// GCC/Clang x86では対応CPUでPOPCNT/AVX2を選択。SIRKIT_DISABLE_SIMDで追加実装・CPU判定を除外。
// count/AND countは4 wordからPOPCNT、64 wordからAVX2。parityは64 wordからAVX2。
// ClangでAVX2を既定対象にしたビルドは、測定で速かった標準ループの自動vector化を使う。

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#ifndef SIRKIT_DYNAMIC_BITSET_WORD_OPERATIONS_DEFINED
#define SIRKIT_DYNAMIC_BITSET_WORD_OPERATIONS_DEFINED
namespace reinsirk::detail {
inline unsigned bitset_word_popcount(std::uint64_t x) noexcept {
    x -= (x >> 1) & UINT64_C(0x5555555555555555);
    x = (x & UINT64_C(0x3333333333333333)) + ((x >> 2) & UINT64_C(0x3333333333333333));
    x = (x + (x >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f);
    return static_cast<unsigned>((x * UINT64_C(0x0101010101010101)) >> 56);
}
inline std::size_t bitset_count_words_scalar(const std::uint64_t* words, std::size_t count) noexcept {
    std::size_t result = 0;
    for (std::size_t i = 0; i < count; ++i) result += bitset_word_popcount(words[i]);
    return result;
}
inline std::size_t bitset_intersection_words_scalar(const std::uint64_t* a, const std::uint64_t* b, std::size_t count) noexcept {
    std::size_t result = 0;
    for (std::size_t i = 0; i < count; ++i) result += bitset_word_popcount(a[i] & b[i]);
    return result;
}
inline bool bitset_intersection_parity_words_scalar(const std::uint64_t* a, const std::uint64_t* b, std::size_t count) noexcept {
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < count; ++i) value ^= a[i] & b[i];
    return (bitset_word_popcount(value) & 1) != 0;
}

#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))

__attribute__((target("popcnt"))) inline std::size_t bitset_count_words_popcnt(const std::uint64_t* a,std::size_t n) noexcept {
    std::size_t s0=0,s1=0,s2=0,s3=0,i=0;
    for(;n-i>=4;i+=4){s0+=static_cast<unsigned>(__builtin_popcountll(a[i]));s1+=static_cast<unsigned>(__builtin_popcountll(a[i+1]));s2+=static_cast<unsigned>(__builtin_popcountll(a[i+2]));s3+=static_cast<unsigned>(__builtin_popcountll(a[i+3]));}
    for(;i<n;++i)s0+=static_cast<unsigned>(__builtin_popcountll(a[i]));
    return s0+s1+s2+s3;
}
__attribute__((target("popcnt"))) inline std::size_t bitset_intersection_words_popcnt(const std::uint64_t* a,const std::uint64_t* b,std::size_t n) noexcept {
    std::size_t s0=0,s1=0,s2=0,s3=0,i=0;
    for(;n-i>=4;i+=4){s0+=static_cast<unsigned>(__builtin_popcountll(a[i]&b[i]));s1+=static_cast<unsigned>(__builtin_popcountll(a[i+1]&b[i+1]));s2+=static_cast<unsigned>(__builtin_popcountll(a[i+2]&b[i+2]));s3+=static_cast<unsigned>(__builtin_popcountll(a[i+3]&b[i+3]));}
    for(;i<n;++i)s0+=static_cast<unsigned>(__builtin_popcountll(a[i]&b[i]));
    return s0+s1+s2+s3;
}
__attribute__((target("avx2,popcnt"))) inline std::size_t bitset_count_words_avx2(const std::uint64_t* a,std::size_t n) noexcept {
    const auto table=_mm256_setr_epi8(0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
    const auto mask=_mm256_set1_epi8(15),zero=_mm256_setzero_si256();auto total=zero;std::size_t i=0;
    for(;n-i>=4;i+=4){const auto x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a+i));const auto lo=_mm256_shuffle_epi8(table,_mm256_and_si256(x,mask));const auto hi=_mm256_shuffle_epi8(table,_mm256_and_si256(_mm256_srli_epi16(x,4),mask));total=_mm256_add_epi64(total,_mm256_sad_epu8(_mm256_add_epi8(lo,hi),zero));}
    alignas(32) std::uint64_t lanes[4];_mm256_store_si256(reinterpret_cast<__m256i*>(lanes),total);auto result=lanes[0]+lanes[1]+lanes[2]+lanes[3];
    for(;i<n;++i)result+=static_cast<unsigned>(__builtin_popcountll(a[i]));
    return static_cast<std::size_t>(result);
}
__attribute__((target("avx2,popcnt"))) inline std::size_t bitset_intersection_words_avx2(const std::uint64_t* a,const std::uint64_t* b,std::size_t n) noexcept {
    const auto table=_mm256_setr_epi8(0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
    const auto mask=_mm256_set1_epi8(15),zero=_mm256_setzero_si256();auto total=zero;std::size_t i=0;
    for(;n-i>=4;i+=4){const auto x=_mm256_and_si256(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a+i)),_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b+i)));const auto lo=_mm256_shuffle_epi8(table,_mm256_and_si256(x,mask));const auto hi=_mm256_shuffle_epi8(table,_mm256_and_si256(_mm256_srli_epi16(x,4),mask));total=_mm256_add_epi64(total,_mm256_sad_epu8(_mm256_add_epi8(lo,hi),zero));}
    alignas(32) std::uint64_t lanes[4];_mm256_store_si256(reinterpret_cast<__m256i*>(lanes),total);auto result=lanes[0]+lanes[1]+lanes[2]+lanes[3];
    for(;i<n;++i)result+=static_cast<unsigned>(__builtin_popcountll(a[i]&b[i]));
    return static_cast<std::size_t>(result);
}

__attribute__((target("avx2,popcnt"), noinline)) inline std::size_t bitset_parity_words_avx2(const std::uint64_t* a,const std::uint64_t* b,std::size_t n)noexcept {
    auto total=_mm256_setzero_si256();std::size_t i=0;
    for(;n-i>=4;i+=4)total=_mm256_xor_si256(total,_mm256_and_si256(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a+i)),_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b+i))));
    alignas(32)std::uint64_t lanes[4];_mm256_store_si256(reinterpret_cast<__m256i*>(lanes),total);auto value=lanes[0]^lanes[1]^lanes[2]^lanes[3];
    for(;i<n;++i)value^=a[i]&b[i];
    return static_cast<unsigned>(__builtin_popcountll(value))&1;
}

inline unsigned bitset_cpu_features() noexcept {
    static const unsigned features = (__builtin_cpu_supports("popcnt") ? 1U : 0U) | (__builtin_cpu_supports("avx2") ? 2U : 0U);
    return features;
}
#endif
inline std::size_t bitset_count_words(const std::uint64_t* words, std::size_t count) noexcept {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#if !(defined(__clang__) && defined(__AVX2__))
    if (count >= 4) {
        const auto features = bitset_cpu_features();
        if (count >= 64 && features == 3) return bitset_count_words_avx2(words, count);
        if (features & 1) return bitset_count_words_popcnt(words, count);
    }
#endif
#endif
    return bitset_count_words_scalar(words, count);
}
inline std::size_t bitset_intersection_words(const std::uint64_t* a, const std::uint64_t* b, std::size_t count) noexcept {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#if !(defined(__clang__) && defined(__AVX2__))
    if (count >= 4) {
        const auto features = bitset_cpu_features();
        if (count >= 64 && features == 3) return bitset_intersection_words_avx2(a, b, count);
        if (features & 1) return bitset_intersection_words_popcnt(a, b, count);
    }
#endif
#endif
    return bitset_intersection_words_scalar(a, b, count);
}
inline bool bitset_intersection_parity_words(const std::uint64_t* a, const std::uint64_t* b, std::size_t count) noexcept {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#if !(defined(__clang__) && defined(__AVX2__))
    if (count >= 64 && bitset_cpu_features() == 3) return bitset_parity_words_avx2(a, b, count) != 0;
#endif
#endif
    return bitset_intersection_parity_words_scalar(a, b, count);
}
}
#endif

#ifndef SIRKIT_DYNAMIC_BITSET_DEFINED
#define SIRKIT_DYNAMIC_BITSET_DEFINED
class DynamicBitset {
    std::size_t size_ = 0;
    std::vector<std::uint64_t> words_;
    static constexpr std::uint64_t low_mask(unsigned bits) noexcept {
        return bits == 64 ? ~std::uint64_t{0} : (std::uint64_t{1} << bits) - 1;
    }
    static std::size_t word_count(std::size_t bits) {
        const auto count = bits / 64 + (bits % 64 != 0);
        if (count > std::vector<std::uint64_t>().max_size()) throw std::length_error("DynamicBitset capacity");
        return count;
    }
    void mask_tail() noexcept { if (size_ % 64) words_.back() &= low_mask(static_cast<unsigned>(size_ % 64)); }
    void check_position(std::size_t pos) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (pos >= size_) throw std::out_of_range("DynamicBitset position");
#else
        (void)pos;
#endif
    }
    void check_length(const DynamicBitset& other) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (size_ != other.size_) throw std::invalid_argument("DynamicBitset length mismatch");
#else
        (void)other;
#endif
    }
public:
    class reference {
        std::uint64_t* word_;
        std::uint64_t mask_;
        friend class DynamicBitset;
        reference(std::uint64_t& word, std::uint64_t mask) noexcept : word_(&word), mask_(mask) {}
    public:
        reference(const reference&) noexcept = default;
        operator bool() const noexcept { return (*word_ & mask_) != 0; }
        bool operator~() const noexcept { return !static_cast<bool>(*this); }
        reference& operator=(bool value) noexcept { if (value) *word_ |= mask_; else *word_ &= ~mask_; return *this; }
        reference& operator=(const reference& other) noexcept { return *this = static_cast<bool>(other); }
        reference& flip() noexcept { *word_ ^= mask_; return *this; }
        reference& operator&=(bool value) noexcept { if (!value) *word_ &= ~mask_; return *this; }
        reference& operator|=(bool value) noexcept { if (value) *word_ |= mask_; return *this; }
        reference& operator^=(bool value) noexcept { if (value) *word_ ^= mask_; return *this; }
    };
    explicit DynamicBitset(std::size_t n = 0, bool value = false) : size_(n), words_(word_count(n), value ? ~std::uint64_t{0} : 0) { mask_tail(); }
    explicit DynamicBitset(std::string_view text) : DynamicBitset(text.size()) {
        for (std::size_t i = 0; i < text.size(); ++i) {
            const char c = text[i];
#ifndef SKIP_RUNTIME_CHECKS
            if (c != '0' && c != '1') throw std::invalid_argument("DynamicBitset binary string");
#endif
            if (c == '1') { const auto pos = text.size() - 1 - i; words_[pos / 64] |= std::uint64_t{1} << (pos % 64); }
        }
    }
    DynamicBitset(const DynamicBitset&) = default;
    DynamicBitset& operator=(const DynamicBitset& other) { if (this != &other) { DynamicBitset copy(other); swap(copy); } return *this; }
    DynamicBitset(DynamicBitset&& other) noexcept { swap(other); }
    DynamicBitset& operator=(DynamicBitset&& other) noexcept { if (this != &other) { clear(); swap(other); } return *this; }
    void swap(DynamicBitset& other) noexcept { std::swap(size_, other.size_); words_.swap(other.words_); }
    friend void swap(DynamicBitset& a, DynamicBitset& b) noexcept { a.swap(b); }
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    std::span<const std::uint64_t> words() const noexcept { return words_; }
    bool test(std::size_t pos) const { check_position(pos); return ((words_[pos / 64] >> (pos % 64)) & 1) != 0; }
    bool operator[](std::size_t pos) const { return test(pos); }
    reference operator[](std::size_t pos) { check_position(pos); return reference(words_[pos / 64], std::uint64_t{1} << (pos % 64)); }
    DynamicBitset& set() noexcept { std::fill(words_.begin(), words_.end(), ~std::uint64_t{0}); mask_tail(); return *this; }
    DynamicBitset& reset() noexcept { std::fill(words_.begin(), words_.end(), 0); return *this; }
    DynamicBitset& flip() noexcept { for (auto& word : words_) word = ~word; mask_tail(); return *this; }
    DynamicBitset& set(std::size_t pos, bool value = true) { check_position(pos); const auto mask = std::uint64_t{1} << (pos % 64); if (value) words_[pos / 64] |= mask; else words_[pos / 64] &= ~mask; return *this; }
    DynamicBitset& reset(std::size_t pos) { return set(pos, false); }
    DynamicBitset& flip(std::size_t pos) { check_position(pos); words_[pos / 64] ^= std::uint64_t{1} << (pos % 64); return *this; }
    bool any() const noexcept { for (const auto word : words_) if (word) return true; return false; }
    bool none() const noexcept { return !any(); }
    bool all() const noexcept {
        if (words_.empty()) return true;
        for (std::size_t i = 0; i + 1 < words_.size(); ++i) if (words_[i] != ~std::uint64_t{0}) return false;
        return words_.back() == low_mask(static_cast<unsigned>(size_ % 64 == 0 ? 64 : size_ % 64));
    }
    std::size_t count() const noexcept { return reinsirk::detail::bitset_count_words(words_.data(), words_.size()); }
    std::size_t count(std::size_t l, std::size_t r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l > r || r > size_) throw std::out_of_range("DynamicBitset range");
#endif
        if (l == r) return 0;
        const auto first = l / 64, last = (r - 1) / 64;
        const auto left = ~std::uint64_t{0} << (l % 64);
        const auto right = low_mask(static_cast<unsigned>(r % 64 == 0 ? 64 : r % 64));
        if (first == last) return reinsirk::detail::bitset_word_popcount(words_[first] & left & right);
        return reinsirk::detail::bitset_word_popcount(words_[first] & left)
            + reinsirk::detail::bitset_count_words(words_.data() + first + 1, last - first - 1)
            + reinsirk::detail::bitset_word_popcount(words_[last] & right);
    }
    std::size_t intersection_count(const DynamicBitset& other) const { check_length(other); return reinsirk::detail::bitset_intersection_words(words_.data(), other.words_.data(), words_.size()); }
    bool intersection_parity(const DynamicBitset& other) const { check_length(other); return reinsirk::detail::bitset_intersection_parity_words(words_.data(), other.words_.data(), words_.size()); }
    DynamicBitset& operator&=(const DynamicBitset& other) { check_length(other); for (std::size_t i = 0; i < words_.size(); ++i) words_[i] &= other.words_[i]; return *this; }
    DynamicBitset& operator|=(const DynamicBitset& other) { check_length(other); for (std::size_t i = 0; i < words_.size(); ++i) words_[i] |= other.words_[i]; return *this; }
    DynamicBitset& operator^=(const DynamicBitset& other) { check_length(other); for (std::size_t i = 0; i < words_.size(); ++i) words_[i] ^= other.words_[i]; return *this; }
    friend DynamicBitset operator&(DynamicBitset a, const DynamicBitset& b) { a &= b; return a; }
    friend DynamicBitset operator|(DynamicBitset a, const DynamicBitset& b) { a |= b; return a; }
    friend DynamicBitset operator^(DynamicBitset a, const DynamicBitset& b) { a ^= b; return a; }
    friend DynamicBitset operator~(DynamicBitset value) { value.flip(); return value; }
    bool operator==(const DynamicBitset& other) const noexcept { return size_ == other.size_ && words_ == other.words_; }
    DynamicBitset& operator<<=(std::size_t shift) noexcept {
        if (shift >= size_) return reset();
        if (!shift) return *this;
        const auto whole = shift / 64; const auto bits = static_cast<unsigned>(shift % 64);
        for (auto i = words_.size(); i != 0;) { --i; std::uint64_t value = 0;
            if (i >= whole) { value = words_[i - whole] << bits; if (bits && i > whole) value |= words_[i - whole - 1] >> (64 - bits); }
            words_[i] = value;
        }
        mask_tail(); return *this;
    }
    DynamicBitset& operator>>=(std::size_t shift) noexcept {
        if (shift >= size_) return reset();
        if (!shift) return *this;
        const auto whole = shift / 64; const auto bits = static_cast<unsigned>(shift % 64);
        for (std::size_t i = 0; i < words_.size(); ++i) { std::uint64_t value = 0; const auto source = i + whole;
            if (source < words_.size()) { value = words_[source] >> bits; if (bits && source + 1 < words_.size()) value |= words_[source + 1] << (64 - bits); }
            words_[i] = value;
        }
        return *this;
    }
    friend DynamicBitset operator<<(DynamicBitset value, std::size_t shift) { value <<= shift; return value; }
    friend DynamicBitset operator>>(DynamicBitset value, std::size_t shift) { value >>= shift; return value; }
    DynamicBitset& shift_or(std::size_t shift) noexcept {
        if (!shift || shift >= size_) return *this;
        const auto whole = shift / 64; const auto bits = static_cast<unsigned>(shift % 64);
        for (auto i = words_.size(); i != whole;) { --i; auto value = words_[i - whole] << bits;
            if (bits && i > whole) value |= words_[i - whole - 1] >> (64 - bits);
            words_[i] |= value;
        }
        mask_tail(); return *this;
    }
    std::size_t find_first() const noexcept {
        for (std::size_t i = 0; i < words_.size(); ++i) if (words_[i]) return i * 64 + static_cast<unsigned>(std::countr_zero(words_[i]));
        return size_;
    }
    std::size_t find_next(std::size_t pos) const noexcept {
        if (pos >= size_ || ++pos == size_) return size_;
        auto i = pos / 64; const auto word = words_[i] & (~std::uint64_t{0} << (pos % 64));
        if (word) return i * 64 + static_cast<unsigned>(std::countr_zero(word));
        while (i < words_.size() - 1) { ++i; if (words_[i]) return i * 64 + static_cast<unsigned>(std::countr_zero(words_[i])); }
        return size_;
    }
    std::size_t find_prev(std::size_t pos) const noexcept {
        pos = std::min(pos, size_); if (!pos) return size_; --pos;
        auto i = pos / 64; const auto word = words_[i] & low_mask(static_cast<unsigned>(pos % 64 + 1));
        if (word) return i * 64 + 63 - static_cast<unsigned>(std::countl_zero(word));
        while (i) { --i; if (words_[i]) return i * 64 + 63 - static_cast<unsigned>(std::countl_zero(words_[i])); }
        return size_;
    }
    std::size_t find_last() const noexcept { return find_prev(size_); }
    void reserve_bits(std::size_t n) { words_.reserve(word_count(n)); }
    void resize(std::size_t n, bool value = false) {
        const auto old = size_; words_.resize(word_count(n), value ? ~std::uint64_t{0} : 0);
        if (n > old && value && old % 64) words_[old / 64] |= ~std::uint64_t{0} << (old % 64);
        size_ = n; mask_tail();
    }
    void clear() noexcept { words_.clear(); size_ = 0; }
    std::string to_string() const {
        std::string result(size_, '0');
        for (std::size_t i = 0; i < size_; ++i) if ((words_[i / 64] >> (i % 64)) & 1) result[size_ - 1 - i] = '1';
        return result;
    }
};
#endif


// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif

#include <cstddef>
int main() {
    int n{}; verify_in >> n;
    DynamicBitset reachable(10001);
    reachable.set(0);
    for (int i = 0; i < n; ++i) {
        std::size_t score{}; verify_in >> score;
        reachable.shift_or(score);
    }
    verify_out << reachable.count() << '\n';
}
