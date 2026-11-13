#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/inverse_matrix_mod_2

// Library: Math/Binary_Matrix.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Binary_Matrix.cpp
// 参考: https://nyaannyaan.github.io/library/matrix/f2-matrix.hpp
// GF(2)の掃き出し法・機能選定を参考に独立実装。内部bit kernelはDynamicBitsetと同一guardで共有。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binary_matrix.md
// API一覧
// BinaryMatrix(rows=0,cols=0): 全0行列。O(rows*ceil(cols/64))時間/領域。GF(2)上の要素0/1を扱う。
// rows()/cols()/empty(): 行数/列数/要素が0個か。row_words(row): 列0を最下位bitとするconst word span。
// test(row,col)/operator()(row,col): bool取得。set(row,col,value=true)/flip(row,col): 要素変更、*thisを返す。
// swap_rows(a,b), xor_rows(target,source,first_col=0): 行交換/first_col以降だけの行XOR。自己XOR可。
// transpose(): 転置。multiply(other)/operator*: 積。operator*=は自己参照可。pow(uint64_t): 正方行列の累乗。
// identity(n): 単位行列。+= / + / ^= / ^は同形行列のGF(2)和。==は形と全要素の一致。
// rank(): 階数。determinant(): 正方行列の行列式bool。0x0の行列式は1。
// inverse(): optional<BinaryMatrix>、非正則ならnullopt。0x0は0x0単位行列を返す。
// solve(span<const uint8_t> rhs): Ax=rhsのoptional<LinearSolution>、不能ならnullopt。rhsは行数個の0/1。
// LinearSolution {particular,kernel,pivot_columns}: particularは列数個のuint8_t特解、
//   kernelは各行が核の基底となる(列数-rank)x列数の行列。任意解は特解と基底行の線形結合。
//   pivot_columnsはRREFのpivot列。0行/0列も可。0行では全変数が自由、0列ではrhsが全0の場合のみ可。
// reset(rows=0,cols=0)/clear(): 再初期化/0x0。copyは独立所有・代入強保証、move元は0x0で再利用可。
// swapはO(1)。set/test/flip/形照会はO(1)、行操作はO(ceil(cols/64))、転置O(rows*cols)。
// AxBの積はO(1+B.rows*B.cols + A.rows*ceil(B.cols/64) + A.rows*B.cols*ceil(A.cols/64))。
// 累乗は行列積O(log(k+1))回。内側0次元でも結果の全0領域は確保する。
// rank/inverseの消去は最悪O(1+rows*cols + rows*min(rows,cols)*ceil(cols/64))時間。元行列を変更しない。
// solveは上記にO(rows+cols+cols*(cols-rank))時間を追加、核は(cols-rank)*ceil(cols/64)語。
// LinearSolutionも独立copy/代入強保証、move元の全フィールドは空。swapはO(1)。
// 確保失敗時は元の内容/viewを維持。viewは成功したreset/clear/代入/所有権移動まで有効。
// 番号・形・rhs長/参照したrhsの0/1検査はSKIP_RUNTIME_CHECKSで除外。容量保護・数学的0判定は維持。
// 0列のsolveは非0のrhsを見つけた時点で終了し、それ以降のrhsは検査しない。
// SIRKIT_DISABLE_SIMDは共有bit kernelの追加CPU判定/専用実装を除く。消去/転置に独自SIMDは使わない。

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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
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

#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))

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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
#if !(defined(__AVX2__))
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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
#if !(defined(__AVX2__))
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
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__x86_64__)) || (defined(__i386__)))
#if !(defined(__AVX2__))
    if (count >= 64 && bitset_cpu_features() == 3) return bitset_parity_words_avx2(a, b, count) != 0;
#endif
#endif
    return bitset_intersection_parity_words_scalar(a, b, count);
}
}
#endif

#include <optional>

#ifndef SIRKIT_BINARY_MATRIX_DEFINED
#define SIRKIT_BINARY_MATRIX_DEFINED
class BinaryMatrix {
public:
    struct LinearSolution;
private:
    std::size_t rows_ = 0, cols_ = 0, stride_ = 0;
    std::vector<std::uint64_t> data_;
    static std::size_t stride_for(std::size_t cols) noexcept { return cols / 64 + (cols % 64 != 0); }
    static std::size_t storage_size(std::size_t rows, std::size_t stride) {
        const auto limit = std::vector<std::uint64_t>().max_size();
        if (stride && rows > limit / stride) throw std::length_error("BinaryMatrix capacity");
        return rows * stride;
    }
    void check_row(std::size_t row) const {
        (void)row;
    }
    void check_position(std::size_t row, std::size_t col) const {
        (void)row; (void)col;
    }
    void check_same_shape(const BinaryMatrix& other) const {
        (void)other;
    }
    void check_square() const {
    }
    void xor_suffix(std::size_t target, std::size_t source, std::size_t first_word) noexcept {
        const auto t = target * stride_, s = source * stride_;
        for (std::size_t word = first_word; word < stride_; ++word) data_[t + word] ^= data_[s + word];
    }
    static std::size_t eliminate(BinaryMatrix& work, BinaryMatrix* companion,
                                             std::vector<std::uint8_t>* rhs, bool reduced, std::vector<std::size_t>* pivots = nullptr) {
        if (pivots) pivots->reserve(std::min(work.rows_, work.cols_));
        if (!work.rows_ || !work.cols_) return 0;
        std::size_t rank = 0;
        for (std::size_t col = 0; col < work.cols_ && rank < work.rows_; ++col) {
            const auto word = col / 64, mask = std::uint64_t{1} << (col % 64);
            auto pivot = rank;
            while (pivot < work.rows_ && !(work.data_[pivot * work.stride_ + word] & mask)) ++pivot;
            if (pivot == work.rows_) continue;
            work.swap_rows(rank, pivot);
            if (companion) companion->swap_rows(rank, pivot);
            if (rhs) std::swap((*rhs)[rank], (*rhs)[pivot]);
            const auto first = reduced ? std::size_t{0} : rank + 1;
            for (auto row = first; row < work.rows_; ++row) if (row != rank && (work.data_[row * work.stride_ + word] & mask)) {
                // pivot行のそれ以前の列は0なのでword途中からでも既存pivotを壊さない。
                work.xor_suffix(row, rank, word);
                if (companion) companion->xor_suffix(row, rank, 0);
                if (rhs) (*rhs)[row] ^= (*rhs)[rank];
            }
            if (pivots) pivots->push_back(col);
            ++rank;
        }
        return rank;
    }
public:
    explicit BinaryMatrix(std::size_t rows = 0, std::size_t cols = 0)
        : rows_(rows), cols_(cols), stride_(stride_for(cols)), data_(storage_size(rows, stride_)) {}
    BinaryMatrix(const BinaryMatrix&) = default;
    BinaryMatrix& operator=(const BinaryMatrix& other) { if (this != &other) { BinaryMatrix copy(other); swap(copy); } return *this; }
    BinaryMatrix(BinaryMatrix&& other) noexcept { swap(other); }
    BinaryMatrix& operator=(BinaryMatrix&& other) noexcept { if (this != &other) { clear(); swap(other); } return *this; }
    void swap(BinaryMatrix& other) noexcept { std::swap(rows_, other.rows_); std::swap(cols_, other.cols_); std::swap(stride_, other.stride_); data_.swap(other.data_); }
    friend void swap(BinaryMatrix& a, BinaryMatrix& b) noexcept { a.swap(b); }
    std::size_t rows() const noexcept { return rows_; }
    std::size_t cols() const noexcept { return cols_; }
    bool empty() const noexcept { return !rows_ || !cols_; }
    std::span<const std::uint64_t> row_words(std::size_t row) const {
        check_row(row); if (!stride_) return {};
        return std::span<const std::uint64_t>(data_).subspan(row * stride_, stride_);
    }
    bool test(std::size_t row, std::size_t col) const { check_position(row, col); return ((data_[row * stride_ + col / 64] >> (col % 64)) & 1) != 0; }
    bool operator()(std::size_t row, std::size_t col) const { return test(row, col); }
    BinaryMatrix& set(std::size_t row, std::size_t col, bool value = true) {
        check_position(row, col); auto& word = data_[row * stride_ + col / 64]; const auto mask = std::uint64_t{1} << (col % 64);
        if (value) word |= mask; else word &= ~mask; return *this;
    }
    BinaryMatrix& flip(std::size_t row, std::size_t col) { check_position(row, col); data_[row * stride_ + col / 64] ^= std::uint64_t{1} << (col % 64); return *this; }
    void swap_rows(std::size_t a, std::size_t b) {
        check_row(a); check_row(b); if (a == b) return;
        for (std::size_t word = 0; word < stride_; ++word) std::swap(data_[a * stride_ + word], data_[b * stride_ + word]);
    }
    void xor_rows(std::size_t target, std::size_t source, std::size_t first_col = 0) {
        check_row(target); check_row(source);
        if (first_col == cols_) return;
        const auto first_word = first_col / 64;
        data_[target * stride_ + first_word] ^= data_[source * stride_ + first_word] & (~std::uint64_t{0} << (first_col % 64));
        xor_suffix(target, source, first_word + 1);
    }
    static BinaryMatrix identity(std::size_t n) { BinaryMatrix result(n, n); for (std::size_t i = 0; i < n; ++i) result.data_[i * result.stride_ + i / 64] |= std::uint64_t{1} << (i % 64); return result; }
    BinaryMatrix transpose() const {
        BinaryMatrix result(cols_, rows_); if (empty()) return result;
        for (std::size_t row = 0; row < rows_; ++row) for (std::size_t col = 0; col < cols_; ++col)
            if ((data_[row * stride_ + col / 64] >> (col % 64)) & 1) result.data_[col * result.stride_ + row / 64] |= std::uint64_t{1} << (row % 64);
        return result;
    }
    BinaryMatrix multiply(const BinaryMatrix& other) const {
        BinaryMatrix result(rows_, other.cols_); if (!rows_ || !cols_ || !other.cols_) return result;
        const auto transposed = other.transpose();
        for (std::size_t row = 0; row < rows_; ++row) for (std::size_t col = 0; col < other.cols_; ++col)
            if (reinsirk::detail::bitset_intersection_parity_words(data_.data() + row * stride_, transposed.data_.data() + col * transposed.stride_, stride_))
                result.data_[row * result.stride_ + col / 64] |= std::uint64_t{1} << (col % 64);
        return result;
    }
    BinaryMatrix operator*(const BinaryMatrix& other) const { return multiply(other); }
    BinaryMatrix& operator*=(const BinaryMatrix& other) { auto result = multiply(other); swap(result); return *this; }
    BinaryMatrix& operator+=(const BinaryMatrix& other) { check_same_shape(other); for (std::size_t i = 0; i < data_.size(); ++i) data_[i] ^= other.data_[i]; return *this; }
    BinaryMatrix& operator^=(const BinaryMatrix& other) { return *this += other; }
    friend BinaryMatrix operator+(BinaryMatrix a, const BinaryMatrix& b) { a += b; return a; }
    friend BinaryMatrix operator^(BinaryMatrix a, const BinaryMatrix& b) { a ^= b; return a; }
    bool operator==(const BinaryMatrix& other) const noexcept { return rows_ == other.rows_ && cols_ == other.cols_ && data_ == other.data_; }
    BinaryMatrix pow(std::uint64_t exponent) const {
        check_square(); auto result = identity(rows_); if (!exponent || !rows_) return result;
        auto base = *this;
        while (exponent) { if (exponent & 1) result *= base; exponent >>= 1; if (exponent) base *= base; }
        return result;
    }
    std::size_t rank() const { auto work = *this; return eliminate(work, nullptr, nullptr, false); }
    bool determinant() const { check_square(); return rank() == rows_; }
    std::optional<BinaryMatrix> inverse() const {
        check_square(); auto work = *this, result = identity(rows_);
        if (eliminate(work, &result, nullptr, true) != rows_) return std::nullopt;
        return result;
    }
    std::optional<LinearSolution> solve(std::span<const std::uint8_t> rhs) const;
    void reset(std::size_t rows = 0, std::size_t cols = 0) { BinaryMatrix next(rows, cols); swap(next); }
    void clear() noexcept { data_.clear(); rows_ = cols_ = stride_ = 0; }
};

struct BinaryMatrix::LinearSolution {
    std::vector<std::uint8_t> particular;
    BinaryMatrix kernel;
    std::vector<std::size_t> pivot_columns;
    LinearSolution() = default;
    LinearSolution(std::vector<std::uint8_t> value, BinaryMatrix basis, std::vector<std::size_t> pivots)
        : particular(std::move(value)), kernel(std::move(basis)), pivot_columns(std::move(pivots)) {}
    LinearSolution(const LinearSolution&) = default;
    LinearSolution& operator=(const LinearSolution& other) { if (this != &other) { LinearSolution copy(other); swap(copy); } return *this; }
    LinearSolution(LinearSolution&& other) noexcept { swap(other); }
    LinearSolution& operator=(LinearSolution&& other) noexcept {
        if (this != &other) { particular.clear(); kernel.clear(); pivot_columns.clear(); swap(other); } return *this;
    }
    void swap(LinearSolution& other) noexcept { particular.swap(other.particular); kernel.swap(other.kernel); pivot_columns.swap(other.pivot_columns); }
    friend void swap(LinearSolution& a, LinearSolution& b) noexcept { a.swap(b); }
};

inline std::optional<BinaryMatrix::LinearSolution> BinaryMatrix::solve(std::span<const std::uint8_t> rhs) const {
    if (!cols_) {
        for (std::size_t row = 0; row < rows_; ++row) {
            if (rhs[row]) return std::nullopt;
        }
        return LinearSolution{};
    }
    std::vector<std::uint8_t> values(rows_);
    for (std::size_t i = 0; i < rows_; ++i) {
        values[i] = rhs[i];
    }
    auto work = *this; std::vector<std::size_t> pivots; const auto rank = eliminate(work, nullptr, &values, true, &pivots);
    for (auto row = rank; row < rows_; ++row) if (values[row]) return std::nullopt;
    LinearSolution result{std::vector<std::uint8_t>(cols_), BinaryMatrix(cols_ - rank, cols_), std::move(pivots)};
    for (std::size_t row = 0; row < rank; ++row) result.particular[result.pivot_columns[row]] = values[row];
    std::size_t pivot = 0, basis = 0;
    for (std::size_t col = 0; col < cols_; ++col) {
        if (pivot < rank && result.pivot_columns[pivot] == col) { ++pivot; continue; }
        result.kernel.set(basis, col);
        for (std::size_t row = 0; row < rank; ++row) if (work.test(row, col)) result.kernel.set(basis, result.pivot_columns[row]);
        ++basis;
    }
    return result;
}
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
#include <string>
int main() {
    std::size_t n{};verify_in>>n;
    BinaryMatrix a(n,n);
    for(std::size_t i=0;i<n;++i){std::string row;verify_in>>row;for(std::size_t j=0;j<n;++j)a.set(i,j,row[j]=='1');}
    const auto result=a.inverse();
    if(!result){verify_out<<-1<<'\n';return 0;}
    for(std::size_t i=0;i<n;++i){for(std::size_t j=0;j<n;++j)verify_out<<(result->test(i,j)?'1':'0');verify_out<<'\n';}
}
