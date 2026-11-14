#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/inverse_matrix

// Library: Math/Gaussian_Elimination.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Gaussian_Elimination.cpp
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/matrix/matrix.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix.md
//
// API一覧
// 以下の消去は正確な体Tが前提。Mint等は利用側が用意する。整数型・一般の合成数法・浮動小数点は対象外。
// GaussianElimination(a,reduced=false): 値で受け取った行列を行基本変形。元のlvalueは変更せず結果を返す。
// GaussianEliminationResult<T>: matrixはピボット1の階段形、pivot_columns/pivotsは列と正規化前の値、row_swapsは交換数。
// reduced=trueなら各ピボットの上も消去する。rank()はピボット数をO(1)で返す。
// rows=m,cols=nとし消去はO(m*n*min(m,n))時間・O(m*n)所有領域。0行・0列・矩形・特異も可。
// MatrixRank(a): 体上の階数。MatrixDeterminant(a): 正方行列の行列式、0x0はT{1}。消去と同計算量。
// MatrixInverse(a): 正方行列の逆行列をoptionalで返す。特異はnullopt、0x0は空の単位行列。O(n³)時間・O(n²)領域。
// SolveLinearSystem(a,b): a*x=bの一解と核の基底をoptionalで返す。不能ならnullopt、b.size()==a.rows()が前提。
// LinearSystemResult<T>: particularは長さnの一解、kernelは各行が核基底（列数n）、pivot_columnsはピボット列。
// SolveはO(m*(n+1)*(1+min(m,n))+n²)時間・O(m*(n+1)+n²)領域。0行では全空間、0列ではbが零のときだけ空の一解。
// 非零ピボットの逆数が1を作らなければdomain_error。これは係数型が体であること全体を検証するものではない。
// 0判定・非可逆ピボットの保護・内部容量上限はSKIP_RUNTIME_CHECKSでも維持する。
//
// [同梱Matrix]
// Matrix<T>: 行優先の連続vectorで所有する行列。Tはコピー可能で0/1構築・必要な環演算を提供する。
// 計算量はTの演算・コピーをO(1)として数える。
// Matrix()/Matrix(r,c,value=T{0}): 0x0/指定形状の一様行列。O(1)/O(r*c)時間・領域。
// Matrix(r,c,vector<T>): 値で受け取ったvectorを所有。move時O(1)、copy時O(r*c)。行リスト{{...},...}はO(r*c)でコピー。
// 行リストは全行同じ長さ、flat vectorはr*c要素。0行・0列も可。内部容量上限は検査省略時も維持。
// rows()/cols()/size()/empty(): 行数/列数/全要素数/要素数0かをO(1)で返す。
// operator()(r,c): 要素の参照。row(r): 行の非所有span。values(): 全値の非所有span。各O(1)。
// rowの列添字はspanと同じく範囲内を渡す。行列の代入・swap・破棄で非所有参照は無効になる。
// copy/move/代入/swap: 独立コピー/所有移譲。コピーO(r*c)、move/swapはO(1)、移動元は0x0。
// +=/-=/+/-: 同形行列の加減。O(r*c)。==/!=は形状と値を比較する。
// */*=: (r,k)*(k,c)の行列積。O(r*c*(1+k))時間・O(r*c)追加領域。入力は変更しない（*=は左辺を更新）。
// transposed(): 転置の独立コピー。O(r*c)時間・領域。assign(r,c,value): 新しい一様行列へ置換する。
// MatrixIdentity<T>(n): nxn単位行列。O(n²)。MatrixPower(a,k): 正方行列の非負整数乗、O(n³ log(k+2))。
// MatrixApply(a,x): a*xの列ベクトルを返す。O(rows*(1+cols))時間・O(rows)領域。
// boolには専用のGF2係数型等を用意する。係数演算は正確な環の演算が前提。固定幅整数の演算オーバーフローや浮動小数点誤差は利用側で管理する。

// GCC/x86のuint32_t行列積は、対応CPUでAVX2を使う。unsignedの積と加算は通常版と同じmod 2^32。
// SIRKIT_DISABLE_SIMDで専用実装とCPU判定を除外する。小さい行列・他の型・非対応CPU/コンパイラは標準ループ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <bit>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MATRIX_DEFINED
#define SIRKIT_MATRIX_DEFINED
template<class T>
class Matrix {
    static_assert(!std::is_same_v<T,bool>,"Matrix<bool>: provide a coefficient type instead of vector<bool>");
    std::size_t rows_=0,columns_=0;
    std::vector<T> values_;
    static std::size_t area(std::size_t rows,std::size_t columns) {
        if(columns&&rows>std::vector<T>{}.max_size()/columns)
            throw std::length_error("Matrix: dimensions overflow");
        return rows*columns;
    }
public:
    using value_type=T;
    using size_type=std::size_t;
    Matrix()=default;
    Matrix(size_type rows,size_type columns,const T& value=T{0})
        :rows_(rows),columns_(columns),values_(area(rows,columns),value){}
    Matrix(size_type rows,size_type columns,std::vector<T> values)
        :rows_(rows),columns_(columns),values_(std::move(values)) {
        const auto expected=area(rows,columns);
#ifndef SKIP_RUNTIME_CHECKS
        if(values_.size()!=expected)throw std::invalid_argument("Matrix: flat size mismatch");
#else
        (void)expected;
#endif
    }
    Matrix(std::initializer_list<std::initializer_list<T>> rows)
        :rows_(rows.size()),columns_(rows.size()?rows.begin()->size():0) {
        values_.reserve(area(rows_,columns_));
        for(const auto& row:rows) {
#ifndef SKIP_RUNTIME_CHECKS
            if(row.size()!=columns_)throw std::invalid_argument("Matrix: ragged rows");
#endif
            values_.insert(values_.end(),row.begin(),row.end());
        }
    }
    Matrix(const Matrix&)=default;
    Matrix(Matrix&& other) noexcept
        :rows_(std::exchange(other.rows_,0)),columns_(std::exchange(other.columns_,0)),
         values_(std::move(other.values_)) { other.values_.clear(); }
    Matrix& operator=(Matrix other) noexcept { swap(other);return *this; }
    void swap(Matrix& other) noexcept {
        std::swap(rows_,other.rows_);std::swap(columns_,other.columns_);values_.swap(other.values_);
    }
    friend void swap(Matrix& a,Matrix& b) noexcept { a.swap(b); }
    size_type rows()const noexcept{return rows_;}
    size_type cols()const noexcept{return columns_;}
    size_type size()const noexcept{return values_.size();}
    bool empty()const noexcept{return values_.empty();}
    std::span<T> values()noexcept{return {values_.data(),values_.size()};}
    std::span<const T> values()const noexcept{return {values_.data(),values_.size()};}
    std::span<T> row(size_type index) {
#ifndef SKIP_RUNTIME_CHECKS
        if(index>=rows_)throw std::out_of_range("Matrix::row: index");
#endif
        const auto offset=index*columns_;
        return {offset?values_.data()+offset:values_.data(),columns_};
    }
    std::span<const T> row(size_type index)const {
#ifndef SKIP_RUNTIME_CHECKS
        if(index>=rows_)throw std::out_of_range("Matrix::row: index");
#endif
        const auto offset=index*columns_;
        return {offset?values_.data()+offset:values_.data(),columns_};
    }
    T& operator()(size_type row_index,size_type column_index) {
#ifndef SKIP_RUNTIME_CHECKS
        if(row_index>=rows_||column_index>=columns_)throw std::out_of_range("Matrix: index");
#endif
        return values_[row_index*columns_+column_index];
    }
    const T& operator()(size_type row_index,size_type column_index)const {
#ifndef SKIP_RUNTIME_CHECKS
        if(row_index>=rows_||column_index>=columns_)throw std::out_of_range("Matrix: index");
#endif
        return values_[row_index*columns_+column_index];
    }
    void assign(size_type rows,size_type columns,const T& value=T{0}) {
        Matrix next(rows,columns,value);swap(next);
    }
    Matrix transposed()const {
        Matrix result(columns_,rows_);
        if(values_.empty())return result;
        for(size_type i=0;i<rows_;++i)
            for(size_type j=0;j<columns_;++j)result.values_[j*rows_+i]=values_[i*columns_+j];
        return result;
    }
    Matrix& operator+=(const Matrix& other) {
#ifndef SKIP_RUNTIME_CHECKS
        if(rows_!=other.rows_||columns_!=other.columns_)throw std::invalid_argument("Matrix: shape mismatch");
#endif
        for(size_type i=0;i<values_.size();++i)values_[i]+=other.values_[i];
        return *this;
    }
    Matrix& operator-=(const Matrix& other) {
#ifndef SKIP_RUNTIME_CHECKS
        if(rows_!=other.rows_||columns_!=other.columns_)throw std::invalid_argument("Matrix: shape mismatch");
#endif
        for(size_type i=0;i<values_.size();++i)values_[i]-=other.values_[i];
        return *this;
    }
    Matrix& operator*=(const Matrix& other);
    friend Matrix operator+(Matrix a,const Matrix& b){a+=b;return a;}
    friend Matrix operator-(Matrix a,const Matrix& b){a-=b;return a;}
    bool operator==(const Matrix&)const=default;
};
#endif
#ifndef SIRKIT_MATRIX_ARITHMETIC_DEFINED
#define SIRKIT_MATRIX_ARITHMETIC_DEFINED
template<class T>
Matrix<T> operator*(const Matrix<T>& a,const Matrix<T>& b) {
#ifndef SKIP_RUNTIME_CHECKS
    if(a.cols()!=b.rows())throw std::invalid_argument("Matrix product: shape mismatch");
#endif
    Matrix<T> result(a.rows(),b.cols());
    if(result.empty()||a.cols()==0)return result;

    for(std::size_t i=0;i<a.rows();++i) {
        auto destination=result.row(i);const auto left=a.row(i);
        for(std::size_t k=0;k<a.cols();++k) {
            const T multiplier=left[k];const auto right=b.row(k);
            for(std::size_t j=0;j<b.cols();++j)destination[j]+=multiplier*right[j];
        }
    }
    return result;
}
template<class T>Matrix<T>& Matrix<T>::operator*=(const Matrix& other) {
    auto result=*this*other;swap(result);return *this;
}
template<class T>Matrix<T> MatrixIdentity(std::size_t n) {
    Matrix<T> result(n,n);for(std::size_t i=0;i<n;++i)result.row(i)[i]=T{1};return result;
}
template<class T>Matrix<T> MatrixPower(Matrix<T> base,std::uint64_t exponent) {
#ifndef SKIP_RUNTIME_CHECKS
    if(base.rows()!=base.cols())throw std::invalid_argument("MatrixPower: non-square");
#endif
    auto result=MatrixIdentity<T>(base.rows());
    while(exponent){if(exponent&1)result*=base;exponent>>=1;if(exponent)base*=base;}
    return result;
}
template<class T>std::vector<T> MatrixApply(const Matrix<T>& matrix,const std::vector<T>& vector) {
#ifndef SKIP_RUNTIME_CHECKS
    if(matrix.cols()!=vector.size())throw std::invalid_argument("MatrixApply: shape mismatch");
#endif
    std::vector<T> result(matrix.rows(),T{0});
    for(std::size_t i=0;i<matrix.rows();++i){const auto row=matrix.row(i);for(std::size_t j=0;j<matrix.cols();++j)result[i]+=row[j]*vector[j];}
    return result;
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <type_traits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GAUSSIAN_RESULT_DEFINED
#define SIRKIT_GAUSSIAN_RESULT_DEFINED
template<class T>struct GaussianEliminationResult {
    Matrix<T> matrix;
    std::vector<std::size_t> pivot_columns;
    std::vector<T> pivots;
    std::size_t row_swaps=0;
    std::size_t rank()const noexcept{return pivot_columns.size();}
};
template<class T>struct LinearSystemResult {
    std::vector<T> particular;
    Matrix<T> kernel;
    std::vector<std::size_t> pivot_columns;
};
#endif
#ifndef SIRKIT_GAUSSIAN_DETAIL_DEFINED
#define SIRKIT_GAUSSIAN_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T>
GaussianEliminationResult<T> field_eliminate(Matrix<T> matrix,std::size_t pivot_end,bool reduced) {
    static_assert(!std::is_integral_v<T>,"Gaussian elimination requires an exact field type, not an integer type");
    GaussianEliminationResult<T> result{std::move(matrix),{},{},0};
    auto& work=result.matrix;
    result.pivot_columns.reserve(std::min(work.rows(),pivot_end));
    result.pivots.reserve(std::min(work.rows(),pivot_end));
    for(std::size_t column=0;column<pivot_end&&result.rank()<work.rows();++column) {
        const auto rank=result.rank();std::size_t candidate=rank;
        while(candidate<work.rows()&&work.row(candidate)[column]==T{0})++candidate;
        if(candidate==work.rows())continue;
        auto pivot_row=work.row(rank);
        if(candidate!=rank) {
            auto other=work.row(candidate);
            for(std::size_t j=column;j<work.cols();++j)std::swap(pivot_row[j],other[j]);
            ++result.row_swaps;
        }
        const T pivot=pivot_row[column];
        result.pivot_columns.push_back(column);result.pivots.push_back(pivot);
        if(pivot!=T{1}) {
            const T inverse=T{1}/pivot;
            if(pivot*inverse!=T{1})throw std::domain_error("Gaussian elimination: non-invertible pivot");
            for(std::size_t j=column+1;j<work.cols();++j)pivot_row[j]*=inverse;
            pivot_row[column]=T{1};
        }
        const auto begin=reduced?std::size_t{0}:rank+1;
        for(std::size_t i=begin;i<work.rows();++i) {
            if(i==rank)continue;
            auto destination=work.row(i);const T factor=destination[column];
            if(factor==T{0})continue;
            for(std::size_t j=column+1;j<work.cols();++j)destination[j]-=factor*pivot_row[j];
            destination[column]=T{0};
        }
    }
    return result;
}
}
#endif
#ifndef SIRKIT_FIELD_GAUSSIAN_DEFINED
#define SIRKIT_FIELD_GAUSSIAN_DEFINED
template<class T>GaussianEliminationResult<T> GaussianElimination(Matrix<T> matrix,bool reduced=false) {
    const auto columns=matrix.cols();
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::field_eliminate(std::move(matrix),columns,reduced);
}
template<class T>std::size_t MatrixRank(const Matrix<T>& matrix) {
    return GaussianElimination(matrix).rank();
}
template<class T>T MatrixDeterminant(const Matrix<T>& matrix) {
#ifndef SKIP_RUNTIME_CHECKS
    if(matrix.rows()!=matrix.cols())throw std::invalid_argument("MatrixDeterminant: non-square");
#endif
    auto result=GaussianElimination(matrix);
    if(result.rank()!=matrix.rows())return T{0};
    T determinant{1};for(const auto& pivot:result.pivots)determinant*=pivot;
    return (result.row_swaps&1)?T{0}-determinant:determinant;
}
template<class T>std::optional<Matrix<T>> MatrixInverse(const Matrix<T>& matrix) {
#ifndef SKIP_RUNTIME_CHECKS
    if(matrix.rows()!=matrix.cols())throw std::invalid_argument("MatrixInverse: non-square");
#endif
    const auto n=matrix.rows();
    if(n>std::numeric_limits<std::size_t>::max()/2)throw std::length_error("MatrixInverse: dimensions overflow");
    Matrix<T> augmented(n,n*2);
    for(std::size_t i=0;i<n;++i){auto row=augmented.row(i);const auto source=matrix.row(i);std::copy(source.begin(),source.end(),row.begin());row[n+i]=T{1};}
    auto elimination=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::field_eliminate(std::move(augmented),n,true);
    if(elimination.rank()!=n)return std::nullopt;
    Matrix<T> inverse(n,n);
    for(std::size_t i=0;i<n;++i){const auto row=elimination.matrix.row(i);std::copy(row.begin()+static_cast<std::ptrdiff_t>(n),row.end(),inverse.row(i).begin());}
    return inverse;
}
template<class T>
std::optional<LinearSystemResult<T>> SolveLinearSystem(const Matrix<T>& matrix,const std::vector<T>& rhs) {
#ifndef SKIP_RUNTIME_CHECKS
    if(matrix.rows()!=rhs.size())throw std::invalid_argument("SolveLinearSystem: rhs size mismatch");
#endif
    const auto m=matrix.rows(),n=matrix.cols();
    if(n==std::numeric_limits<std::size_t>::max())throw std::length_error("SolveLinearSystem: dimensions overflow");
    Matrix<T> augmented(m,n+1);
    for(std::size_t i=0;i<m;++i){auto row=augmented.row(i);const auto source=matrix.row(i);std::copy(source.begin(),source.end(),row.begin());row[n]=rhs[i];}
    auto elimination=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::field_eliminate(std::move(augmented),n,true);
    const auto rank=elimination.rank();
    for(std::size_t i=rank;i<m;++i)if(elimination.matrix.row(i)[n]!=T{0})return std::nullopt;
    LinearSystemResult<T> result{std::vector<T>(n,T{0}),Matrix<T>(n-rank,n),std::move(elimination.pivot_columns)};
    std::vector<std::uint8_t> pivot(n,0);
    for(std::size_t i=0;i<rank;++i){const auto column=result.pivot_columns[i];pivot[column]=1;result.particular[column]=elimination.matrix.row(i)[n];}
    std::size_t basis=0;
    for(std::size_t column=0;column<n;++column) {
        if(pivot[column])continue;
        auto vector=result.kernel.row(basis++);vector[column]=T{1};
        for(std::size_t i=0;i<rank;++i)vector[result.pivot_columns[i]]=T{0}-elimination.matrix.row(i)[column];
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

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

// Verify support: Verify/current/common/matrix_field.hpp
// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <stdexcept>
#ifndef SIRKIT_VERIFY_MATRIX_FIELD_DEFINED
#define SIRKIT_VERIFY_MATRIX_FIELD_DEFINED
struct VerifyMatrixField {
    static constexpr std::uint32_t modulus=998244353;
    std::uint32_t value=0;
    VerifyMatrixField()=default;
    VerifyMatrixField(std::uint64_t input):value(static_cast<std::uint32_t>(input%modulus)){}
    std::uint32_t get()const noexcept{return value;}
    VerifyMatrixField& operator+=(VerifyMatrixField b)noexcept{value+=b.value;if(value>=modulus)value-=modulus;return *this;}
    VerifyMatrixField& operator-=(VerifyMatrixField b)noexcept{value=value>=b.value?value-b.value:value+modulus-b.value;return *this;}
    VerifyMatrixField& operator*=(VerifyMatrixField b)noexcept{value=static_cast<std::uint32_t>(std::uint64_t{value}*b.value%modulus);return *this;}
    friend VerifyMatrixField operator+(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a+=b;}
    friend VerifyMatrixField operator-(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a-=b;}
    friend VerifyMatrixField operator*(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a*=b;}
    friend bool operator==(VerifyMatrixField,VerifyMatrixField)=default;
    friend VerifyMatrixField operator/(VerifyMatrixField a,VerifyMatrixField b){
        if(b.value==0)throw std::domain_error("verification field: zero divisor");
        VerifyMatrixField inverse{1};std::uint32_t exponent=modulus-2;
        while(exponent){if(exponent&1)inverse*=b;b*=b;exponent>>=1;}return a*inverse;
    }
};
#endif

#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::size_t n{};if(!(verify_in>>n))return 0;
    Matrix<VerifyMatrixField>a(n,n);
    for(auto& value:a.values()){std::uint32_t raw{};verify_in>>raw;value=VerifyMatrixField{raw};}
    const auto inverse=MatrixInverse(a);
    if(!inverse){verify_out<<-1<<'\n';return 0;}
    const auto& result=*inverse;
    for(std::size_t i=0;i<result.rows();++i)for(std::size_t j=0;j<result.cols();++j)verify_out<<result(i,j).get()<<(j+1==result.cols()?'\n':' ');
}
