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
#ifndef SKIP_RUNTIME_CHECKS
        if (row >= rows_) throw std::out_of_range("BinaryMatrix row");
#else
        (void)row;
#endif
    }
    void check_position(std::size_t row, std::size_t col) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (row >= rows_ || col >= cols_) throw std::out_of_range("BinaryMatrix position");
#else
        (void)row; (void)col;
#endif
    }
    void check_same_shape(const BinaryMatrix& other) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (rows_ != other.rows_ || cols_ != other.cols_) throw std::invalid_argument("BinaryMatrix shape");
#else
        (void)other;
#endif
    }
    void check_square() const {
#ifndef SKIP_RUNTIME_CHECKS
        if (rows_ != cols_) throw std::invalid_argument("BinaryMatrix square");
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
        if (first_col > cols_) throw std::out_of_range("BinaryMatrix first column");
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
        if (cols_ != other.rows_) throw std::invalid_argument("BinaryMatrix product shape");
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
    if (rhs.size() != rows_) throw std::invalid_argument("BinaryMatrix rhs length");
#endif
    if (!cols_) {
        for (std::size_t row = 0; row < rows_; ++row) {
#ifndef SKIP_RUNTIME_CHECKS
            if (rhs[row] > 1) throw std::invalid_argument("BinaryMatrix rhs bit");
#endif
            if (rhs[row]) return std::nullopt;
        }
        return LinearSolution{};
    }
    std::vector<std::uint8_t> values(rows_);
    for (std::size_t i = 0; i < rows_; ++i) {
#ifndef SKIP_RUNTIME_CHECKS
        if (rhs[i] > 1) throw std::invalid_argument("BinaryMatrix rhs bit");
#endif
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
