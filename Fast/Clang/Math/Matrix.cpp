// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Matrix.cpp
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/matrix/matrix.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix.md
//
// API一覧
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
        (void)expected;
    }
    Matrix(std::initializer_list<std::initializer_list<T>> rows)
        :rows_(rows.size()),columns_(rows.size()?rows.begin()->size():0) {
        values_.reserve(area(rows_,columns_));
        for(const auto& row:rows) {
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
        const auto offset=index*columns_;
        return {offset?values_.data()+offset:values_.data(),columns_};
    }
    std::span<const T> row(size_type index)const {
        const auto offset=index*columns_;
        return {offset?values_.data()+offset:values_.data(),columns_};
    }
    T& operator()(size_type row_index,size_type column_index) {
        return values_[row_index*columns_+column_index];
    }
    const T& operator()(size_type row_index,size_type column_index)const {
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
        for(size_type i=0;i<values_.size();++i)values_[i]+=other.values_[i];
        return *this;
    }
    Matrix& operator-=(const Matrix& other) {
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
    auto result=MatrixIdentity<T>(base.rows());
    while(exponent){if(exponent&1)result*=base;exponent>>=1;if(exponent)base*=base;}
    return result;
}
template<class T>std::vector<T> MatrixApply(const Matrix<T>& matrix,const std::vector<T>& vector) {
    std::vector<T> result(matrix.rows(),T{0});
    for(std::size_t i=0;i<matrix.rows();++i){const auto row=matrix.row(i);for(std::size_t j=0;j<matrix.cols();++j)result[i]+=row[j]*vector[j];}
    return result;
}
#endif
