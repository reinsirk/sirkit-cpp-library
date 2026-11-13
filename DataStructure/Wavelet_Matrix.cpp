// SPDX-License-Identifier: CC0-1.0
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/wavelet_matrix_basic
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/wavelet_matrix.md
//
// API一覧
// [WaveletMatrix<T=int64_t>]
// bool以外の64bit以下の整数列。符号付きの最小値/最大値も扱う。区間は半開・順位kは0始まり。
// WaveletMatrix()/WaveletMatrix(values): 空/列を構築。vectorとspanを受け取る。
// build(values): 独立した列へ再構築。失敗時は元の列を維持。clear()/swap(): 空にする/交換する。
// size()/empty(): 要素数/空ならtrue、O(1)。コピーは独立。move元は空になる。
// access(i)/operator[](i): i番目の値。count(l,r,x)/rank(x,l,r): xの個数。
// count_less(l,r,x)/count_less_equal(l,r,x): x未満/x以下の個数。
// range_freq(l,r,lower,upper): lower<=value<upperの個数。lower>=upperなら0。
// kth_smallest(l,r,k)/kth_largest(l,r,k): 昇順/降順k番目。0<=k<r-lが前提。
// range_min(l,r)/range_max(l,r): 非空区間の最小/最大。
// predecessor(l,r,x)/successor(l,r,x): x未満の最大/x以上の最小。なければnullopt。
// Bは値の順序符号を最小値で平行移動した最大値のbit幅（0..64）。構築O(n(1+B))。
// 格納はO(1+B*ceil(n/64))語、構築中は追加O(n)。照会はO(1+B)。n<=UINT32_MAX。
// SKIP_RUNTIME_CHECKSで添字/区間/順位の検査を除く。件数の表現上限は常に確認する。
// rankは標準SWAR経路と、GCC/Clang x86のCPU対応を確認したPOPCNT経路を持つ。
// 32/64bit型の構築はn>=128で対応CPUのAVX2を使う。小さい列・他の整数型は標準経路。
// コンパイル対象が既にPOPCNT/AVX2を要求する場合、追加のCPU判定を省く。
// SIRKIT_DISABLE_SIMDで追加のCPU判定・POPCNT専用経路・AVX2実装を除く。

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#if !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#endif

#ifndef SIRKIT_WAVELET_POPCOUNT_DEFINED
#define SIRKIT_WAVELET_POPCOUNT_DEFINED
namespace reinsirk::detail {
inline unsigned wavelet_software_popcount(std::uint64_t value) noexcept {
    value -= (value >> 1) & UINT64_C(0x5555555555555555);
    value = (value & UINT64_C(0x3333333333333333)) + ((value >> 2) & UINT64_C(0x3333333333333333));
    value = (value + (value >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f);
    return static_cast<unsigned>((value * UINT64_C(0x0101010101010101)) >> 56);
}
#if !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__)) && !defined(__POPCNT__)
inline const bool wavelet_has_popcnt = [] { __builtin_cpu_init(); return static_cast<bool>(__builtin_cpu_supports("popcnt")); }();
#if defined(__x86_64__)
inline unsigned wavelet_hardware_popcount(std::uint64_t value) noexcept {
    std::uint64_t result;
    __asm__ volatile("popcnt %1, %0" : "=r"(result) : "r"(value) : "cc");
    return static_cast<unsigned>(result);
}
#else
__attribute__((target("popcnt"))) inline unsigned wavelet_hardware_popcount(std::uint64_t value) noexcept { return static_cast<unsigned>(std::popcount(value)); }
#endif
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
#if !defined(__AVX2__)
inline const bool wavelet_has_avx2 = [] { __builtin_cpu_init(); return static_cast<bool>(__builtin_cpu_supports("avx2")); }();
#endif
#endif
inline unsigned wavelet_popcount(std::uint64_t value) noexcept {
#if defined(__POPCNT__)
    return static_cast<unsigned>(std::popcount(value));
#elif !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
    return wavelet_has_popcnt ? wavelet_hardware_popcount(value) : wavelet_software_popcount(value);
#else
    return wavelet_software_popcount(value);
#endif
}
}
#endif

#ifndef SIRKIT_WAVELET_RANK_BITS_DEFINED
#define SIRKIT_WAVELET_RANK_BITS_DEFINED
namespace reinsirk::detail {
class WaveletRankBits {
    std::vector<std::uint64_t> words_;
    std::vector<std::uint32_t> prefix_;
public:
    explicit WaveletRankBits(std::size_t count) : words_(count / 64 + (count % 64 != 0)) {}
    void set(std::size_t index) noexcept { words_[index >> 6] |= std::uint64_t{1} << (index & 63); }
#if defined(__POPCNT__)
    void finish() {
        prefix_.resize(words_.size() + 1);
        for (std::size_t i = 0; i < words_.size(); ++i)
            prefix_[i + 1] = prefix_[i] + static_cast<std::uint32_t>(std::popcount(words_[i]));
    }
#else
    void finish() {
        prefix_.resize(words_.size() + 1);
        for (std::size_t i = 0; i < words_.size(); ++i)
            prefix_[i + 1] = prefix_[i] + static_cast<std::uint32_t>(wavelet_popcount(words_[i]));
    }
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
    template<class U> __attribute__((target("avx2")))
    std::size_t build_avx2(std::span<const U> values, unsigned shift) {
        prefix_.resize(words_.size() + 1);
        for (std::size_t word = 0; word < words_.size(); ++word) {
            const auto start = word * 64;
            const auto count = std::min(std::size_t{64}, values.size() - start);
            std::uint64_t packed = 0;
            std::size_t i = 0;
            if constexpr (sizeof(U) == 4) {
                const auto shifts = _mm256_set1_epi32(static_cast<int>(31 - shift));
                for (; i + 8 <= count; i += 8) {
                    const auto loaded = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values.data() + start + i));
                    const auto shifted = _mm256_sllv_epi32(loaded, shifts);
                    packed |= static_cast<std::uint64_t>(_mm256_movemask_ps(_mm256_castsi256_ps(shifted))) << i;
                }
            } else if constexpr (sizeof(U) == 8) {
                const auto shifts = _mm256_set1_epi64x(static_cast<std::int64_t>(63 - shift));
                for (; i + 4 <= count; i += 4) {
                    const auto loaded = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values.data() + start + i));
                    const auto shifted = _mm256_sllv_epi64(loaded, shifts);
                    packed |= static_cast<std::uint64_t>(_mm256_movemask_pd(_mm256_castsi256_pd(shifted))) << i;
                }
            }
            for (; i < count; ++i)
                packed |= static_cast<std::uint64_t>((values[start + i] >> shift) & U{1}) << i;
            words_[word] = packed;
            prefix_[word + 1] = prefix_[word] + wavelet_popcount(packed);
        }
        return values.size() - prefix_.back();
    }
#endif
    template<class U> std::size_t build_bits(std::span<const U> values, unsigned shift) {
#if !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        if constexpr (sizeof(U) == 4 || sizeof(U) == 8) {
#if defined(__AVX2__)
            if (values.size() >= 128) return build_avx2(values, shift);
#else
            if (values.size() >= 128 && wavelet_has_avx2) return build_avx2(values, shift);
#endif
        }
#endif
        prefix_.resize(words_.size() + 1);
        for (std::size_t word = 0; word < words_.size(); ++word) {
            const auto start = word * 64;
            const auto count = std::min(std::size_t{64}, values.size() - start);
            std::uint64_t packed = 0;
            for (std::size_t i = 0; i < count; ++i)
                packed |= static_cast<std::uint64_t>((values[start + i] >> shift) & U{1}) << i;
            words_[word] = packed;
            prefix_[word + 1] = prefix_[word] + wavelet_popcount(packed);
        }
        return values.size() - prefix_.back();
    }
#if !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
    template<class U> __attribute__((target("avx2")))
    void partition_avx2(std::span<const U> values, std::size_t zeros, std::span<U> output) const {
        constexpr unsigned lanes = 32 / sizeof(U);
        static constexpr auto permutation = [] {
            std::array<std::array<std::array<std::int32_t, 8>, 2>, 1u << lanes> table{};
            for (unsigned mask = 0; mask < (1u << lanes); ++mask) {
                for (unsigned group = 0; group < 2; ++group) {
                    unsigned position = 0;
                    for (unsigned i = 0; i < lanes; ++i) if (((mask >> i) & 1u) == group) {
                        for (unsigned part = 0; part < sizeof(U) / 4; ++part)
                            table[mask][group][position++] = static_cast<std::int32_t>(i * (sizeof(U) / 4) + part);
                    }
                }
            }
            return table;
        }();
        static constexpr auto masks = [] {
            std::array<std::array<std::int32_t, 8>, lanes + 1> table{};
            for (unsigned count = 0; count <= lanes; ++count)
                for (unsigned i = 0; i < count * (sizeof(U) / 4); ++i) table[count][i] = -1;
            return table;
        }();
        static constexpr auto counts = [] {
            std::array<unsigned char, 1u << lanes> table{};
            for (unsigned mask = 0; mask < (1u << lanes); ++mask) {
                unsigned count = 0;
                for (unsigned i = 0; i < lanes; ++i) count += (mask >> i) & 1u;
                table[mask] = static_cast<unsigned char>(count);
            }
            return table;
        }();
        std::size_t low = 0, high = zeros, i = 0;
        for (; i + lanes <= values.size(); i += lanes) {
            const auto packed = static_cast<unsigned>((words_[i >> 6] >> (i & 63)) & ((1u << lanes) - 1));
            const auto ones = static_cast<unsigned>(counts[packed]), zero_count = lanes - ones;
            const auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values.data() + i));
            const auto zero_order = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(permutation[packed][0].data()));
            const auto one_order = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(permutation[packed][1].data()));
            const auto zero_mask = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(masks[zero_count].data()));
            const auto one_mask = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(masks[ones].data()));
            if constexpr (sizeof(U) == 4) {
                _mm256_maskstore_epi32(reinterpret_cast<std::int32_t*>(output.data() + low), zero_mask,
                                      _mm256_permutevar8x32_epi32(value, zero_order));
                _mm256_maskstore_epi32(reinterpret_cast<std::int32_t*>(output.data() + high), one_mask,
                                      _mm256_permutevar8x32_epi32(value, one_order));
            } else {
                // 組込関数が要求する64bitのABI型を使う。
                using StoreWord = decltype(_mm256_extract_epi64(__m256i{}, 0));
                _mm256_maskstore_epi64(reinterpret_cast<StoreWord*>(output.data() + low), zero_mask,
                                      _mm256_permutevar8x32_epi32(value, zero_order));
                _mm256_maskstore_epi64(reinterpret_cast<StoreWord*>(output.data() + high), one_mask,
                                      _mm256_permutevar8x32_epi32(value, one_order));
            }
            low += zero_count; high += ones;
        }
        for (; i < values.size(); ++i) {
            const auto bit = static_cast<std::size_t>((words_[i >> 6] >> (i & 63)) & 1);
            output[bit ? high++ : low++] = values[i];
        }
    }
#endif
    template<class U> void partition(std::span<const U> values, unsigned shift,
                                     std::size_t zeros, std::span<U> output) const {
#if !defined(SIRKIT_DISABLE_SIMD) && !defined(SIRKIT_WAVELET_FORCE_SOFTWARE) && (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
        if constexpr (sizeof(U) == 4 || sizeof(U) == 8) {
#if defined(__AVX2__)
            if (values.size() >= 128) { partition_avx2(values, zeros, output); return; }
#else
            if (values.size() >= 128 && wavelet_has_avx2) { partition_avx2(values, zeros, output); return; }
#endif
        }
#endif
        std::size_t low = 0, high = zeros;
        for (U value : values) {
            const auto bit = static_cast<std::size_t>((value >> shift) & U{1});
            output[bit ? high : low] = value;
            low += bit ^ std::size_t{1}; high += bit;
        }
    }
    bool at(std::size_t index) const noexcept { return (words_[index >> 6] >> (index & 63)) & 1; }
#if defined(__POPCNT__)
    std::size_t rank1(std::size_t end) const noexcept {
        const auto offset = end & 63;
        // endが64の倍数ならwords_[end/64]を読まない。end==nも含む。
        return prefix_[end >> 6] + (offset == 0 ? 0 :
            static_cast<std::size_t>(std::popcount(words_[end >> 6] & ((std::uint64_t{1} << offset) - 1))));
    }
#else
    std::size_t rank1(std::size_t end) const noexcept {
        const auto offset = end & 63;
        // endが64の倍数ならwords_[end/64]を読まない。end==nも含む。
        return prefix_[end >> 6] + (offset == 0 ? 0 :
            static_cast<std::size_t>(wavelet_popcount(words_[end >> 6] & ((std::uint64_t{1} << offset) - 1))));
    }
#endif
};
}
#endif

#ifndef SIRKIT_WAVELET_MATRIX_DEFINED
#define SIRKIT_WAVELET_MATRIX_DEFINED
template <class T = std::int64_t>
class WaveletMatrix {
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= sizeof(std::uint64_t));
    static_assert(std::is_same_v<T, std::remove_cv_t<T>>);
    using U = std::make_unsigned_t<T>;
    static constexpr unsigned digits = std::numeric_limits<U>::digits;
    static constexpr U sign = std::is_signed_v<T> ? static_cast<U>(U{1} << (digits - 1)) : U{0};
    static U ordered(T value) noexcept { return static_cast<U>(value) ^ sign; }
    static T restore(U code) noexcept { return std::bit_cast<T>(static_cast<U>(code ^ sign)); }

    std::size_t n_ = 0;
    unsigned bits_ = 0;
    U base_ = 0, maximum_ = 0;
    std::vector<reinsirk::detail::WaveletRankBits> levels_;
    std::vector<std::size_t> zero_;

    void check_range(std::size_t left, std::size_t right) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > n_) throw std::out_of_range("WaveletMatrix range");
#else
        static_cast<void>(left); static_cast<void>(right);
#endif
    }
    template<class Work>
    void build_levels(std::span<const T> values) {
        std::vector<Work> current(n_), next(n_);
        for (std::size_t i = 0; i < n_; ++i)
            current[i] = static_cast<Work>(ordered(values[i]) - base_);
        levels_.reserve(bits_); zero_.reserve(bits_);
        for (unsigned depth = 0; depth < bits_; ++depth) {
            const unsigned shift = bits_ - 1 - depth;
            levels_.emplace_back(n_);
            auto& bits = levels_.back();
            const auto zeros = bits.build_bits(std::span<const Work>(current), shift);
            zero_.push_back(zeros);
            bits.partition(std::span<const Work>(current), shift, zeros, std::span<Work>(next));
            current.swap(next);
        }
    }
    void initialize(std::span<const T> values) {
        if (values.size() > std::numeric_limits<std::uint32_t>::max())
            throw std::length_error("WaveletMatrix count limit");
        n_ = values.size();
        if (n_ == 0) return;
        base_ = maximum_ = ordered(values[0]);
        for (T value : values) {
            const auto key = ordered(value);
            base_ = std::min(base_, key); maximum_ = std::max(maximum_, key);
        }
        bits_ = static_cast<unsigned>(std::bit_width(static_cast<U>(maximum_ - base_)));
        if (bits_ == 0) return;
        if constexpr (sizeof(U) > sizeof(std::uint32_t)) {
            if (bits_ <= 32) { build_levels<std::uint32_t>(values); return; }
        }
        build_levels<U>(values);
    }
    template<bool StopWhenEmpty>
    std::size_t rank_code(U code, std::size_t left, std::size_t right) const noexcept {
        for (unsigned depth = 0; depth < bits_; ++depth) {
            const auto a = levels_[depth].rank1(left), b = levels_[depth].rank1(right);
            if ((code >> (bits_ - 1 - depth)) & U{1}) { left = zero_[depth] + a; right = zero_[depth] + b; }
            else { left -= a; right -= b; }
            if constexpr (StopWhenEmpty) { if (left == right) return 0; }
        }
        return right - left;
    }
    std::size_t count_less_unchecked(std::size_t left, std::size_t right, U value) const noexcept {
        if (left == right || value <= base_) return 0;
        if (value > maximum_) return right - left;
        const U code = static_cast<U>(value - base_);
        std::size_t result = 0;
        for (unsigned depth = 0; depth < bits_; ++depth) {
            const auto ones_left = levels_[depth].rank1(left), ones_right = levels_[depth].rank1(right);
            const auto zeros_left = left - ones_left, zeros_right = right - ones_right;
            if ((code >> (bits_ - 1 - depth)) & U{1}) {
                result += zeros_right - zeros_left;
                left = zero_[depth] + ones_left; right = zero_[depth] + ones_right;
            } else { left = zeros_left; right = zeros_right; }
        }
        return result;
    }
public:
    WaveletMatrix() = default;
    explicit WaveletMatrix(std::span<const T> values) { initialize(values); }
    explicit WaveletMatrix(const std::vector<T>& values) : WaveletMatrix(std::span<const T>(values)) {}
    WaveletMatrix(const WaveletMatrix&) = default;
    WaveletMatrix& operator=(const WaveletMatrix& other) {
        if (this != &other) { WaveletMatrix next(other); swap(next); }
        return *this;
    }
    WaveletMatrix(WaveletMatrix&& other) noexcept
        : n_(std::exchange(other.n_, 0)), bits_(std::exchange(other.bits_, 0)),
          base_(std::exchange(other.base_, U{0})), maximum_(std::exchange(other.maximum_, U{0})),
          levels_(std::move(other.levels_)), zero_(std::move(other.zero_)) {}
    WaveletMatrix& operator=(WaveletMatrix&& other) noexcept {
        if (this != &other) { WaveletMatrix temporary(std::move(other)); swap(temporary); }
        return *this;
    }
    void swap(WaveletMatrix& other) noexcept {
        std::swap(n_, other.n_); std::swap(bits_, other.bits_);
        std::swap(base_, other.base_); std::swap(maximum_, other.maximum_);
        levels_.swap(other.levels_); zero_.swap(other.zero_);
    }
    friend void swap(WaveletMatrix& a, WaveletMatrix& b) noexcept { a.swap(b); }
    void build(std::span<const T> values) { WaveletMatrix next(values); swap(next); }
    void build(const std::vector<T>& values) { build(std::span<const T>(values)); }
    void clear() noexcept { WaveletMatrix next; swap(next); }
    std::size_t size() const noexcept { return n_; }
    bool empty() const noexcept { return n_ == 0; }
    T access(std::size_t index) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (index >= n_) throw std::out_of_range("WaveletMatrix index");
#endif
        U code = 0;
        for (unsigned depth = 0; depth < bits_; ++depth) {
            const bool bit = levels_[depth].at(index);
            const auto ones = levels_[depth].rank1(index);
            if (bit) { code |= static_cast<U>(U{1} << (bits_ - 1 - depth)); index = zero_[depth] + ones; }
            else index -= ones;
        }
        return restore(static_cast<U>(base_ + code));
    }
    T operator[](std::size_t index) const { return access(index); }
    std::size_t rank(T value, std::size_t left, std::size_t right) const {
        check_range(left, right);
        const U key = ordered(value);
        if (left == right || key < base_ || key > maximum_) return 0;
        const U code = static_cast<U>(key - base_);
        if (bits_ <= 1) {
            if (bits_ == 0) return right - left;
            const auto ones = levels_[0].rank1(right) - levels_[0].rank1(left);
            return code == 0 ? (right - left) - ones : ones;
        }
        // 幅や列が小さい場合は、各段の早期終了判定を省く。
        return bits_ <= 8 || n_ <= 128 ? rank_code<false>(code, left, right) : rank_code<true>(code, left, right);
    }
    std::size_t count(std::size_t left, std::size_t right, T value) const { return rank(value, left, right); }
    std::size_t count_less(std::size_t left, std::size_t right, T value) const {
        check_range(left, right); return count_less_unchecked(left, right, ordered(value));
    }
    std::size_t count_less_equal(std::size_t left, std::size_t right, T value) const {
        check_range(left, right);
        const U key = ordered(value);
        return key == std::numeric_limits<U>::max() ? right - left :
            count_less_unchecked(left, right, static_cast<U>(key + U{1}));
    }
    std::size_t range_freq(std::size_t left, std::size_t right, T lower, T upper) const {
        check_range(left, right);
        return lower < upper ? count_less_unchecked(left, right, ordered(upper)) - count_less_unchecked(left, right, ordered(lower)) : 0;
    }
    T kth_smallest(std::size_t left, std::size_t right, std::size_t k) const {
        check_range(left, right);
#ifndef SKIP_RUNTIME_CHECKS
        if (k >= right - left) throw std::out_of_range("WaveletMatrix rank");
#endif
        U code = 0;
        for (unsigned depth = 0; depth < bits_; ++depth) {
            const auto a = levels_[depth].rank1(left), b = levels_[depth].rank1(right);
            const auto low = left - a, high = right - b;
            if (k < high - low) { left = low; right = high; }
            else {
                k -= high - low; code |= static_cast<U>(U{1} << (bits_ - 1 - depth));
                left = zero_[depth] + a; right = zero_[depth] + b;
            }
        }
        return restore(static_cast<U>(base_ + code));
    }
    T kth_largest(std::size_t left, std::size_t right, std::size_t k) const {
        check_range(left, right);
#ifndef SKIP_RUNTIME_CHECKS
        if (k >= right - left) throw std::out_of_range("WaveletMatrix rank");
#endif
        return kth_smallest(left, right, right - left - 1 - k);
    }
    T range_min(std::size_t left, std::size_t right) const { return kth_smallest(left, right, 0); }
    T range_max(std::size_t left, std::size_t right) const { return kth_largest(left, right, 0); }
    std::optional<T> predecessor(std::size_t left, std::size_t right, T value) const {
        const auto count = count_less(left, right, value);
        if (count == 0) return std::nullopt;
        return kth_smallest(left, right, count - 1);
    }
    std::optional<T> successor(std::size_t left, std::size_t right, T value) const {
        const auto count = count_less(left, right, value);
        if (count == right - left) return std::nullopt;
        return kth_smallest(left, right, count);
    }
};
template <class T> WaveletMatrix(const std::vector<T>&) -> WaveletMatrix<T>;
template <class T> WaveletMatrix(std::span<const T>) -> WaveletMatrix<T>;
#endif
